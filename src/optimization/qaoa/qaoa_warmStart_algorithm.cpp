/// Modifications Copyright (c) Quantum Brilliance.
///
/// This file is adapted from one that is part of XACC:
/// https://github.com/eclipse/xacc/blob/master/quantum/plugins/algorithms/qaoa/qaoa.cpp

/*******************************************************************************
 * Copyright (c) 2019 UT-Battelle, LLC.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompanies this
 * distribution. The Eclipse Public License is available at
 * http://www.eclipse.org/legal/epl-v10.html and the Eclipse Distribution
 *License is available at https://eclipse.org/org/documents/edl-v10.php
 *
 * Contributors:
 *   Thien Nguyen - initial API and implementation
 *******************************************************************************/

#include "qristal/core/optimization/qaoa/qaoa_warmStart_algorithm.hpp"

#include "xacc.hpp"
#include "Circuit.hpp"
#include "xacc_service.hpp"
#include "PauliOperator.hpp"
#include "xacc_observable.hpp"
#include "CompositeInstruction.hpp"
#include "AlgorithmGradientStrategy.hpp"
#include "IRTransformation.hpp"
#include <cassert>
#include <iomanip>

namespace xacc::algorithm {

  bool WS_QAOA::initialize(const HeterogeneousMap &parameters) {
    bool initializeOk = true;
    // Hyper-parameters for QAOA:
    // (1) Accelerator (QPU)
    if (!parameters.pointerLikeExists<Accelerator>("accelerator")) {
      std::cout << "'accelerator' is required.\n";
      // We check all required params; hence don't early return on failure.
      initializeOk = false;
    }

    // (2) Classical optimizer
    if (!parameters.pointerLikeExists<Optimizer>("optimizer")) {
      std::cout << "'optimizer' is required.\n";
      initializeOk = false;
    }

    // (3) Number of mixing and cost function steps to use (default = 1)
    m_nbSteps = 1;
    if (parameters.keyExists<int>("steps")) {
      m_nbSteps = parameters.get<int>("steps");
    }

    // (4) Cost Hamiltonian to construct the max-cut cost Hamiltonian from.
    if (!parameters.pointerLikeExists<Observable>("observable")) {
          std::cout << "'observable' is required.\n";
          initializeOk = false;
      }

    // (5) Good cut
    if (parameters.keyExists<std::string>("good_cut")) {
        good_cut = parameters.get<std::string>("good_cut");
        //std::cout << "Here is our good cut " << good_cut << std::endl;
    }

    // Default is Extended ParameterizedMode (less steps, more params)
    m_parameterizedMode = "Extended";
    if (parameters.stringExists("parameter-scheme")) {
      m_parameterizedMode = parameters.getString("parameter-scheme");
    }

    if (initializeOk) {
      m_costHamObs = parameters.getPointerLike<Observable>("observable");
      m_qpu = parameters.getPointerLike<Accelerator>("accelerator");
      m_optimizer = parameters.getPointerLike<Optimizer>("optimizer");
      }
      m_irTransformation = nullptr;
      // This QPU has topology-constraint
      if (!m_qpu->getConnectivity().empty()) {
        if (parameters.pointerLikeExists<xacc::IRTransformation>("placement")) {
          m_irTransformation = xacc::as_shared_ptr(
              parameters.getPointerLike<xacc::IRTransformation>("placement"));
          if (m_irTransformation->type() !=
              xacc::IRTransformationType::Placement) {
            xacc::error(m_irTransformation->name() +
                        " is not a placement service.");
          }
        }
      }

    // we need this for ADAPT-QAOA (Daniel)
    if (parameters.pointerLikeExists<CompositeInstruction>("ansatz")) {
      externalAnsatz =
          parameters.get<std::shared_ptr<CompositeInstruction>>("ansatz");
    }

    if (parameters.pointerLikeExists<AlgorithmGradientStrategy>(
            "gradient_strategy")) {
      gradientStrategy =
          parameters.get<std::shared_ptr<AlgorithmGradientStrategy>>(
              "gradient_strategy");
    }

    if (parameters.stringExists("gradient_strategy") &&
        !parameters.pointerLikeExists<AlgorithmGradientStrategy>(
            "gradient_strategy") &&
        m_optimizer->isGradientBased()) {
      gradientStrategy = xacc::getService<AlgorithmGradientStrategy>(
          parameters.getString("gradient_strategy"));
      gradientStrategy->initialize(parameters);
    }

    if ((parameters.stringExists("gradient_strategy") ||
         parameters.pointerLikeExists<AlgorithmGradientStrategy>(
             "gradient_strategy")) &&
        !m_optimizer->isGradientBased()) {
      xacc::warning(
          "Chosen optimizer does not support gradients. Using default.");
    }

    if (parameters.keyExists<bool>("maximize")) {
        m_maximize = parameters.get<bool>("maximize");
    }

    m_shuffleTerms = false;
    if (parameters.keyExists<bool>("shuffle-terms")) {
      m_shuffleTerms = parameters.get<bool>("shuffle-terms");
    }

    if (m_optimizer && m_optimizer->isGradientBased() &&
        gradientStrategy == nullptr) {
      // No gradient strategy was provided, just use autodiff.
      gradientStrategy = xacc::getService<AlgorithmGradientStrategy>("autodiff");
      gradientStrategy->initialize(
          {{"observable", xacc::as_shared_ptr(m_costHamObs)}});
    }
    return initializeOk;
  }

