// Copyright (c) 2022 Quantum Brilliance Pty Ltd

#include "qristal/core/noise_model/noise_model.hpp"
#include <dlfcn.h>
#include <vector>
#include <algorithm>
#include <unsupported/Eigen/KroneckerProduct>


namespace {
// Kron combine 2 noise channels
qristal::NoiseChannel noise_channel_expand(const qristal::NoiseChannel& noise_channel_1,
                                      const qristal::NoiseChannel& noise_channel_2) {
  qristal::NoiseChannel combined_channel;
  const auto mat_to_eigen =
      [](const qristal::KrausOperator::Matrix& mat) -> Eigen::MatrixXcd {
    Eigen::MatrixXcd eigen_mat(mat.size(), mat[0].size());
    for (int row = 0; row < eigen_mat.rows(); ++row) {
      for (int col = 0; col < eigen_mat.cols(); ++col) {
        eigen_mat(row, col) = mat[row][col];
      }
    }
    return eigen_mat;
  };

  const auto eigen_to_mat =
      [](const Eigen::MatrixXcd& eigen_mat) -> qristal::KrausOperator::Matrix {
    qristal::KrausOperator::Matrix mat(eigen_mat.rows());
    for (int row = 0; row < eigen_mat.rows(); ++row) {
      std::vector<std::complex<double>> row_data(eigen_mat.cols());
      for (int col = 0; col < eigen_mat.cols(); ++col) {
        row_data[col] = eigen_mat(row, col);
      }
      mat[row] = row_data;
    }
    return mat;
  };

  for (const auto& kraus1 : noise_channel_1) {
    for (const auto& kraus2 : noise_channel_2) {
      auto qubits = kraus1.qubits;
      qubits.insert(qubits.end(), kraus2.qubits.begin(), kraus2.qubits.end());
      // Check that the qubits from the two Kraus operators are different
      if (std::unique(qubits.begin(), qubits.end()) != qubits.end()) {
        throw std::runtime_error(
            "Cannot Kronecker combine two Kraus operators operating on the "
            "same qubit.");
      }
      Eigen::MatrixXcd expanded_mat = Eigen::kroneckerProduct(
          mat_to_eigen(kraus1.matrix), mat_to_eigen(kraus2.matrix));
      qristal::KrausOperator expanded_kraus_op;
      expanded_kraus_op.qubits = qubits;
      expanded_kraus_op.matrix = eigen_to_mat(expanded_mat);
      combined_channel.emplace_back(expanded_kraus_op);
    }
  }
  return combined_channel;
}

}
namespace qristal
{
    NoiseModel::NoiseModel(const nlohmann::json &js) : m_qobj_noise_model(js)
    {
        // TODO: parse qubit connectivity topology from qObj
    }

