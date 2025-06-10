#pragma once
#include <AllGateVisitor.hpp>
#include <NoiseModel.hpp>
#include <sstream>
#include <unordered_map>

namespace qristal {
template <typename T>
bool contains(const std::vector<T> &container, const T &val) {
  return std::find(container.begin(), container.end(), val) != container.end();
}

// Qubit life-time can be a single value (assuming all qubits are the same)
// or specific values for each qubits.
union QubitLifetime {
  double val;
  std::vector<double> vals;
  QubitLifetime() {}
  ~QubitLifetime() {}
};

struct QbNoiseParams {
  QubitLifetime T1;
  QubitLifetime T2;
  bool is_uniform;
  QbNoiseParams(double in_T1, double in_T2) : is_uniform(true) {
    T1.val = in_T1;
    T2.val = in_T2;
  };

  QbNoiseParams(const std::vector<double> &in_T1s,
                const std::vector<double> &in_T2s)
      : is_uniform(false) {
    assert(in_T1s.size() == in_T2s.size());
    T1.vals = in_T1s;
    T2.vals = in_T2s;
  };

  QbNoiseParams(const QbNoiseParams &other) : is_uniform(other.is_uniform) {
    if (is_uniform) {
      T1.val = other.T1.val;
      T2.val = other.T2.val;

    } else {
      T1.vals = other.T1.vals;
      T2.vals = other.T2.vals;
    }
  }

  double computeAmplitudeDampingRate(double gateTime, int qubit) {
    const double qubitT1 = is_uniform ? T1.val : T1.vals[qubit];
    const double rate = 1.0 / qubitT1;
    return 1.0 - std::exp(-gateTime * rate);
  }

  // Ref:
  // https://quantumcomputing.stackexchange.com/questions/17690/t-1-t-2-thermal-relaxation-example/17692#17692
  double computePhaseDampingRate(double gateTime, int qubit) {
    const double qubitT1 = is_uniform ? T1.val : T1.vals[qubit];
    const double qubitT2 = is_uniform ? T2.val : T2.vals[qubit];
    if (qubitT2 >= 2.0 * qubitT1) {
      return 0.0;
    }
    const double qubitTphi = 1.0 / (1.0 / qubitT2 - 1.0 / (2.0 * qubitT1));
    assert(qubitTphi > 0.0);
    const double rate = 1.0 / qubitTphi;
    return 1.0 - std::exp(-gateTime * rate);
  }
};

struct QbHardwareModel {
  QbNoiseParams noiseModel;
  double rxGateTime;
  double ryGateTime;
  double czGateTime;
  // Gate name -> rx, ry, cz layer count
  // !!IMPORTANT!! This has considered layering of gates.
  static inline const std::unordered_map<std::string,
                                         std::tuple<double, double, double>>
      XACC_GATE_NAME_TO_QB_GATE_LAYER_COUNTS{
          {"CNOT", {2, 2, 1}}, {"X", {1, 0, 0}},     {"Y", {0, 1, 0}},
          {"Z", {2, 1, 0}},    {"Rx", {1, 0, 0}},    {"Ry", {0, 1, 0}},
          {"Rz", {2, 1, 0}},   {"H", {1, 1, 0}},     {"I", {1, 0, 0}},
          {"S", {2, 1, 0}},    {"Sdg", {2, 1, 0}},   {"T", {2, 1, 0}},
          {"Tdg", {2, 1, 0}},  {"Swap", {4, 4, 3}},  {"iSwap", {5, 4, 2}},
          {"CY", {18, 10, 2}}, {"CZ", {0, 0, 1}},    {"XX", {8, 7, 2}},
          {"XY", {0, 0, 0}},   {"RZZ", {6, 5, 2}},   {"U1", {2, 1, 0}},
          {"U", {4, 3, 0}},    {"CPhase", {8, 5, 2}}};

  QbHardwareModel(double in_rxTime, double in_ryTime, double in_czTime,
                  double T1, double T2)
      : rxGateTime(in_rxTime), ryGateTime(in_ryTime), czGateTime(in_czTime),
        noiseModel(T1, T2) {}