  const std::vector<std::string> WS_QAOA::requiredParameters() const {
    return {"accelerator", "optimizer", "observable", "good_cut"};
  }

  void WS_QAOA::execute(const std::shared_ptr<AcceleratorBuffer> buffer) const {
    const int nbQubits = buffer->size();
    // we need this for ADAPT-QAOA (Daniel)
    std::shared_ptr<CompositeInstruction> kernel;
    if (externalAnsatz) {
      kernel = externalAnsatz;
    } else {
        HeterogeneousMap m;
        kernel = std::dynamic_pointer_cast<CompositeInstruction>(
            xacc::getService<Instruction>("ws_qaoa"));
          m.insert("nbQubits", nbQubits);
          m.insert("nbSteps", m_nbSteps);
          m.insert("good_cut", good_cut);
          m.insert("cost-ham", m_costHamObs);
          m.insert("parameter-scheme", m_parameterizedMode);
          m.insert("shuffle-terms", m_shuffleTerms);
        kernel->expand(m);
    }

    // Handle Max-cut optimization on shots-based backends (including physical
    // backends). We only want to execute a single circuit for observable with all
    // commuting terms such as the maxcut Hamiltonian.
    // Limitation: this grouping cannot handle gradient strategy at the moment.
    // Observe the cost Hamiltonian:
    auto getObservedKernels = [&] (const std::vector<double> x = {}) {
      auto evaled = x.empty() ? kernel : kernel->operator()(x);
      if (dynamic_cast<xacc::quantum::PauliOperator *>(m_costHamObs)) {
        return m_costHamObs->observe(evaled, {{"accelerator", m_qpu}});
      } else {
        return m_costHamObs->observe(evaled);
      }
    };

    // Grouping is possible (no gradient strategy)
    // TODO: Gradient strategy to handle grouping as well.
    int iterCount = 0;
    if (m_costHamObs->getNonIdentitySubTerms().size() > 1 &&
        getObservedKernels().size() == 1 && !gradientStrategy) {
      OptFunction f(
          [&, this](const std::vector<double> &x, std::vector<double> &dx) {
            auto tmpBuffer = xacc::qalloc(buffer->size());
            std::vector<std::shared_ptr<CompositeInstruction>> fsToExec{
                getObservedKernels()[0]->operator()(x)};
            if (m_irTransformation) {
              for (auto &composite : fsToExec) {
                m_irTransformation->apply(
                    composite, xacc::as_shared_ptr<xacc::Accelerator>(m_qpu));
              }
            }
            m_qpu->execute(tmpBuffer, fsToExec);
            double energy = m_costHamObs->postProcess(tmpBuffer);
            // We will only have one child buffer for each parameter set.
            assert(tmpBuffer->getChildren().size() == 1);
            auto result_buf = tmpBuffer->getChildren()[0];
            result_buf->addExtraInfo("parameters", x);
            result_buf->addExtraInfo("energy", energy);
            buffer->appendChild("Iter" + std::to_string(iterCount), result_buf);

            std::stringstream ss;

            ss << "Iter " << iterCount << ": E("
               << (!x.empty() ? std::to_string(x[0]) : "");
            for (int i = 1; i < x.size(); i++) {
              ss << "," << std::setprecision(3) << x[i];
              if (i > 4) {
                // Don't print too many params
                ss << ", ...";
                break;
              }
            }
            ss << ") = " << std::setprecision(12) << energy;
            xacc::info(ss.str());
            iterCount++;
            if (m_maximize)
              energy *= -1.0;
            return energy;
          }, kernel->nVariables());
      auto result = m_optimizer->optimize(f);
      // Reports the final cost:
      double finalCost = result.first;
      if (m_maximize)
        finalCost *= -1.0;
      buffer->addExtraInfo("opt-val", ExtraInfo(finalCost));
      buffer->addExtraInfo("opt-params", ExtraInfo(result.second));
      return;
    }

    // Construct the optimizer/minimizer:
    OptFunction f(
        [&, this](const std::vector<double> &x, std::vector<double> &dx) {
          std::vector<double> coefficients;
          std::vector<std::string> kernelNames;
          std::vector<std::shared_ptr<CompositeInstruction>> fsToExec;

          double identityCoeff = 0.0;
          int nInstructionsEnergy = 0, nInstructionsGradient = 0;
          for (auto &f : getObservedKernels(x)) {
            kernelNames.push_back(f->name());
            std::complex<double> coeff = f->getCoefficient();

            int nFunctionInstructions = 0;
            if (f->getInstruction(0)->isComposite()) {
              nFunctionInstructions =
                  kernel->nInstructions() + f->nInstructions() - 1;
            } else {
              nFunctionInstructions = f->nInstructions();
            }

            if (nFunctionInstructions > kernel->nInstructions()) {
              fsToExec.push_back(f);
              coefficients.push_back(std::real(coeff));
            } else {
              identityCoeff += std::real(coeff);
            }
          }

          // enables gradients (Daniel)
          if (gradientStrategy) {

            auto gradFsToExec =
                gradientStrategy->getGradientExecutions(kernel, x);
            // Add gradient instructions to be sent to the qpu
            nInstructionsEnergy = fsToExec.size();
            nInstructionsGradient = gradFsToExec.size();
            for (auto inst : gradFsToExec) {
              fsToExec.push_back(inst);
            }
            xacc::info("Number of instructions for energy calculation: " +
                       std::to_string(nInstructionsEnergy));
            xacc::info("Number of instructions for gradient calculation: " +
                       std::to_string(nInstructionsGradient));
          }

          auto tmpBuffer = xacc::qalloc(buffer->size());
          if (m_irTransformation) {
            for (auto &composite : fsToExec) {
              m_irTransformation->apply(
                  composite, xacc::as_shared_ptr<xacc::Accelerator>(m_qpu));
            }
          }
          m_qpu->execute(tmpBuffer, fsToExec);
          auto buffers = tmpBuffer->getChildren();

          double energy = identityCoeff;
          auto idBuffer = xacc::qalloc(buffer->size());
          idBuffer->addExtraInfo("coefficient", identityCoeff);
          idBuffer->setName("I");
          idBuffer->addExtraInfo("kernel", "I");
          idBuffer->addExtraInfo("parameters", x);
          idBuffer->addExtraInfo("exp-val-z", 1.0);
          buffer->appendChild("I", idBuffer);

          if (gradientStrategy) { // gradient-based optimization

            for (int i = 0; i < nInstructionsEnergy; i++) { // compute energy
              auto expval = buffers[i]->getExpectationValueZ();
              energy += expval * coefficients[i];
              buffers[i]->addExtraInfo("coefficient", coefficients[i]);
              buffers[i]->addExtraInfo("kernel", fsToExec[i]->name());
              buffers[i]->addExtraInfo("exp-val-z", expval);
              buffers[i]->addExtraInfo("parameters", x);
              buffer->appendChild(fsToExec[i]->name(), buffers[i]);
            }

            std::stringstream ss;
            ss << std::setprecision(12) << "Current Energy: " << energy;
            xacc::info(ss.str());
            ss.str(std::string());

            // If gradientStrategy is numerical, pass the energy
            // We subtract the identityCoeff from the energy
            // instead of passing the energy because the gradients
            // only take the coefficients of parameterized instructions
            if (gradientStrategy->isNumerical()) {
              gradientStrategy->setFunctionValue(energy - identityCoeff);
            }

            // update gradient vector
            gradientStrategy->compute(
                dx, std::vector<std::shared_ptr<AcceleratorBuffer>>(
                        buffers.begin() + nInstructionsEnergy, buffers.end()));

          } else { // normal QAOA run

            for (int i = 0; i < buffers.size(); i++) {
              auto expval = buffers[i]->getExpectationValueZ();
              energy += expval * coefficients[i];
              buffers[i]->addExtraInfo("coefficient", coefficients[i]);
              buffers[i]->addExtraInfo("kernel", fsToExec[i]->name());
              buffers[i]->addExtraInfo("exp-val-z", expval);
              buffers[i]->addExtraInfo("parameters", x);
              buffer->appendChild(fsToExec[i]->name(), buffers[i]);
            }
          }

          std::stringstream ss;
          iterCount++;
          ss << "Iter " << iterCount << ": E("
             << (!x.empty() ? std::to_string(x[0]) : "");
          for (int i = 1; i < x.size(); i++) {
            ss << "," << std::setprecision(3) << x[i];
            if (i > 4) {
              // Don't print too many params
              ss << ", ...";
              break;
            }
          }
          ss << ") = " << std::setprecision(12) << energy;
          xacc::info(ss.str());

          if (m_maximize) energy *= -1.0;
          return energy;
        },
        kernel->nVariables());

    auto result = m_optimizer->optimize(f);

    // Reports the final cost:
    double finalCost = result.first;
    if (m_maximize) finalCost *= -1.0;
    buffer->addExtraInfo("opt-val", ExtraInfo(finalCost));
    buffer->addExtraInfo("opt-params", ExtraInfo(result.second));
  }

