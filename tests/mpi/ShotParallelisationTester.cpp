#include <qristal/core/circuit_builder.hpp>
#include <qristal/core/mpi/results_types.hpp>
#include <qristal/core/mpi/workload_partitioning.hpp>
#include <qristal/core/session.hpp>

#include <boost/math/distributions/normal.hpp>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <range/v3/view/join.hpp>
#include <range/v3/view/zip.hpp>

#include <cmath>
#include <cstdint>
#include <sstream>
#include <ranges>

using namespace qristal;

constexpr double CIRCUIT_PARAM_ALPHA = M_PI / 3;
constexpr double CIRCUIT_PARAM_BETA = 2 * M_PI / 7;
constexpr int32_t CIRCUIT_NUMBER_OF_SHOTS = 1000000;
constexpr int32_t CIRCUIT_NUMBER_OF_QUBITS = 2;

void setup_and_run_circuit(session &session) {

  session.acc = "aer";
  session.qn = CIRCUIT_NUMBER_OF_QUBITS;
  session.sn = CIRCUIT_NUMBER_OF_SHOTS;
  session.calc_gradients = true;

  auto circuit = CircuitBuilder();
  circuit.RX(0, "alpha");
  circuit.RX(1, "beta");
  circuit.MeasureAll(-1);

  session.irtarget = circuit.get();
  session.circuit_parameters = {CIRCUIT_PARAM_ALPHA, CIRCUIT_PARAM_BETA};

  session.run();
}

int32_t one_in_n_expected_failures(int32_t number_standard_deviations) {
  const double z = static_cast<double>(number_standard_deviations);
  // Calculate the proportion outside the input number of standard deviations
  const double p_outside = 2 * (1.0 - 0.5 * (1.0 + std::erf(z / std::sqrt(2.0))));
  const double number_of_possible_bitstrings = static_cast<double>(std::pow(2, CIRCUIT_NUMBER_OF_QUBITS));
  const double p_at_least_one_bitstring_outside = p_outside * number_of_possible_bitstrings;
  return static_cast<int32_t>(std::round(1.0 / p_at_least_one_bitstring_outside));
}

std::string tolerance_check_error_msg(double probability, double tolerance_stddev) {
    std::stringstream ss;
    ss << "The calculated probability is " << probability << " standard deviations away from the expected "
          "probability. Note that the tolerance used for this test is " << tolerance_stddev << " standard "
          "deviations from the calculated expected probability for the bitstring. Whilst unlikely (1 in " <<
          one_in_n_expected_failures(tolerance_stddev) << " test runs) it is possible this failure was "
          "just because this run happened to fall outside of the tolerance limits. Unless you've changed "
          "MPI shot parallelisation-related code, this might be the reason for the test failing. Try "
          "re-running the test a few more times to see whether this is the explanation for the current failure.";
    return ss.str();
}