  double getGateTime(const std::string &gateName) const {
    const auto iter = XACC_GATE_NAME_TO_QB_GATE_LAYER_COUNTS.find(gateName);
    assert(iter != XACC_GATE_NAME_TO_QB_GATE_LAYER_COUNTS.end());
    if (iter != XACC_GATE_NAME_TO_QB_GATE_LAYER_COUNTS.end()) {
      const auto [rxCount, ryCount, czCount] = iter->second;
      return rxCount * rxGateTime + ryCount * ryGateTime + czCount * czGateTime;
    }
    return rxGateTime;
  }

  static inline QbHardwareModel &DEFAULT_MODEL() {
    static QbHardwareModel model(1.0e3, 1.0e3, 1.0e3, 1.0e9, 1.0e6);
    return model;
  }
};

class AWSOpenQASM3Visitor : public xacc::quantum::AllGateVisitor {
private:
  bool m_noise;
  bool m_verbatim;
  std::stringstream m_openQasm;
  int measure_gate_count = 0;
  std::shared_ptr<xacc::NoiseModel> m_noiseModel;
  // device.properties.action["braket.ir.openqasm.program"].dict()["supportedOperations"]
  std::vector<std::string> m_supportedOps;
  std::string m_regName;
  QbHardwareModel m_hardwareModel;
  // Mapping from XACC gate name to AWS gate name
  static inline const std::unordered_map<std::string, std::string>
      XACC_GATE_NAME_TO_AWS_GATE_NAME = {
          {"CNOT", "cnot"},     {"X", "x"},
          {"Y", "y"},           {"Z", "z"},
          {"Rx", "rx"},         {"Ry", "ry"},
          {"Rz", "rz"},         {"H", "h"},
          {"I", "i"},           {"S", "s"},
          {"Sdg", "si"},        {"T", "t"},
          {"Tdg", "ti"},        {"Swap", "swap"},
          {"iSwap", "iswap"},   {"CY", "cy"},
          {"CZ", "cz"},         {"XX", "xx"},
          {"XY", "xy"},         {"RZZ", "zz"},
          {"U1", "phaseshift"}, {"CPhase", "cphaseshift"}};

public:
  AWSOpenQASM3Visitor(
      int nbQubit = 1,
      bool noise = false,
      bool verbatim = true, 
      const QbHardwareModel &hardwareModel = QbHardwareModel::DEFAULT_MODEL(),
      const std::vector<std::string> &supportedOps = {})
      : m_supportedOps(supportedOps), m_regName(verbatim ? "$" : "q"),
        m_hardwareModel(hardwareModel) {
    // Preample
    m_noise = noise;
    m_verbatim = verbatim;
    m_openQasm << "OPENQASM 3;\n";
    if (!verbatim) {
      m_openQasm << "qubit[" << nbQubit << "] q;\n";
    }
    else{
      m_openQasm << "bit[" << nbQubit <<   "] c;\n";
      m_openQasm << "#pragma braket verbatim \nbox{ \n"; 
    }
  }

public:
  void visit(xacc::quantum::RZZ &rzz) override {
    // Ising (ZZ) gate.
    const auto iter = XACC_GATE_NAME_TO_AWS_GATE_NAME.find(rzz.name());
    assert(iter != XACC_GATE_NAME_TO_AWS_GATE_NAME.end());
    addOpenQasm3Gate(iter->second, rzz.bits(),
                     m_hardwareModel.getGateTime(rzz.name()),
                     {rzz.getParameter(0).as<double>()});
  }

  void visit(xacc::quantum::Hadamard &h) override {
    const auto iter = XACC_GATE_NAME_TO_AWS_GATE_NAME.find(h.name());
    assert(iter != XACC_GATE_NAME_TO_AWS_GATE_NAME.end());
    addOpenQasm3Gate(iter->second, h.bits(),
                     m_hardwareModel.getGateTime(h.name()));
  }

  void visit(xacc::quantum::CNOT &cx) override {
    const auto iter = XACC_GATE_NAME_TO_AWS_GATE_NAME.find(cx.name());
    assert(iter != XACC_GATE_NAME_TO_AWS_GATE_NAME.end());
    addOpenQasm3Gate(iter->second, cx.bits(),
                     m_hardwareModel.getGateTime(cx.name()));
  }

  void visit(xacc::quantum::Rz &rz) override {
    const auto iter = XACC_GATE_NAME_TO_AWS_GATE_NAME.find(rz.name());
    assert(iter != XACC_GATE_NAME_TO_AWS_GATE_NAME.end());
    addOpenQasm3Gate(iter->second, rz.bits(),
                     m_hardwareModel.getGateTime(rz.name()),
                     {rz.getParameter(0).as<double>()});
  }

