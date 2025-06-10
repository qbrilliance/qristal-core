// Copyright (c) Quantum Brilliance Pty Ltd

#pragma once

// STL
#include <string>
#include <future>
#include <chrono>
#include <thread>
#include <cassert>
#include <fstream>
#include <memory>
#include <numeric>
#include <iomanip>

// XACC
#include <PauliOperator.hpp>
#include <ObservableTransform.hpp>
#include <Utils.hpp>
#include <xacc.hpp>
#include <xacc_observable.hpp>
#include <xacc_service.hpp>
#include <AcceleratorDecorator.hpp>

// Qristal
#include <qristal/core/optimization/vqee/mpi_wrapper.hpp>
#include <qristal/core/optimization/vqee/case_generator.hpp>

namespace qristal::vqee {

  /// Variational Quantum Eigensolver (VQE) hybrid quantum-classical algorithm
  class VQEE {
  private:
    const bool  isRoot_ {GetRank() == 0}; // is MPI master process?
    const bool  isParallel_ {GetSize() >1};
    Params&     params_;

  public:

    /// Constructor that accepts qristal::vqee::Params
    VQEE(Params& params) : params_{params} {}

  // - - - - - - member functions - - - - - - //
  private:
    // Split a Pauli into multiple sub-Paulis according to a max number of terms constraint.
    std::vector<std::shared_ptr<xacc::quantum::PauliOperator>> splitPauli(std::shared_ptr<xacc::quantum::PauliOperator> &in_pauli, int nTermsPerSplit) const;

    // helper constructors/initializers
    std::shared_ptr<xacc::Accelerator> getAccelerator(const std::string& accName) const;
    std::shared_ptr<xacc::CompositeInstruction> getAnsatz() const;
    std::shared_ptr<xacc::Observable> getObservable();

    /// Subroutines for visualising VQE convergence

    /**
     * @brief Locate the iteration that produced the optimum theta
     *
     * @return Index that matches the optimum iteration found by XACC
     */
    size_t getOptimumIterationE();

    /**
     * @brief Create a text-art bar graph for energy and each element of theta.
     * The resultant visualisation is stored in params_.vis.
     * The iteration corresponding to the optimum is marked '**'.
     *
     * Colour markings:
     * Red - indicates the optimum iteration identified by XACC
     * Green - indicates an iteration with the same energy of the optimum energy
     *
     * Example output:
     *
     * Iteration 96
     * Energy         |################# -0.95
     * Theta
     *        Element 0      |####################### 0.33*pi
     *        Element 1      |##################### 0.19*pi
     *        Element 2      |################### -0.12*pi
     *        Element 3      |######################### 0.55*pi
     * Iteration 97
     * Energy       **|################ -0.98
     * Theta
     *        Element 0    **|####################### 0.3*pi
     *        Element 1    **|##################### 0.19*pi
     *        Element 2    **|#################### -0.059*pi
     *        Element 3    **|######################### 0.5*pi
     * Iteration 98
     * Energy         |################# -0.97
     * Theta
     *        Element 0      |###################### 0.29*pi
     *        Element 1      |##################### 0.14*pi
     *        Element 2      |#################### 0.061*pi
     *        Element 3      |######################### 0.55*pi
     *
     * @param in_title A string to be used as a title before the text-art output
     * @param in_start_elem Index for the starting element within theta for which the next in_elem_n elements will be visualised
     * @param in_scale A scaling factor to adjust the size of the visual bar output
     * @param in_width The size of a blank separator between bar labels and bars
     * @param in_precision The precision for printing a value at the right extremity of a bar
     */
    void generateThetaEnergyVis(const std::string in_title, const size_t in_start_elem = 0, const int in_scale = 20,  const int in_width = 2, const int in_precision = 2);

    /**
     * @brief Create a text-art bar graph for energy.
     * The resultant visualisation is stored in params_.vis.
     * The iteration corresponding to the optimum is marked '**'.
     * Values are displayed at the right extremity of each bar.
     * The scaling of bars is based on the energy at the first iteration, and rescales as soon as
     * the size of a bar in any iteration is zero.
     *
     * @param in_val An array with consecutive iterations separated by in_stride elements
     * @param in_title A string to be used as a title before the text-art output
     * @param in_stride The stride separating consecutive iterations
     * @param in_scale A scaling factor to adjust the size of the visual bar output
     * @param in_width The size of a blank separator between bar labels and bars
     * @param in_precision The precision for printing a value at the right extremity of a bar
     *
     * @return String representation of energy convergence trace
     */
    std::string generateEnergyVis(const std::vector<double> in_val, const std::string in_title, const int in_stride = 1, const int in_scale = 20, const int in_width = 2, const int in_precision = 2);

  public:

    /// Set up and optimize VQE problem
    void optimize();

  };

}