void check_all_bitstring_probabilities(const session::OutProbabilitiesType &all_bitstring_probabilities, const session::OutCountsType &all_bitstring_counts) {
  EXPECT_FLOAT_EQ(std::accumulate(all_bitstring_probabilities.begin(), all_bitstring_probabilities.end(), 0.0), 1.0);
  auto counts_from_probs = all_bitstring_probabilities | std::views::transform([](session::ProbabilityType prob) {
                             return static_cast<int32_t>(std::round(prob * CIRCUIT_NUMBER_OF_SHOTS));
                           });
  EXPECT_THAT(std::vector<int32_t>(counts_from_probs.begin(), counts_from_probs.end()),
              testing::ContainerEq(all_bitstring_counts));

  // Check that the probabilities are what are expected for the input circuit.
  // The below expected probabilities were obtained using the circuit and its
  // parameters.

  // P_00 = pow(cos(alpha/2),2)*pow(cos(beta/2),2)
  const session::ProbabilityType expected_prob_00 =
      std::pow(std::cos(CIRCUIT_PARAM_ALPHA / 2.0), 2.0) * std::pow(std::cos(CIRCUIT_PARAM_BETA / 2), 2);
  ASSERT_FLOAT_EQ(expected_prob_00, 0.6088086756970252);
  // P_01 = pow(sin(alpha/2),2)*pow(cos(beta/2),2)
  const session::ProbabilityType expected_prob_10 =
      std::pow(std::sin(CIRCUIT_PARAM_ALPHA / 2.0), 2.0) * std::pow(std::cos(CIRCUIT_PARAM_BETA / 2), 2);
  ASSERT_FLOAT_EQ(expected_prob_10, 0.20293622523234164);
  // P_10 = pow(cos(alpha/2),2)*pow(sin(beta/2),2)
  const session::ProbabilityType expected_prob_01 =
      std::pow(std::cos(CIRCUIT_PARAM_ALPHA / 2.0), 2.0) * std::pow(std::sin(CIRCUIT_PARAM_BETA / 2), 2);
  ASSERT_FLOAT_EQ(expected_prob_01, 0.14119132430297496);
  // P_11 = pow(sin(alpha/2),2)*pow(sin(beta/2),2)
  const session::ProbabilityType expected_prob_11 =
      std::pow(std::sin(CIRCUIT_PARAM_ALPHA / 2.0), 2.0) * std::pow(std::sin(CIRCUIT_PARAM_BETA / 2), 2);
  ASSERT_FLOAT_EQ(expected_prob_11, 0.047063774767658294);

  const std::vector<double> expected_probs{expected_prob_00, expected_prob_10, expected_prob_01, expected_prob_11};

  // The tolerance used for checking the calculated probabilities are correct
  // is +/- 5 standard deviations from the calculated probability for a
  // multinomial distribution
  auto probability_standard_deviation = [](session::ProbabilityType prob) {
    return std::sqrt(prob * (1 - prob) / CIRCUIT_NUMBER_OF_SHOTS);
  };

  auto expected_probs_calculated_probs = ::ranges::views::zip(expected_probs, all_bitstring_probabilities);
  for (auto expected_prob_calculated_prob : expected_probs_calculated_probs) {
    auto &[expected_prob, calculated_prob] = expected_prob_calculated_prob;
    constexpr int32_t number_standard_deviations_tolerance = 5;
    const double tolerance_prob = number_standard_deviations_tolerance * probability_standard_deviation(expected_prob);
    EXPECT_NEAR(calculated_prob, expected_prob, tolerance_prob) << 
      tolerance_check_error_msg(
        std::abs(calculated_prob - expected_prob) / probability_standard_deviation(expected_prob),
        number_standard_deviations_tolerance);
  }
}