  void visit(xacc::quantum::Ry &ry) override {
    const auto iter = XACC_GATE_NAME_TO_AWS_GATE_NAME.find(ry.name());
    assert(iter != XACC_GATE_NAME_TO_AWS_GATE_NAME.end());
    addOpenQasm3Gate(iter->second, ry.bits(),
                     m_hardwareModel.getGateTime(ry.name()),
                     {ry.getParameter(0).as<double>()});
  }

  void visit(xacc::quantum::Rx &rx) override {
    const auto iter = XACC_GATE_NAME_TO_AWS_GATE_NAME.find(rx.name());
    assert(iter != XACC_GATE_NAME_TO_AWS_GATE_NAME.end());
    addOpenQasm3Gate(iter->second, rx.bits(),
                     m_hardwareModel.getGateTime(rx.name()),
                     {rx.getParameter(0).as<double>()});
  }

  void visit(xacc::quantum::U1 &u1) override {
    const auto iter = XACC_GATE_NAME_TO_AWS_GATE_NAME.find(u1.name());
    assert(iter != XACC_GATE_NAME_TO_AWS_GATE_NAME.end());
    addOpenQasm3Gate(iter->second, u1.bits(),
                     m_hardwareModel.getGateTime(u1.name()),
                     {u1.getParameter(0).as<double>()});
  }

  void visit(xacc::quantum::X &x) override {
    const auto iter = XACC_GATE_NAME_TO_AWS_GATE_NAME.find(x.name());
    assert(iter != XACC_GATE_NAME_TO_AWS_GATE_NAME.end());
    addOpenQasm3Gate(iter->second, x.bits(),
                     m_hardwareModel.getGateTime(x.name()));
  }
  void visit(xacc::quantum::Y &y) override {
    const auto iter = XACC_GATE_NAME_TO_AWS_GATE_NAME.find(y.name());
    assert(iter != XACC_GATE_NAME_TO_AWS_GATE_NAME.end());
    addOpenQasm3Gate(iter->second, y.bits(),
                     m_hardwareModel.getGateTime(y.name()));
  }
  void visit(xacc::quantum::Z &z) override {
    const auto iter = XACC_GATE_NAME_TO_AWS_GATE_NAME.find(z.name());
    assert(iter != XACC_GATE_NAME_TO_AWS_GATE_NAME.end());
    addOpenQasm3Gate(iter->second, z.bits(),
                     m_hardwareModel.getGateTime(z.name()));
  }

  void visit(xacc::quantum::CY &cy) override {
    const auto iter = XACC_GATE_NAME_TO_AWS_GATE_NAME.find(cy.name());
    assert(iter != XACC_GATE_NAME_TO_AWS_GATE_NAME.end());
    addOpenQasm3Gate(iter->second, cy.bits(),
                     m_hardwareModel.getGateTime(cy.name()));
  }

  void visit(xacc::quantum::CZ &cz) override {
    const auto iter = XACC_GATE_NAME_TO_AWS_GATE_NAME.find(cz.name());
    assert(iter != XACC_GATE_NAME_TO_AWS_GATE_NAME.end());
    addOpenQasm3Gate(iter->second, cz.bits(),
                     m_hardwareModel.getGateTime(cz.name()));
  }

  void visit(xacc::quantum::Swap &swap) override {
    const auto iter = XACC_GATE_NAME_TO_AWS_GATE_NAME.find(swap.name());
    assert(iter != XACC_GATE_NAME_TO_AWS_GATE_NAME.end());
    addOpenQasm3Gate(iter->second, swap.bits(),
                     m_hardwareModel.getGateTime(swap.name()));
  }

