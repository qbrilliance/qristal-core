/***
 *** Copyright (c) 2022 Quantum Brilliance Pty Ltd
 ***/

#include "qristal/core/circuits/amplitude_amplification.hpp"
#include "qristal/core/circuits/canonical_amplitude_estimation.hpp"
#include "qristal/core/circuits/comparator.hpp"
#include "qristal/core/circuits/efficient_encoding.hpp"
#include "qristal/core/circuits/mcu_with_ancilla.hpp"
#include "qristal/core/circuits/phase_estimation.hpp"
#include "qristal/core/circuits/equality_checker.hpp"
#include "qristal/core/circuits/q_prime_unitary.hpp"
#include "qristal/core/circuits/ripple_adder.hpp"
#include "qristal/core/circuits/u_prime_unitary.hpp"
#include "qristal/core/circuits/uq_prime_unitary.hpp"
#include "qristal/core/circuits/w_prime_unitary.hpp"
#include "qristal/core/circuits/controlled_swap.hpp"
#include "qristal/core/circuits/controlled_addition.hpp"
#include "qristal/core/circuits/generalised_mcx.hpp"
#include "qristal/core/circuits/compare_beam_oracle.hpp"
#include "qristal/core/circuits/inverse_circuit.hpp"
#include "qristal/core/circuits/init_rep_flag.hpp"
#include "qristal/core/circuits/ae_to_metric.hpp"
#include "qristal/core/circuits/qd_beam_state_prep.hpp"
#include "qristal/core/circuits/superposition_adder.hpp"
#include "qristal/core/circuits/pseudo_trace_amplitude_estimation.hpp"
#include "qristal/core/circuits/subtraction.hpp"
#include "qristal/core/circuits/controlled_subtraction.hpp"
#include "qristal/core/circuits/proper_fraction_division.hpp"
#include "qristal/core/circuits/controlled_proper_fraction_division.hpp"
#include "qristal/core/circuits/compare_gt.hpp"
#include "qristal/core/circuits/multiplication.hpp"
#include "qristal/core/circuits/controlled_multiplication.hpp"
#include "qristal/core/circuits/mean_value_finder.hpp"

#include "cppmicroservices/BundleActivator.h"
#include "cppmicroservices/BundleContext.h"
#include "cppmicroservices/ServiceProperties.h"
using namespace cppmicroservices;

class US_ABI_LOCAL CircuitGenActivator : public BundleActivator {
public:
  CircuitGenActivator() {}

  void Start(BundleContext context) {
    context.RegisterService<xacc::Instruction>(
        std::make_shared<qristal::CanonicalAmplitudeEstimation>());
    context.RegisterService<xacc::Instruction>(
        std::make_shared<qristal::PhaseEstimation>());
    context.RegisterService<xacc::Instruction>(
        std::make_shared<qristal::AmplitudeAmplification>());
    context.RegisterService<xacc::Instruction>(
        std::make_shared<qristal::UQPrime>());
    context.RegisterService<xacc::Instruction>(
        std::make_shared<qristal::WPrime>());
    context.RegisterService<xacc::Instruction>(
        std::make_shared<qristal::UPrime>());
    context.RegisterService<xacc::Instruction>(
        std::make_shared<qristal::QPrime>());
    context.RegisterService<xacc::Instruction>(
        std::make_shared<qristal::RippleCarryAdder>());
    context.RegisterService<xacc::Instruction>(
        std::make_shared<qristal::Comparator>());
    context.RegisterService<xacc::Instruction>(
        std::make_shared<qristal::EfficientEncoding>());
    context.RegisterService<xacc::Instruction>(
        std::make_shared<qristal::MultiControlledUWithAncilla>());
    context.RegisterService<xacc::Instruction>(
        std::make_shared<qristal::EqualityChecker>());
    context.RegisterService<xacc::Instruction>(
        std::make_shared<qristal::ControlledSwap>());
    context.RegisterService<xacc::Instruction>(
        std::make_shared<qristal::ControlledAddition>());
    context.RegisterService<xacc::Instruction>(
        std::make_shared<qristal::GeneralisedMCX>());
    context.RegisterService<xacc::Instruction>(
        std::make_shared<qristal::CompareBeamOracle>());
    context.RegisterService<xacc::Instruction>(
        std::make_shared<qristal::InverseCircuit>());
    context.RegisterService<xacc::Instruction>(
        std::make_shared<qristal::InitRepeatFlag>());
    context.RegisterService<xacc::Instruction>(
        std::make_shared<qristal::InitRepeatFlag>());
    context.RegisterService<xacc::Instruction>(
        std::make_shared<qristal::AEtoMetric>());
    context.RegisterService<xacc::Instruction>(
        std::make_shared<qristal::BeamStatePrep>());
    context.RegisterService<xacc::Instruction>(
        std::make_shared<qristal::SuperpositionAdder>());
    context.RegisterService<xacc::Instruction>(
        std::make_shared<qristal::PseudoTraceAmplitudeEstimation>());
    context.RegisterService<xacc::Instruction>(
        std::make_shared<qristal::Subtraction>());
    context.RegisterService<xacc::Instruction>(
        std::make_shared<qristal::ControlledSubtraction>());
    context.RegisterService<xacc::Instruction>(
        std::make_shared<qristal::ProperFractionDivision>());
    context.RegisterService<xacc::Instruction>(
        std::make_shared<qristal::ControlledProperFractionDivision>());
    context.RegisterService<xacc::Instruction>(
        std::make_shared<qristal::CompareGT>());
    context.RegisterService<xacc::Instruction>(
        std::make_shared<qristal::Multiplication>());
    context.RegisterService<xacc::Instruction>(
    std::make_shared<qristal::ControlledMultiplication>());
    context.RegisterService<xacc::Instruction>(
    std::make_shared<qristal::MeanValueFinder>());
  }

  void Stop(BundleContext context) {}
};

CPPMICROSERVICES_EXPORT_BUNDLE_ACTIVATOR(CircuitGenActivator)
