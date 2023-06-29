// Copyright (c) 2022 Quantum Brilliance Pty Ltd
#include "Algorithm.hpp"
#include "AlgorithmGradientStrategy.hpp"
#include "ObservableTransform.hpp"
#include "xacc.hpp"
#include "xacc_plugin.hpp"
#include "xacc_service.hpp"
#include <complex>
#include <iomanip>
#include <cassert>
#include <numeric>
namespace xacc {
namespace algorithm {
class VqeGen : public Algorithm {
public:
  bool initialize(const HeterogeneousMap &parameters) override {
    if (!parameters.pointerLikeExists<Observable>("observable")) {
      std::cout << "'observable' is required\n";
      return false;
    }

    if (!parameters.pointerLikeExists<CompositeInstruction>("ansatz")) {
      std::cout << "'ansatz' in required\n";
      return false;
    }

    if (!parameters.pointerLikeExists<Accelerator>("accelerator")) {
      std::cout << "'accelerator' is required\n";
      return false;
    }

    m_observable = parameters.getPointerLike<Observable>("observable");
    if (parameters.pointerLikeExists<Optimizer>("optimizer")) {
      m_optimizer = parameters.getPointerLike<Optimizer>("optimizer");
    }
    m_accelerator = parameters.getPointerLike<Accelerator>("accelerator");
    m_kernel = parameters.getPointerLike<CompositeInstruction>("ansatz");
    m_parameters = parameters;

    return true;
  }
  const std::vector<std::string> requiredParameters() const override {
    return {"ansatz", "accelerator", "observable"};
  }
  void execute(const std::shared_ptr<AcceleratorBuffer> buffer) const override {
    if (m_accelerator->name() != "qpp") {      
      throw std::range_error("To use direct expectation, you must use the qpp backend.");
    }
    
    auto ham_sparse = m_observable->name() != "pauli"
                          ? xacc::getService<xacc::ObservableTransform>("jw")
                                ->transform(xacc::as_shared_ptr(m_observable))
                                ->to_sparse_matrix()
                          : m_observable->to_sparse_matrix();
    std::vector<double> energies;
    OptFunction f(
        [&, this](const std::vector<double> &x, std::vector<double> &dx) {
          auto evaled = m_kernel->operator()(x);
          auto tmp_buffer = xacc::qalloc(buffer->size());
          // Step 1: execute the ansatz
          xacc::ScopeTimer qpuTimer("ansazt sim", false);
          m_accelerator->execute(tmp_buffer, evaled);
          //   std::cout << "Execute time = " << qpuTimer.getDurationMs()
          //             << " [ms]\n";
         
          xacc::ScopeTimer expCalcTimer("exp-val calc", false);
          const double energy = [&]() {
            if (m_accelerator->getExecutionInfo().key_exists_any_type(
                    xacc::ExecutionInfo::WaveFuncKey)) {
              auto waveFn =
                  m_accelerator
                      ->getExecutionInfo<xacc::ExecutionInfo::WaveFuncPtrType>(
                          xacc::ExecutionInfo::WaveFuncKey);
              //   for (const auto &elem : *waveFn) {
              //     std::cout << elem << "\n";
              //   }
              return computeExpVal(ham_sparse, *waveFn);
            } else if (m_accelerator->getExecutionInfo().key_exists_any_type(
                           xacc::ExecutionInfo::DmKey)) {
              auto dmMat = m_accelerator->getExecutionInfo<
                  xacc::ExecutionInfo::DensityMatrixPtrType>(
                  xacc::ExecutionInfo::DmKey);
              return computeExpVal(ham_sparse, *dmMat);
            } else {
              xacc::error("Could not retrieve the state vector or density "
                          "matrix data.");
              return 0.0;
            }
          }();
          //   std::cout << "Energy evaluation time = " <<
          //   qpuTimer.getDurationMs()
          //             << " [ms]\n";
          std::stringstream ss;
          ss << "E(" << (!x.empty() ? std::to_string(x[0]) : "");
          for (int i = 1; i < x.size(); i++)
            ss << "," << x[i];
          ss << ") = " << std::setprecision(12) << energy;
          xacc::info(ss.str());
          // Saves the energy value.
          energies.emplace_back(energy);
          // Append a child buffer to save the value of x
          auto itenBuffer = xacc::qalloc(buffer->size());
          itenBuffer->setName("parameters_at_iter");
          itenBuffer->addExtraInfo("parameters", x);
          buffer->appendChild("parameters_at_iter", itenBuffer);

          if (m_optimizer->isGradientBased()) {
            constexpr double step_size = 1e-7;
            std::vector<double> plus_results, minus_results;
            for (int paramIdx = 0; paramIdx < x.size(); ++paramIdx) {
              auto x_new = x;
              x_new[paramIdx] = x[paramIdx] + step_size;
              auto evaled_plus = m_kernel->operator()(x_new);
              auto tmp_buffer_plus = xacc::qalloc(buffer->size());

              m_accelerator->execute(tmp_buffer_plus, evaled_plus);
              if (m_accelerator->getExecutionInfo().key_exists_any_type(
                      xacc::ExecutionInfo::WaveFuncKey)) {
                auto waveFnPlus = m_accelerator->getExecutionInfo<
                    xacc::ExecutionInfo::WaveFuncPtrType>(
                    xacc::ExecutionInfo::WaveFuncKey);
                const auto energy_plus = computeExpVal(ham_sparse, *waveFnPlus);

                plus_results.emplace_back(energy_plus);
              } else if (m_accelerator->getExecutionInfo().key_exists_any_type(
                             xacc::ExecutionInfo::DmKey)) {
                auto dmPlus = m_accelerator->getExecutionInfo<
                    xacc::ExecutionInfo::DensityMatrixPtrType>(
                    xacc::ExecutionInfo::DmKey);
                const auto energy_plus = computeExpVal(ham_sparse, *dmPlus);

                plus_results.emplace_back(energy_plus);
              } else {
                xacc::error("Could not retrieve the state vector or density "
                            "matrix data.");
              }

              x_new[paramIdx] = x[paramIdx] - step_size;
              auto evaled_minus = m_kernel->operator()(x_new);
              auto tmp_buffer_minus = xacc::qalloc(buffer->size());

              m_accelerator->execute(tmp_buffer_minus, evaled_minus);

              if (m_accelerator->getExecutionInfo().key_exists_any_type(
                      xacc::ExecutionInfo::WaveFuncKey)) {
                auto waveFnMinus = m_accelerator->getExecutionInfo<
                    xacc::ExecutionInfo::WaveFuncPtrType>(
                    xacc::ExecutionInfo::WaveFuncKey);
                const auto energy_minus =
                    computeExpVal(ham_sparse, *waveFnMinus);

                minus_results.emplace_back(energy_minus);
              } else if (m_accelerator->getExecutionInfo().key_exists_any_type(
                             xacc::ExecutionInfo::DmKey)) {
                auto dmMinus = m_accelerator->getExecutionInfo<
                    xacc::ExecutionInfo::DensityMatrixPtrType>(
                    xacc::ExecutionInfo::DmKey);
                const auto energy_minus = computeExpVal(ham_sparse, *dmMinus);

                minus_results.emplace_back(energy_minus);
              } else {
                xacc::error("Could not retrieve the state vector or density "
                            "matrix data.");
              }
            }
            assert(plus_results.size() == x.size());
            assert(minus_results.size() == x.size());
            for (int paramIdx = 0; paramIdx < x.size(); ++paramIdx) {
              dx[paramIdx] =
                  (plus_results[paramIdx] - minus_results[paramIdx]) /
                  (2.0 * step_size);
              //   std::cout << "dx[" << paramIdx << "] = " << dx[paramIdx] <<
              //   "\n";
            }
          }
          return energy;
        },
        m_kernel->nVariables());
    auto result = m_optimizer->optimize(f);

    buffer->addExtraInfo("opt-val", ExtraInfo(result.first));
    buffer->addExtraInfo("opt-params", ExtraInfo(result.second));
    buffer->addExtraInfo("params-energy", ExtraInfo(energies));
  }
  std::vector<double> execute(const std::shared_ptr<AcceleratorBuffer> buffer,
                              const std::vector<double> &parameters) override {
    return {};
  }
  const std::string name() const override { return "vqe-gen"; }
  const std::string description() const override { return ""; }
  DEFINE_ALGORITHM_CLONE(VqeGen)
private:
  double computeExpVal(std::vector<SparseTriplet> &ham_mat,
                       const std::vector<std::complex<double>> &ket) const {

    const auto nbBits = [](unsigned int ket_size) -> unsigned int {
      if (ket_size == 1) {
        return 0;
      }
      unsigned int ret = 0;
      while (ket_size > 1) {
        ket_size >>= 1;
        ret++;
      }
      return ret;
    };
    const auto reverseBits = [](unsigned int n, size_t nbBits) {
      unsigned int rev = 0;
      for (size_t i = 0; i < nbBits; ++i) {
        if ((n & (1 << i)) == (1 << i)) {
          rev ^= (1 << (nbBits - i - 1));
        }
      }
      return rev;
    };
    std::vector<std::complex<double>> ham_ket(ket.size(), 0.0);
    const auto nbQubits = nbBits(ket.size());
#pragma omp for
    for (size_t i = 0; i < ham_mat.size(); ++i) {
      auto &triplet = ham_mat[i];
      const auto row = reverseBits(triplet.row(), nbQubits);
      //   std::cout << "row = " << row << " from " << triplet.row() << "\n";
      const auto col = reverseBits(triplet.col(), nbQubits);
      const auto val = triplet.coeff();
      //   std::cout << "(" << row << ", " << col << ") = " << triplet.coeff()
      //             << "\n";
      ham_ket[row] += (val * ket[col]);
    }
    std::complex<double> exp_val = 0.0;
    for (int i = 0; i < ket.size(); ++i) {
      exp_val += (std::conj(ket[i]) * ham_ket[i]);
    }
    // std::cout << "Exp-val = " << exp_val << "\n";
    return exp_val.real();
  }