  void visit(xacc::quantum::fSim &fsim) override {
    // TODO
  }
  void visit(xacc::quantum::iSwap &isw) override {
    const auto iter = XACC_GATE_NAME_TO_AWS_GATE_NAME.find(isw.name());
    assert(iter != XACC_GATE_NAME_TO_AWS_GATE_NAME.end());
    addOpenQasm3Gate(iter->second, isw.bits(),
                     m_hardwareModel.getGateTime(isw.name()));
  }
  void visit(xacc::quantum::XY &xy) override {
    // Ising (XY) gate.
    const auto iter = XACC_GATE_NAME_TO_AWS_GATE_NAME.find(xy.name());
    assert(iter != XACC_GATE_NAME_TO_AWS_GATE_NAME.end());
    addOpenQasm3Gate(iter->second, xy.bits(),
                     m_hardwareModel.getGateTime(xy.name()),
                     {xy.getParameter(0).as<double>()});
  }
  void visit(xacc::quantum::CRZ &crz) override {
    const auto iter = XACC_GATE_NAME_TO_AWS_GATE_NAME.find(crz.name());
    assert(iter != XACC_GATE_NAME_TO_AWS_GATE_NAME.end());
    addOpenQasm3Gate(iter->second, crz.bits(),
                     m_hardwareModel.getGateTime(crz.name()),
                     {crz.getParameter(0).as<double>()});
  }
  void visit(xacc::quantum::CH &ch) override {
    // TODO
  }
  void visit(xacc::quantum::S &s) override {
    const auto iter = XACC_GATE_NAME_TO_AWS_GATE_NAME.find(s.name());
    assert(iter != XACC_GATE_NAME_TO_AWS_GATE_NAME.end());
    addOpenQasm3Gate(iter->second, s.bits(),
                     m_hardwareModel.getGateTime(s.name()));
  }
  void visit(xacc::quantum::CPhase &cp) override {
    const auto iter = XACC_GATE_NAME_TO_AWS_GATE_NAME.find(cp.name());
    assert(iter != XACC_GATE_NAME_TO_AWS_GATE_NAME.end());
    addOpenQasm3Gate(iter->second, cp.bits(),
                     m_hardwareModel.getGateTime(cp.name()),
                     {cp.getParameter(0).as<double>()});
  }

  void visit(xacc::quantum::Measure &m) override {
      measure_gate_count += 1;

         if (m_verbatim){
              if (measure_gate_count == 1)   m_openQasm << "}\n";
              m_openQasm << "c[" << measure_gate_count-1 << "] = ";
            }
            
    addOpenQasm3Gate("measure", m.bits(), 0.0);
  }
  void visit(xacc::quantum::Identity &i) override {
    const auto iter = XACC_GATE_NAME_TO_AWS_GATE_NAME.find(i.name());
    assert(iter != XACC_GATE_NAME_TO_AWS_GATE_NAME.end());
    addOpenQasm3Gate(iter->second, i.bits(),
                     m_hardwareModel.getGateTime(i.name()));
  }

  void visit(xacc::quantum::U &u) override {
    const double theta = u.getParameter(0).as<double>();
    const double phi = u.getParameter(1).as<double>();
    const double lam = u.getParameter(2).as<double>();
    addOpenQasm3Gate("rz", u.bits(), m_hardwareModel.getGateTime("Rz"), {lam});
    addOpenQasm3Gate("rx", u.bits(), m_hardwareModel.getGateTime("Rx"),
                     {M_PI / 2.0});
    addOpenQasm3Gate("rz", u.bits(), m_hardwareModel.getGateTime("Rz"),
                     {theta});
    addOpenQasm3Gate("rx", u.bits(), m_hardwareModel.getGateTime("Rx"),
                     {-M_PI / 2.0});
    addOpenQasm3Gate("rz", u.bits(), m_hardwareModel.getGateTime("Rz"), {phi});
  }

  void visit(xacc::quantum::Rphi &r) override {
    // TODO
    assert(false);
  }

  void visit(xacc::quantum::XX &xx) override {
    // Ising (XX) gate.
    const auto iter = XACC_GATE_NAME_TO_AWS_GATE_NAME.find(xx.name());
    assert(iter != XACC_GATE_NAME_TO_AWS_GATE_NAME.end());
    addOpenQasm3Gate(iter->second, xx.bits(),
                     m_hardwareModel.getGateTime(xx.name()),
                     {xx.getParameter(0).as<double>()});
  }