void check_all_bitstring_probability_gradients(const session::OutProbabilityGradientsType &all_bitstring_probability_gradients, const session &session) {
  // The below gradient values were calculated using an analytical method based on the circuit parameters and circuit
  // itself dP_00/d_alpha = -1/2*sin(alpha)*pow(cos(beta/2),2)
  const double expected_prob_gradient_alpha_00 =
      -0.5 * std::sin(CIRCUIT_PARAM_ALPHA) * std::pow(std::cos(CIRCUIT_PARAM_BETA / 2.0), 2.0);
  ASSERT_FLOAT_EQ(expected_prob_gradient_alpha_00, -0.35149585279865697);
  // dP_10/d_alpha = 1/2*sin(alpha)*pow(cos(beta/2),2)
  const double expected_prob_gradient_alpha_10 =
      0.5 * std::sin(CIRCUIT_PARAM_ALPHA) * std::pow(std::cos(CIRCUIT_PARAM_BETA / 2.0), 2.0);
  ASSERT_FLOAT_EQ(expected_prob_gradient_alpha_10, 0.35149585279865697);
  // dP_01/d_alpha = -1/2*sin(alpha)*pow(sin(beta/2),2)
  const double expected_prob_gradient_alpha_01 =
      -0.5 * std::sin(CIRCUIT_PARAM_ALPHA) * std::pow(std::sin(CIRCUIT_PARAM_BETA / 2.0), 2.0);
  ASSERT_FLOAT_EQ(expected_prob_gradient_alpha_01, -0.08151684909356231);
  // dP_11/d_alpha = 1/2*sin(alpha)*pow(sin(beta/2),2)
  const double expected_prob_gradient_alpha_11 =
      0.5 * std::sin(CIRCUIT_PARAM_ALPHA) * std::pow(std::sin(CIRCUIT_PARAM_BETA / 2.0), 2.0);
  ASSERT_FLOAT_EQ(expected_prob_gradient_alpha_11, 0.08151684909356231);
  // dP_00/d_beta = -1/2*sin(beta)*pow(cos(alpha/2),2)
  const double expected_prob_gradient_beta_00 =
      -0.5 * std::sin(CIRCUIT_PARAM_BETA) * std::pow(std::cos(CIRCUIT_PARAM_ALPHA / 2.0), 2.0);
  ASSERT_FLOAT_EQ(expected_prob_gradient_beta_00, -0.2931868059255112);
  // dP_10/d_beta = -1/2*sin(beta)*pow(sin(alpha/2),2)
  const double expected_prob_gradient_beta_10 =
      -0.5 * std::sin(CIRCUIT_PARAM_BETA) * std::pow(std::sin(CIRCUIT_PARAM_ALPHA / 2.0), 2.0);
  ASSERT_FLOAT_EQ(expected_prob_gradient_beta_10, -0.0977289353085037);
  // dP_01/d_beta = 1/2*sin(beta)*pow(cos(alpha/2),2)
  const double expected_prob_gradient_beta_01 =
      0.5 * std::sin(CIRCUIT_PARAM_BETA) * std::pow(std::cos(CIRCUIT_PARAM_ALPHA / 2.0), 2.0);
  ASSERT_FLOAT_EQ(expected_prob_gradient_beta_01, 0.2931868059255112);
  // dP_11/d_beta = 1/2*sin(beta)*pow(sin(alpha/2),2)
  const double expected_prob_gradient_beta_11 =
      0.5 * std::sin(CIRCUIT_PARAM_BETA) * std::pow(std::sin(CIRCUIT_PARAM_ALPHA / 2.0), 2.0);
  ASSERT_FLOAT_EQ(expected_prob_gradient_beta_11, 0.0977289353085037);

  // Tolerances were calculated using the pre-determined standard deviation of the calculated gradients from ~16,000
  // runs of the circuit
  constexpr int32_t number_standard_deviations_tolerance = 5;
  constexpr double expected_prob_gradient_alpha_00_tolerance = number_standard_deviations_tolerance * 0.00024648;
  constexpr double expected_prob_gradient_alpha_10_tolerance = number_standard_deviations_tolerance * 0.00024271;
  constexpr double expected_prob_gradient_alpha_01_tolerance = number_standard_deviations_tolerance * 0.00019939;
  constexpr double expected_prob_gradient_alpha_11_tolerance = number_standard_deviations_tolerance * 0.0001975;
  constexpr double expected_prob_gradient_beta_00_tolerance = number_standard_deviations_tolerance * 0.00027225;
  constexpr double expected_prob_gradient_beta_10_tolerance = number_standard_deviations_tolerance * 0.00022358;
  constexpr double expected_prob_gradient_beta_01_tolerance = number_standard_deviations_tolerance * 0.0002721;
  constexpr double expected_prob_gradient_beta_11_tolerance = number_standard_deviations_tolerance * 0.00022249;

  // Group into vectors for checks
  std::vector<double> expected_prob_gradients{expected_prob_gradient_alpha_00, expected_prob_gradient_alpha_10,
                                              expected_prob_gradient_alpha_01, expected_prob_gradient_alpha_11,
                                              expected_prob_gradient_beta_00,  expected_prob_gradient_beta_10,
                                              expected_prob_gradient_beta_01,  expected_prob_gradient_beta_11};
  std::vector<double> prob_gradients_tolerances{
      expected_prob_gradient_alpha_00_tolerance, expected_prob_gradient_alpha_10_tolerance,
      expected_prob_gradient_alpha_01_tolerance, expected_prob_gradient_alpha_11_tolerance,
      expected_prob_gradient_beta_00_tolerance,  expected_prob_gradient_beta_10_tolerance,
      expected_prob_gradient_beta_01_tolerance,  expected_prob_gradient_beta_11_tolerance};

  // Perform checks
  // The outer vector size should be the number of parameters
  EXPECT_EQ(all_bitstring_probability_gradients.size(), session.circuit_parameters.size());

  // The sum of gradients for each parameter should be 0
  for (const auto &gradient : all_bitstring_probability_gradients) {
    constexpr double gradient_sum_tolerance = 1e-12;
    EXPECT_NEAR(std::accumulate(gradient.begin(), gradient.end(), 0.0), 0.0, gradient_sum_tolerance);

    // Each inner vector size should be 2 raised to the number of qubits
    EXPECT_EQ(gradient.size(), std::pow(2, CIRCUIT_NUMBER_OF_QUBITS));
  }

  auto expecteds_tolerances_calculateds = ::ranges::views::zip(expected_prob_gradients, prob_gradients_tolerances,
                                                               ::ranges::views::join(all_bitstring_probability_gradients));
  for (const auto &expected_tolerance_calculated : expecteds_tolerances_calculateds) {
    auto &[expected, tolerance, calculated] = expected_tolerance_calculated;

    // The actual gradient should be within the tolerance limits of the analytically calculated value
    EXPECT_NEAR(expected, calculated, tolerance) <<
      tolerance_check_error_msg(std::abs(calculated - expected) / (tolerance / number_standard_deviations_tolerance),
        number_standard_deviations_tolerance);
  }
}