    NoiseModel::NoiseModel(const NoiseProperties &noise_props)
    {
        // Get qubit connectivity topology
        m_qubit_topology = noise_props.qubit_topology;

        // Add readout errors
        m_readout_errors = noise_props.readout_errors;

        // Thermal noise:
        for (const auto &[gate_name, operands_to_durations] : noise_props.gate_time_us)
        {
            for (const auto &[qubits, gate_duration] : operands_to_durations)
            {
                // Retrieve gate error (e.g., from randomized benchmarking) if any
                const double rb_pauli_rate = [&noise_props](const std::string &gate_name, const std::vector<size_t> &qubits)
                {
                    const auto iter1 = noise_props.gate_pauli_errors.find(gate_name);
                    if (iter1 != noise_props.gate_pauli_errors.end())
                    {
                        auto &operands_to_error_rates = iter1->second;
                        const auto iter2 = operands_to_error_rates.find(qubits);
                        if (iter2 != operands_to_error_rates.end())
                        {
                            return iter2->second;
                        }
                    }
                    return 0.0;
                }(gate_name, qubits);

                for (const auto &qubit : qubits)
                {
                    const auto t1_iter = noise_props.t1_us.find(qubit);
                    const auto t2_iter = noise_props.t2_us.find(qubit);
                    assert(t1_iter != noise_props.t1_us.end());
                    assert(t2_iter != noise_props.t2_us.end());
                    const auto t1 = t1_iter->second;
                    const auto t2 = t2_iter->second;
                    const double amp_damp_rate = 1.0 - std::exp(-gate_duration / t1);
                    const double phase_damp_rate = 1.0 - std::exp(-gate_duration / t2);

                    // 1-qubit gate error
                    auto thermal_relaxation = GeneralizedPhaseAmplitudeDampingChannel::Create(qubit, 0.0, amp_damp_rate, phase_damp_rate);
                    add_gate_error(thermal_relaxation, gate_name, qubits);

                    // Adds depolarizing noise if needed.
                    const double equiv_pauli_rate = decoherence_pauli_error(t1, t2, gate_duration);
                    if (rb_pauli_rate > equiv_pauli_rate)
                    {
                        const double p_depol = rb_pauli_rate - equiv_pauli_rate;
                        add_gate_error(DepolarizingChannel::Create(qubit, p_depol), gate_name, {qubit});
                    }

                    // 2-qubit gate error
                    if (qubits.size() == 2)
                    {
                        const double gate_error = rb_pauli_rate;
                        auto thermal_relaxation_q1 = GeneralizedPhaseAmplitudeDampingChannel::Create(qubits[0], 0.0, amp_damp_rate, phase_damp_rate);
                        auto thermal_relaxation_q2 = GeneralizedPhaseAmplitudeDampingChannel::Create(qubits[1], 0.0, amp_damp_rate, phase_damp_rate);
                        auto expanded_thermal_relax_channel = noise_channel_expand(thermal_relaxation_q1, thermal_relaxation_q2);
                        add_gate_error(expanded_thermal_relax_channel, gate_name, {qubits[0], qubits[1]});
                        add_gate_error(DepolarizingChannel::Create(qubits[0], qubits[1], gate_error), gate_name, {qubits[0], qubits[1]});
                        // Adding symmetric noise for both directions: i.e., cz q1, q2 has the same noise as cz q2, q1
                        add_gate_error(expanded_thermal_relax_channel, gate_name, {qubits[1], qubits[0]});
                        add_gate_error(DepolarizingChannel::Create(qubits[0], qubits[1], gate_error), gate_name, {qubits[1], qubits[0]});
                    }
                }
            }
        }
    }

    // Build and return a registered noise model.
    NoiseModel::NoiseModel(const std::string& name,
                           size_t nb_qubits,
                           std::optional<QubitConnectivity> connectivity,
                           std::optional<std::reference_wrapper<const std::vector<std::pair<size_t, size_t>>>> connected_pairs)
    {
        // If the default model has been requested, just load it and be done
        if (name == "default")
        {
            if (connected_pairs != std::nullopt) make_default(*this, nb_qubits, *connectivity, *connected_pairs);
            else if (connectivity != std::nullopt) make_default(*this, nb_qubits, *connectivity);
            else make_default(*this, nb_qubits);
            return;
        }

        // Otherwise, we try to load the model from the emulator.
        static const char *EMULATOR_NOISE_MODEL_LIB_NAME = "libqristal_emulator.so";
        void *handle = dlopen(EMULATOR_NOISE_MODEL_LIB_NAME, RTLD_LOCAL | RTLD_LAZY | RTLD_DEEPBIND);
        if (!handle)
        {
            char *error_msg = dlerror();
            throw std::runtime_error("Failed to load noise model " + name + ".\n"
                                     "This model is not present in the Qristal SDK, and Qristal failed to load the noise modelling "
                                     "library from the Qristal Emulator.\n" + (error_msg ? std::string(error_msg) : ""));
        }

        // Clear all errors
        dlerror();
        // Attempt to get the function pointer from the shared library
        auto get_emulator_noise_model = reinterpret_cast<NoiseModel*(*)(const char*)>(dlsym(handle, "get_emulator_noise_model"));
        // Check that nothing went wrong
        char* error_msg = dlerror();
        // Encountered an error:
        if (error_msg)
        {
            throw std::runtime_error("Failed to load get_emulator_noise_model from " +
                                     std::string(EMULATOR_NOISE_MODEL_LIB_NAME)+ ".\n" + std::string(error_msg) + ".");
        }
        // Get the noise model
        NoiseModel* model = get_emulator_noise_model(name.c_str());
        // Check that a valid result was returned.
        if (model == nullptr)
        {
            throw std::runtime_error("The noise model " + name + " exists in neither the Qristal SDK nor the Qristal Emulator.");
        }
        model->name = name;
        *this = *model;
    }

    std::string NoiseModel::get_qobj_compiler() const {
      return m_qobj_compiler;
    }

