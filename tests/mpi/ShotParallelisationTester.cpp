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
#include <ranges>

using namespace qristal;

constexpr double CIRCUIT_PARAM_ALPHA = M_PI / 3;
constexpr double CIRCUIT_PARAM_BETA = 2 * M_PI / 7;
constexpr int32_t CIRCUIT_NUMBER_OF_SHOTS = 1000000;
constexpr int32_t CIRCUIT_NUMBER_OF_QUBITS = 2;

void setup_and_run_circuit(session &session) {
  session.init();

  session.set_acc("aer");
  session.set_qn(CIRCUIT_NUMBER_OF_QUBITS);
  session.set_sn(CIRCUIT_NUMBER_OF_SHOTS);
  session.set_calc_jacobian(true);

  auto circuit = CircuitBuilder();
  circuit.RX(0, "alpha");
  circuit.RX(1, "beta");
  circuit.MeasureAll(-1);

  std::vector<double> circuit_param_vec = {CIRCUIT_PARAM_ALPHA, CIRCUIT_PARAM_BETA};
  session.set_irtarget_ms({{circuit.get()}});
  session.set_parameter_vectors({{circuit_param_vec}});

  session.run();
}

constexpr std::string_view TOLERANCE_CHECK_ERROR_MSG =
    "The calculated probability is {} standard deviations away from the expected probability. Note that the tolerance "
    "used for this test is {} standard deviations from the calculated expected probability for the bitstring. Whilst "
    "unlikely (1 in {} test runs) it is possible this failure was just because this run happened to fall outside of "
    "the tolerance limits. Unless you've changed MPI shot parallelisation-related code, this is a might be the reason "
    "for the test failing. Try re-running the test a few more times so see whether this is the explanation for the "
    "current failure.";

int32_t one_in_n_expected_failures(int32_t number_standard_deviations) {
  const double z = static_cast<double>(number_standard_deviations);
  // Calculate the proportion outside the input number of standard deviations
  const double p_outside = 2 * (1.0 - 0.5 * (1.0 + std::erf(z / std::sqrt(2.0))));
  const double number_of_possible_bitstrings = static_cast<double>(std::pow(2, CIRCUIT_NUMBER_OF_QUBITS));
  const double p_at_least_one_bitstring_outside = p_outside * number_of_possible_bitstrings;
  return static_cast<int32_t>(std::round(1.0 / p_at_least_one_bitstring_outside));
}

void check_out_probs(const session::OutProbabilitiesType &out_probs, const session::OutCountsType &out_counts) {
  EXPECT_FLOAT_EQ(std::accumulate(out_probs.begin(), out_probs.end(), 0.0), 1.0);
  auto counts_from_probs = out_probs | std::views::transform([](session::ProbabilityType prob) {
                             return static_cast<int32_t>(std::round(prob * CIRCUIT_NUMBER_OF_SHOTS));
                           });
  EXPECT_THAT(std::vector<int32_t>(counts_from_probs.begin(), counts_from_probs.end()),
              testing::ContainerEq(out_counts));

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

  auto expected_probs_calculated_probs = ::ranges::views::zip(expected_probs, out_probs);
  for (auto expected_prob_calculated_prob : expected_probs_calculated_probs) {
    auto &[expected_prob, calculated_prob] = expected_prob_calculated_prob;
    constexpr int32_t number_standard_deviations_tolerance = 5;
    const double tolerance_prob = number_standard_deviations_tolerance * probability_standard_deviation(expected_prob);
    EXPECT_NEAR(calculated_prob, expected_prob, tolerance_prob) << fmt::format(
        TOLERANCE_CHECK_ERROR_MSG,
        std::abs(calculated_prob - expected_prob) / probability_standard_deviation(expected_prob),
        number_standard_deviations_tolerance, one_in_n_expected_failures(number_standard_deviations_tolerance));
  }
}

