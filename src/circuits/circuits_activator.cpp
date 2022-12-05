/***
 *** Copyright (c) 2022 Quantum Brilliance Pty Ltd
 ***/

#include "qb/core/circuits/amplitude_amplification.hpp"
#include "qb/core/circuits/canonical_amplitude_estimation.hpp"
#include "qb/core/circuits/comparator.hpp"
#include "qb/core/circuits/efficient_encoding.hpp"
#include "qb/core/circuits/mcu_with_ancilla.hpp"
#include "qb/core/circuits/phase_estimation.hpp"
#include "qb/core/circuits/equality_checker.hpp"
#include "qb/core/circuits/q_prime_unitary.hpp"
#include "qb/core/circuits/ripple_adder.hpp"
#include "qb/core/circuits/u_prime_unitary.hpp"
#include "qb/core/circuits/uq_prime_unitary.hpp"
#include "qb/core/circuits/w_prime_unitary.hpp"
#include "qb/core/circuits/controlled_swap.hpp"
#include "qb/core/circuits/controlled_addition.hpp"
#include "qb/core/circuits/generalised_mcx.hpp"
#include "qb/core/circuits/compare_beam_oracle.hpp"
#include "qb/core/circuits/inverse_circuit.hpp"
#include "qb/core/circuits/init_rep_flag.hpp"
#include "qb/core/circuits/ae_to_metric.hpp"
#include "qb/core/circuits/qd_beam_state_prep.hpp"
#include "qb/core/circuits/superposition_adder.hpp"
#include "qb/core/circuits/pseudo_trace_amplitude_estimation.hpp"
#include "qb/core/circuits/subtraction.hpp"
#include "qb/core/circuits/controlled_subtraction.hpp"
#include "qb/core/circuits/proper_fraction_division.hpp"
#include "qb/core/circuits/controlled_proper_fraction_division.hpp"
#include "qb/core/circuits/compare_gt.hpp"
#include "qb/core/circuits/multiplication.hpp"
#include "qb/core/circuits/controlled_multiplication.hpp"
#include "qb/core/circuits/mean_value_finder.hpp"

#include "cppmicroservices/BundleActivator.h"
#include "cppmicroservices/BundleContext.h"
#include "cppmicroservices/ServiceProperties.h"
using namespace cppmicroservices;

class US_ABI_LOCAL CircuitGenActivator : public BundleActivator {
public:
  CircuitGenActivator() {}

  void Start(BundleContext context) {
    context.RegisterService<xacc::Instruction>(
        std::make_shared<qb::CanonicalAmplitudeEstimation>());
    context.RegisterService<xacc::Instruction>(
        std::make_shared<qb::PhaseEstimation>());
    context.RegisterService<xacc::Instruction>(
        std::make_shared<qb::AmplitudeAmplification>());
    context.RegisterService<xacc::Instruction>(
        std::make_shared<qb::UQPrime>());
    context.RegisterService<xacc::Instruction>(
        std::make_shared<qb::WPrime>());
    context.RegisterService<xacc::Instruction>(
        std::make_shared<qb::UPrime>());
    context.RegisterService<xacc::Instruction>(
        std::make_shared<qb::QPrime>());
    context.RegisterService<xacc::Instruction>(
        std::make_shared<qb::RippleCarryAdder>());
    context.RegisterService<xacc::Instruction>(
        std::make_shared<qb::Comparator>());
    context.RegisterService<xacc::Instruction>(
        std::make_shared<qb::EfficientEncoding>());
    context.RegisterService<xacc::Instruction>(
        std::make_shared<qb::MultiControlledUWithAncilla>());
    context.RegisterService<xacc::Instruction>(
        std::make_shared<qb::EqualityChecker>());
    context.RegisterService<xacc::Instruction>(
        std::make_shared<qb::ControlledSwap>());
    context.RegisterService<xacc::Instruction>(
        std::make_shared<qb::ControlledAddition>());
    context.RegisterService<xacc::Instruction>(
        std::make_shared<qb::GeneralisedMCX>());
    context.RegisterService<xacc::Instruction>(
        std::make_shared<qb::CompareBeamOracle>());
    context.RegisterService<xacc::Instruction>(
        std::make_shared<qb::InverseCircuit>());
    context.RegisterService<xacc::Instruction>(
        std::make_shared<qb::InitRepeatFlag>());
    context.RegisterService<xacc::Instruction>(
        std::make_shared<qb::InitRepeatFlag>());
    context.RegisterService<xacc::Instruction>(
        std::make_shared<qb::AEtoMetric>());
    context.RegisterService<xacc::Instruction>(
        std::make_shared<qb::BeamStatePrep>());
    context.RegisterService<xacc::Instruction>(
        std::make_shared<qb::SuperpositionAdder>());
    context.RegisterService<xacc::Instruction>(
        std::make_shared<qb::PseudoTraceAmplitudeEstimation>());
    context.RegisterService<xacc::Instruction>(
        std::make_shared<qb::Subtraction>());
    context.RegisterService<xacc::Instruction>(
        std::make_shared<qb::ControlledSubtraction>());
    context.RegisterService<xacc::Instruction>(
        std::make_shared<qb::ProperFractionDivision>());
    context.RegisterService<xacc::Instruction>(
        std::make_shared<qb::ControlledProperFractionDivision>());
    context.RegisterService<xacc::Instruction>(
        std::make_shared<qb::CompareGT>());
    context.RegisterService<xacc::Instruction>(
        std::make_shared<qb::Multiplication>());
    context.RegisterService<xacc::Instruction>(
    std::make_shared<qb::ControlledMultiplication>());
    context.RegisterService<xacc::Instruction>(
    std::make_shared<qb::MeanValueFinder>());
  }

  void Stop(BundleContext context) {}
};

CPPMICROSERVICES_EXPORT_BUNDLE_ACTIVATOR(CircuitGenActivator)