    void NoiseModel::set_qobj_compiler(const std::string &qobj_compiler) {
      const std::vector<std::string> SUPPORTED_QOBJ_COMPILERS = {"qristal-qobj",
                                                                 "xacc-qobj"};
      // Check if this is something we supported/exposed in Qristal.
      if (std::find(SUPPORTED_QOBJ_COMPILERS.begin(),
                    SUPPORTED_QOBJ_COMPILERS.end(),
                    qobj_compiler) == SUPPORTED_QOBJ_COMPILERS.end()) {
        std::stringstream error_msg;
        error_msg << "Invalid qobj_compiler. The followings are supported:\n";
        for (const auto &name : SUPPORTED_QOBJ_COMPILERS) {
          error_msg << " - " << name << "\n";
        }
        throw std::invalid_argument(error_msg.str());
      }
      m_qobj_compiler = qobj_compiler;
    }

    std::vector<std::string> NoiseModel::get_qobj_basis_gates() const {
      if (m_qobj_compiler == "qristal-qobj") {
        return {"rx", "ry", "cz"};
      }

      assert(m_qobj_compiler == "xacc-qobj");
      // The default IBM's QObj gate set.
      return {"u1", "u2", "u3", "cx"};
    }

    noise_aware_placement_config NoiseModel::to_noise_aware_placement_config() const
    {
      noise_aware_placement_config config;
      // Populate connectivity info
      for (const auto &[q1, q2] : get_connectivity()) {
        config.qubit_connectivity.emplace_back(
            std::make_pair(static_cast<size_t>(q1), static_cast<size_t>(q2)));
      }

      // Populate readout errors
      // Average readout error =  (p(0|1) + p(1|0))/2
      for (const auto &[qId, readout_error] : get_readout_errors()) {
        config.avg_qubit_readout_errors.emplace(
            qId, (readout_error.p_01 + readout_error.p_10) / 2.0);
      }

      // Map from qubit index -> list of single-qubit gate errors for averaging.
      std::unordered_map<size_t, std::vector<double>> qubit_id_to_gate_errors;
      // Iterate over the whole noise channel map
      for (const auto &[gate_name, operands_to_noise_channels] : get_noise_channels()) {
        for (const auto &[qubits, noise_channels] : operands_to_noise_channels) {
          // We only expect single- or two-qubit gates here
          assert(qubits.size() == 1 || qubits.size() == 2);
          if (qubits.size() == 2) {
            if ((std::find(config.qubit_connectivity.begin(),
                           config.qubit_connectivity.end(),
                           std::make_pair(qubits[0], qubits[1])) ==
                 config.qubit_connectivity.end()) &&
                (std::find(config.qubit_connectivity.begin(),
                           config.qubit_connectivity.end(),
                           std::make_pair(qubits[1], qubits[0])) ==
                 config.qubit_connectivity.end())) {
              std::stringstream error_ss;
              error_ss << "Invalid noise specifications for two-qubit gate '"
                       << gate_name << "' between **uncoupled** qubits "
                       << qubits[0] << " and " << qubits[1]
                       << ". Please check your input noise model.";
              throw std::logic_error(error_ss.str());
            }
            double fid = 1.0;
            /// Combine fidelity if there are multiple channels
            for (const auto &chan : noise_channels) {
              fid *= process_fidelity(chan);
            }
            /// Note: we only expect a single type of two-qubit gate.
            config.avg_two_qubit_gate_errors[std::make_pair(qubits[0], qubits[1])] = 1.0 - fid;
          } else if (qubits.size() == 1) {
            double fid = 1.0;
            for (const auto &chan : noise_channels) {
              fid *= process_fidelity(chan);
            }
            qubit_id_to_gate_errors[qubits[0]].emplace_back(1.0 - fid);
          }
        }
      }

      /// Average all single-qubit gate errors at a qubit node.
      for (const auto &[qubit, gate_errors] : qubit_id_to_gate_errors) {
        assert(!gate_errors.empty());
        config.avg_single_qubit_gate_errors[qubit] = std::reduce(gate_errors.begin(), gate_errors.end()) / gate_errors.size();
      }

      return config;
    }

    double NoiseModel::decoherence_pauli_error(double t1, double tphi, double gate_time)
    {
        // Formula:
        // (1/4) * [1 - e^(-t/T1)] + (1/2) * [1 - e^(-t/(2*T1) - t/Tphi]
        const auto gamma_2 = (1 / (2 * t1)) + 1 / tphi;
        const auto exp1 = std::exp(-gate_time / t1);
        const auto exp2 = std::exp(-gate_time * gamma_2);
        const auto px = 0.25 * (1 - exp1);
        const auto py = px;
        const auto pz = 0.5 * (1 - exp2) - px;
        return px + py + pz;
    }