  double computeExpVal(std::vector<SparseTriplet> &ham_mat,
                       const std::vector<std::vector<std::complex<double>>> &dm) const {

    const auto nbBits = [](unsigned int ket_size) -> unsigned int {
      if (ket_size == 1) {
        return 0;
      }
      unsigned int ret = 0;
      while (ket_size > 1) {
        ket_size >>= 1;
        ret++;
      }
      return ret;
    };
    const auto reverseBits = [](unsigned int n, size_t nbBits) {
      unsigned int rev = 0;
      for (size_t i = 0; i < nbBits; ++i) {
        if ((n & (1 << i)) == (1 << i)) {
          rev ^= (1 << (nbBits - i - 1));
        }
      }
      return rev;
    };
    const auto nbQubits = nbBits(dm.size());
    std::vector<std::complex<double>> diag_elements(dm.size(), 0.0);
#pragma omp for
    for (size_t i = 0; i < ham_mat.size(); ++i) {
      auto &triplet = ham_mat[i];
      const auto row = reverseBits(triplet.row(), nbQubits);
      //   std::cout << "row = " << row << " from " << triplet.row() << "\n";
      const auto col = reverseBits(triplet.col(), nbQubits);
      const auto val = triplet.coeff();
      //   std::cout << "(" << row << ", " << col << ") = " << triplet.coeff()
      //             << "\n";
      diag_elements[row] += (val * dm[col][row]);
    }
    const std::complex<double> exp_val = std::accumulate(
        diag_elements.begin(), diag_elements.end(), std::complex<double>(0.0));
    // std::cout << "Exp-val = " << exp_val << "\n";
    return exp_val.real();
  }

private:
  Observable *m_observable;
  Optimizer *m_optimizer;
  CompositeInstruction *m_kernel;
  Accelerator *m_accelerator;
  HeterogeneousMap m_parameters;
};
} // namespace algorithm
} // namespace xacc
REGISTER_ALGORITHM(xacc::algorithm::VqeGen)