TEST(ShotParallelisationTester, ChecksShotsParallelisedCorrectly) {
  session session;

  setup_and_run_circuit(session);

  const session::ResultsMapType &results = session.results();
  const session::OutCountsType &all_bitstring_counts = session.all_bitstring_counts();
  const session::OutProbabilitiesType &all_bitstring_probabilities = session.all_bitstring_probabilities();
  const session::OutProbabilityGradientsType &all_bitstring_probability_gradients = session.all_bitstring_probability_gradients();

  auto shots_from_results = [](const mpi::ResultsMap &results) {
    return std::accumulate(std::views::values(results).begin(), std::views::values(results).end(), INT32_C(0));
  };
  auto shots_from_all_bitstring_counts = [](const session::OutCountsType &all_bitstring_counts) {
    return std::accumulate(all_bitstring_counts.begin(), all_bitstring_counts.end(), INT32_C(0));
  };

  if (session.get_mpi_process_id() == 0) {
    // Check rank 0 collected all results

    // Check results
    EXPECT_EQ(shots_from_results(results), CIRCUIT_NUMBER_OF_SHOTS);

    // Check counts
    EXPECT_EQ(shots_from_all_bitstring_counts(all_bitstring_counts), CIRCUIT_NUMBER_OF_SHOTS);

    // Check probs
    check_all_bitstring_probabilities(all_bitstring_probabilities, all_bitstring_counts);

    // Check all_bitstring_probability_gradients
    check_all_bitstring_probability_gradients(all_bitstring_probability_gradients, session);
  } else {
    // Check worker processes only completed its assigned processes
    const int32_t expected_shots = mpi::shots_for_mpi_process(session.get_total_mpi_processes(),
                                                              CIRCUIT_NUMBER_OF_SHOTS, session.get_mpi_process_id());
    EXPECT_EQ(shots_from_results(results), expected_shots);
    EXPECT_EQ(shots_from_all_bitstring_counts(all_bitstring_counts), expected_shots);
    // all_bitstring_probabilities and all_bitstring_probability_gradients are calculated off the all_bitstring_counts so
    // validating shot count from the results and all_bitstring_counts is sufficient to
    // test MPI shot parallelisation in the worker processes
  }
}