  std::vector<double>
  WS_QAOA::execute(const std::shared_ptr<AcceleratorBuffer> buffer,
                const std::vector<double> &x) {
    const int nbQubits = buffer->size();
    std::shared_ptr<CompositeInstruction> kernel;
    if (externalAnsatz) {
      kernel = externalAnsatz;
    } else if (m_single_exec_kernel) {
      kernel = m_single_exec_kernel;
    } else {
      HeterogeneousMap m;
      kernel = std::dynamic_pointer_cast<CompositeInstruction>(
            xacc::getService<Instruction>("ws_qaoa"));
      m.insert("nbQubits", nbQubits);
      m.insert("nbSteps", m_nbSteps);
      m.insert("good_cut", good_cut);
      m.insert("cost-ham", m_costHamObs);
      m.insert("parameter-scheme", m_parameterizedMode);
      m.insert("shuffle-terms", m_shuffleTerms);
      kernel->expand(m);
      // save this kernel for future calls to execute
      m_single_exec_kernel = kernel;
    }

    // Observe the cost Hamiltonian, with the input Accelerator:
    // i.e. perform grouping (e.g. max-cut QAOA, Pauli) if possible:
    kernel = kernel->operator()(x);
    auto kernels = [&] {
      if (dynamic_cast<xacc::quantum::PauliOperator *>(m_costHamObs)) {
        return m_costHamObs->observe(kernel, {{"accelerator", m_qpu}});
      } else {
        return m_costHamObs->observe(kernel);
      }
    }();

    if (m_costHamObs->getNonIdentitySubTerms().size() > 1 &&
        kernels.size() == 1) {
      // Grouping was done:
      // just execute the single observed kernel:
      std::vector<std::shared_ptr<CompositeInstruction>> fsToExec{
          kernels[0]};
      if (m_irTransformation) {
        for (auto &composite : fsToExec) {
          m_irTransformation->apply(
              composite, xacc::as_shared_ptr<xacc::Accelerator>(m_qpu));
        }
      }
      m_qpu->execute(buffer, fsToExec);
      const double finalCost = m_costHamObs->postProcess(buffer);
      // std::cout << "Compute energy from grouping: " << finalCost << "\n";
      return { finalCost };
    }

    std::vector<double> coefficients;
    std::vector<std::string> kernelNames;
    std::vector<std::shared_ptr<CompositeInstruction>> fsToExec;

    double identityCoeff = 0.0;
    for (auto &f : kernels) {
      kernelNames.push_back(f->name());
      std::complex<double> coeff = f->getCoefficient();

      int nFunctionInstructions = 0;
      if (f->getInstruction(0)->isComposite()) {
        nFunctionInstructions = kernel->nInstructions() + f->nInstructions() - 1;
      } else {
        nFunctionInstructions = f->nInstructions();
      }

      if (nFunctionInstructions > kernel->nInstructions()) {
        fsToExec.push_back(f);
        coefficients.push_back(std::real(coeff));
      } else {
        identityCoeff += std::real(coeff);
      }
    }

    auto tmpBuffer = xacc::qalloc(buffer->size());
    if (m_irTransformation) {
      for (auto &composite : fsToExec) {
        m_irTransformation->apply(composite,
                                  xacc::as_shared_ptr<xacc::Accelerator>(m_qpu));
      }
    }
    m_qpu->execute(tmpBuffer, fsToExec);
    auto buffers = tmpBuffer->getChildren();

    double energy = identityCoeff;
    auto idBuffer = xacc::qalloc(buffer->size());
    idBuffer->addExtraInfo("coefficient", identityCoeff);
    idBuffer->setName("I");
    idBuffer->addExtraInfo("kernel", "I");
    idBuffer->addExtraInfo("parameters", x);
    idBuffer->addExtraInfo("exp-val-z", 1.0);
    buffer->appendChild("I", idBuffer);

    for (int i = 0; i < buffers.size(); i++) {
      auto expval = buffers[i]->getExpectationValueZ();
      energy += expval * coefficients[i];
      buffers[i]->addExtraInfo("coefficient", coefficients[i]);
      buffers[i]->addExtraInfo("kernel", fsToExec[i]->name());
      buffers[i]->addExtraInfo("exp-val-z", expval);
      buffers[i]->addExtraInfo("parameters", x);
      buffer->appendChild(fsToExec[i]->name(), buffers[i]);
    }

    // WARNING: Removing the parameter shifting here. Remember for later
    // in case of any tests that fail.
    const double finalCost = energy;
      //   m_maxcutProblem ? (-0.5 * energy +
      //                      0.5 * (m_costHamObs->getNonIdentitySubTerms().size()))
      //                   : energy;
    return {finalCost};
  }

}