void check_out_prob_jacobians(const session::OutProbabilityGradientsType &out_prob_jacobians, const session &session) {
  // The below jacobian values were calculated using an analytical method based on the circuit parameters and circuit
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

  // Tolerances were calculated using the pre-determined standard deviation of the calculated jacobians from ~16,000
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
  std::vector<double> expected_prob_jacobians{expected_prob_gradient_alpha_00, expected_prob_gradient_alpha_10,
                                              expected_prob_gradient_alpha_01, expected_prob_gradient_alpha_11,
                                              expected_prob_gradient_beta_00,  expected_prob_gradient_beta_10,
                                              expected_prob_gradient_beta_01,  expected_prob_gradient_beta_11};
  std::vector<double> prob_jacobians_tolerances{
      expected_prob_gradient_alpha_00_tolerance, expected_prob_gradient_alpha_10_tolerance,
      expected_prob_gradient_alpha_01_tolerance, expected_prob_gradient_alpha_11_tolerance,
      expected_prob_gradient_beta_00_tolerance,  expected_prob_gradient_beta_10_tolerance,
      expected_prob_gradient_beta_01_tolerance,  expected_prob_gradient_beta_11_tolerance};

  // Perform checks
  // The outer vector size should be the number of parameters
  EXPECT_EQ(out_prob_jacobians.size(), session.get_parameter_vectors()[0][0].size());

  // The sum of jacobians for each parameter should be 0
  for (const auto &out_prob_jacobian : out_prob_jacobians) {
    constexpr double jacobian_sum_tolerance = 1e-12;
    EXPECT_NEAR(std::accumulate(out_prob_jacobian.begin(), out_prob_jacobian.end(), 0.0), 0.0, jacobian_sum_tolerance);

    // Each inner vector size should be 2 raised to the number of qubits
    EXPECT_EQ(out_prob_jacobian.size(), std::pow(2, CIRCUIT_NUMBER_OF_QUBITS));
  }

  auto expecteds_tolerances_calculateds = ::ranges::views::zip(expected_prob_jacobians, prob_jacobians_tolerances,
                                                               ::ranges::views::join(out_prob_jacobians));
  for (const auto &expected_tolerance_calculated : expecteds_tolerances_calculateds) {
    auto &[expected, tolerance, calculated] = expected_tolerance_calculated;

    // The actual jacobian element should be within the tolerance limits of the analytically calculated value
    EXPECT_NEAR(expected, calculated, tolerance) << fmt::format(
        TOLERANCE_CHECK_ERROR_MSG, std::abs(calculated - expected) / (tolerance / number_standard_deviations_tolerance),
        number_standard_deviations_tolerance, one_in_n_expected_failures(number_standard_deviations_tolerance));
  }
}

TEST(ShotParallelisationTester, ChecksShotsParallelisedCorrectly) {
  session session;

  setup_and_run_circuit(session);

  const session::ResultsMapType &results = session.results()[0][0];
  const session::OutCountsType &out_counts = session.get_out_counts()[0][0];
  const session::OutProbabilitiesType &out_probs = session.get_out_probs()[0][0];
  const session::OutProbabilityGradientsType &out_prob_jacobians = session.get_out_prob_jacobians()[0][0];

  auto shots_from_results = [](const mpi::ResultsMap &results) {
    return std::accumulate(std::views::values(results).begin(), std::views::values(results).end(), INT32_C(0));
  };
  auto shots_from_out_counts = [](const session::OutCountsType &out_counts) {
    return std::accumulate(out_counts.begin(), out_counts.end(), INT32_C(0));
  };

  if (session.get_mpi_process_id() == 0) {
    // Check rank 0 collected all results

    // Check results
    EXPECT_EQ(shots_from_results(results), CIRCUIT_NUMBER_OF_SHOTS);

    // Check counts
    EXPECT_EQ(shots_from_out_counts(out_counts), CIRCUIT_NUMBER_OF_SHOTS);

    // Check probs
    check_out_probs(out_probs, out_counts);

    // Check out_prob_jacobians
    check_out_prob_jacobians(out_prob_jacobians, session);
  } else {
    // Check worker processes only completed its assigned processes
    const int32_t expected_shots = mpi::shots_for_mpi_process(session.get_total_mpi_processes(),
                                                              CIRCUIT_NUMBER_OF_SHOTS, session.get_mpi_process_id());
    EXPECT_EQ(shots_from_results(results), expected_shots);
    EXPECT_EQ(shots_from_out_counts(out_counts), expected_shots);
    // out_probs and out_prob_jacobians are calculated off the out_counts so
    // validating shot count from the results and out_counts is sufficient to
    // test MPI shot parallelisation in the worker processes
  }
}