  void visit(xacc::quantum::Sdg &sdg) override {
    const auto iter = XACC_GATE_NAME_TO_AWS_GATE_NAME.find(sdg.name());
    assert(iter != XACC_GATE_NAME_TO_AWS_GATE_NAME.end());
    addOpenQasm3Gate(iter->second, sdg.bits(),
                     m_hardwareModel.getGateTime(sdg.name()));
  }
  void visit(xacc::quantum::T &t) override {
    const auto iter = XACC_GATE_NAME_TO_AWS_GATE_NAME.find(t.name());
    assert(iter != XACC_GATE_NAME_TO_AWS_GATE_NAME.end());
    addOpenQasm3Gate(iter->second, t.bits(),
                     m_hardwareModel.getGateTime(t.name()));
  }
  void visit(xacc::quantum::Tdg &tdg) override {
    const auto iter = XACC_GATE_NAME_TO_AWS_GATE_NAME.find(tdg.name());
    assert(iter != XACC_GATE_NAME_TO_AWS_GATE_NAME.end());
    addOpenQasm3Gate(iter->second, tdg.bits(),
                     m_hardwareModel.getGateTime(tdg.name()));
  }
  void visit(xacc::quantum::IfStmt &tdg) override {
    // TODO
  }
  void visit(xacc::quantum::Reset &reset) override {
    // TODO
  }
  std::string getOpenQasm() const { return m_openQasm.str(); }

private:
  void addOpenQasm3Gate(const std::string &gateName,
                        const std::vector<size_t> &operands, double gateTime,
                        const std::vector<double> &params = {}) {
    
    m_openQasm << gateName;

    if (m_verbatim){      
      if (!params.empty()) {
        m_openQasm << "(";
        for (int i = 0; i < params.size() - 1; ++i) {
          m_openQasm << params[i] << ", ";
        }
        m_openQasm << params[params.size() - 1] << ")";
      }
      m_openQasm << " ";
      for (int i = 0; i < operands.size() - 1; ++i) {
        m_openQasm << m_regName << operands[i] << ", ";
      }
      //if (gateName == "measure") m_openQasm << "c[" << operands[operands.size() - 1] << "] = ";
      m_openQasm << m_regName << operands[operands.size() - 1] << ";\n";
      // Note: by default, noise channels are applied **AFTER** the gate.
      //std::cout << "m_noise is" << m_noise << "\n" ;
      //std::cout << "m_verbatim is" << m_verbatim << "\n" ;
      if (m_noise){
        if (gateName != "measure") {
          for (const auto &qubit : operands) {
            const auto amplitude_damping_rate =
                m_hardwareModel.noiseModel.computeAmplitudeDampingRate(gateTime,
                                                                       qubit);
            const auto phase_dammping_rate =
                m_hardwareModel.noiseModel.computeAmplitudeDampingRate(gateTime,
                                                                       qubit);
            m_openQasm << "#pragma braket noise amplitude_damping("
                       << amplitude_damping_rate << ") $" << qubit << "\n";
            m_openQasm << "#pragma braket noise phase_damping("
                       << amplitude_damping_rate << ") $" << qubit << "\n";
          }
        }
      } 
    }
    else {
      if (!params.empty()) {
        m_openQasm << "(";
        for (int i = 0; i < params.size() - 1; ++i) {
          m_openQasm << params[i] << ", ";
        }
        m_openQasm << params[params.size() - 1] << ")";
      }
      m_openQasm << " ";
      for (int i = 0; i < operands.size() - 1; ++i) {
        m_openQasm << m_regName << "[" << operands[i] << "], ";
      }
      m_openQasm << m_regName << "[" << operands[operands.size() - 1] << "];\n";
      // Note: by default, noise channels are applied **AFTER** the gate.
      //std::cout << "m_noise is" << m_noise << "\n" ;
      //std::cout << "m_verbatim is" << m_verbatim << "\n" ;
      if (m_noise){
        if (gateName != "measure") {
          for (const auto &qubit : operands) {
            const auto amplitude_damping_rate =
                m_hardwareModel.noiseModel.computeAmplitudeDampingRate(gateTime,
                                                                       qubit);
            const auto phase_dammping_rate =
                m_hardwareModel.noiseModel.computeAmplitudeDampingRate(gateTime,
                                                                       qubit);
            m_openQasm << "#pragma braket noise amplitude_damping("
                       << amplitude_damping_rate << ") q[" << qubit << "]\n";
            m_openQasm << "#pragma braket noise phase_damping("
                       << amplitude_damping_rate << ") q[" << qubit << "]\n";
          }
        }
      }
    }
  }
};
}