    void NoiseModel::add_gate_error(const NoiseChannel &noise_channel, const std::string &gate_name, const std::vector<size_t> &qubits)
    {
        auto iter = m_noise_channels.find(gate_name);
        if (iter != m_noise_channels.end())
        {
            auto operands_to_noise_map = iter->second;
            auto iter2 = operands_to_noise_map.find(qubits);
            if (iter2 != operands_to_noise_map.end())
            {
                auto noise_channels =  iter2->second;
                noise_channels.emplace_back(noise_channel);
                m_noise_channels[gate_name][qubits] = noise_channels;
            }
            else
            {
                m_noise_channels[gate_name][qubits] = {noise_channel};
            }
        }
        else
        {
            std::map<std::vector<size_t>, std::vector<NoiseChannel>> gate_noise_map;
            gate_noise_map[qubits] = {noise_channel};
            // First time seeing this gate
            m_noise_channels.emplace(gate_name, std::move(gate_noise_map));
        }
    }

    void NoiseModel::set_qubit_readout_error(size_t qubitIdx, const ReadoutError &ro_error)
    {
        m_readout_errors[qubitIdx] = ro_error;
    }

    std::string NoiseModel::to_json() const
    {
        if (!m_qobj_noise_model.empty())
        {
            // If the noise model was provided as a json object, e.g., externally built by qiskit,
            // just use it as is.
            return m_qobj_noise_model.dump(6);
        }

        // Build the noise model QObj Json
        nlohmann::json noiseModel;
        std::vector<nlohmann::json> noiseElements;
        // Readout errors
        for (const auto &[qIdx, ro_error] : m_readout_errors)
        {
            const auto meas0Prep1 = ro_error.p_01;
            const auto meas1Prep0 = ro_error.p_10;

            const std::vector<std::vector<double>> probs{
                {1 - meas1Prep0, meas1Prep0}, {meas0Prep1, 1 - meas0Prep1}};
            nlohmann::json element;
            element["type"] = "roerror";
            element["operations"] = std::vector<std::string>{"measure"};
            element["probabilities"] = probs;
            element["gate_qubits"] = std::vector<std::vector<std::size_t>>{{qIdx}};
            noiseElements.push_back(element);
        }

        // Gate errors
        for (const auto &[gateName, channel_map] : m_noise_channels)
        {
            for (const auto &[qubits, noise_channels] : channel_map)
            {
                nlohmann::json element;
                element["type"] = "qerror";
                element["operations"] = std::vector<std::string>{gateName};
                element["gate_qubits"] = std::vector<std::vector<std::size_t>>{qubits};
                std::vector<nlohmann::json> instructions;
                for (const auto &noise_channel : noise_channels)
                {
                    nlohmann::json instruction;
                    instruction["name"] = "kraus";
                    auto kraus_mats = nlohmann::json::array();
                    for (const auto &kraus_op : noise_channel)
                    {
                        kraus_mats.push_back(kraus_op.matrix);
                        std::vector<int> kraus_qubits(kraus_op.qubits.size());
                        std::iota(kraus_qubits.begin(), kraus_qubits.end(), 0);
                        instruction["qubits"] = kraus_qubits;
                    }
                    instruction["params"] = kraus_mats;
                    instructions.emplace_back(instruction);
                }
                assert(instructions.size() == noise_channels.size());
                element["instructions"] =
                    std::vector<std::vector<nlohmann::json>>{instructions};
                element["probabilities"] = std::vector<double>(1, 1.0);
                noiseElements.push_back(element);
            }
        }
        noiseModel["errors"] = noiseElements;
        return noiseModel.dump(6);
    }

    std::vector<std::pair<int, int>> NoiseModel::get_connectivity() const
    {
        return m_qubit_topology;
    }

    const std::unordered_map<size_t, ReadoutError> &NoiseModel::get_readout_errors() const
    {
        return m_readout_errors;
    }

    const std::unordered_map<std::string, std::map<std::vector<size_t>, std::vector<NoiseChannel>>> &NoiseModel::get_noise_channels() const
    {
        return m_noise_channels;
    }

    void NoiseModel::add_qubit_connectivity(int q1, int q2)
    {
        m_qubit_topology.emplace_back(std::make_pair(q1, q2));
    }
}
