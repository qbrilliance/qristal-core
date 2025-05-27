#include <qristal/core/mpi/mpi_manager.hpp>
#include <qristal/core/mpi/results_serialisation.hpp>
#include <qristal/core/mpi/workload_partitioning.hpp>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <numeric>
#include <ranges>
#include <vector>

#include <range/v3/view/zip.hpp>

namespace qristal::mpi {

int32_t shots_for_mpi_process(int32_t total_processes, int32_t total_shots,
                              int32_t mpi_process_id) {
  int32_t shots_for_process = total_shots / total_processes;
  int32_t shots_for_process_remainder = total_shots % total_processes;

  // All processes other than the supervisor need to take the remainder of the
  // workload
  if (mpi_process_id != 0 &&
      shots_for_process_remainder - mpi_process_id >= 0) {
    shots_for_process++;
  }

  return shots_for_process;
}

void send_results_to_supervisor(
    MpiManager &mpi_manager, ResultsMap &results,
    std::optional<std::reference_wrapper<ResultsMap>> results_native,
    std::optional<std::span<Count>> out_counts,
    std::optional<std::span<Probability>> out_probs,
    std::optional<std::reference_wrapper<OutProbabilityGradients>>
        out_prob_gradients) {
  // Number of shots run by this process
  // The number of shots that the process ran is required by the supervisor to
  // combine the results. Because all backends may not guarantee that all of the
  // configured shots will be run, it must be sent here. This is calculated
  // by summing the counts of each qubit result in the map.
  auto map_values = results | std::views::values;
  int32_t num_shots =
      std::reduce(map_values.begin(), map_values.end(), INT32_C(0));
  mpi_manager.send_to_supervisor(
      std::span<serialisation::ShotCountType>{&num_shots, 1},
      MessageTags::SHOT_COUNT);

  // Results map
  std::vector<serialisation::ResultsType> packed_results =
      serialisation::pack_results_map(results);
  mpi_manager.send_to_supervisor(
      std::span<serialisation::ResultsType>{packed_results},
      MessageTags::RESULTS_MAP);

  // Native results map
  if (results_native) {
    auto packed_results_native =
        serialisation::pack_results_map(*results_native);
    mpi_manager.send_to_supervisor(
        std::span<serialisation::ResultsType>{packed_results_native},
        MessageTags::RESULTS_NATIVE_MAP);
  }

  // Out counts
  if (out_counts) {
    mpi_manager.send_to_supervisor(*out_counts, MessageTags::COUNTS);
  }

  // Out probs
  if (out_probs) {
    mpi_manager.send_to_supervisor(*out_probs, MessageTags::PROBABILITIES);
  }

  // Out gradients
  if (out_prob_gradients) {
    std::vector<serialisation::GradientsType> packed_gradients =
        serialisation::pack_gradients(*out_prob_gradients);
    mpi_manager.send_to_supervisor(
        std::span<serialisation::GradientsType>{packed_gradients},
        MessageTags::PROBABILITY_GRADIENTS);
  }
}

void collect_results_from_mpi_processes(
    MpiManager &mpi_manager, int32_t total_shots_requested,
    int32_t supervisor_shot_count, ResultsMap &results,
    std::optional<std::reference_wrapper<ResultsMap>> results_native,
    std::optional<std::span<Count>> out_counts,
    std::optional<std::span<Probability>> out_probs,
    std::optional<std::reference_wrapper<OutProbabilityGradients>>
        out_prob_gradients) {
  // Shot counts
  // Initialise to the number of MPI processes and set each to the supervisor
  // shot count. The vector stores shot counts for each process such that the
  // index into the vector corresponds to the MPI process ID. I.e. an MPI
  // process's ID can be used to index the vector and retrieve the shot count
  // for it.
  std::vector<serialisation::ShotCountType> shot_counts(
      mpi_manager.get_total_processes(), supervisor_shot_count);
  // Populate the other indexes with the shot counts of the relevant processes
  mpi_manager.receive_from_others<serialisation::ShotCountType>(
      MessageTags::SHOT_COUNT,
      [&shot_counts](int32_t id,
                     std::span<serialisation::ShotCountType> shot_count) {
        shot_counts[id] = shot_count.front();
      });

  // Results map
  mpi_manager.receive_from_others<serialisation::ResultsType>(
      MessageTags::RESULTS_MAP,
      [&results](int32_t id, std::span<serialisation::ResultsType> buffer) {
        serialisation::unpack_results_map(
            buffer,
            [&results](std::vector<bool> key, serialisation::CountsType value) {
              // Add the value to the map
              results[key] += value;
            });
      });

  // Native results map
  if (results_native) {
    mpi_manager.receive_from_others<serialisation::ResultsType>(
        MessageTags::RESULTS_MAP,
        [&results_native](int32_t id,
                          std::span<serialisation::ResultsType> buffer) {
          serialisation::unpack_results_map(
              buffer, [&results_native](std::vector<bool> key,
                                        serialisation::CountsType value) {
                // Add the value to the map
                results_native.value().get()[key] += value;
              });
        });
  }

  // Out counts
  if (out_counts) {
    mpi_manager.receive_from_others<serialisation::CountsType>(
        MessageTags::COUNTS,
        [&out_counts](int32_t id, std::span<serialisation::CountsType> buffer) {
          // Sum the out_counts from the process
          std::ranges::transform(*out_counts, buffer, out_counts->begin(),
                                 std::plus<>{});
        });
  }

  // Out probs
  if (out_probs) {
    // Rescale probabilities calculated by this process (supervisor)
    std::ranges::for_each(*out_probs,
                          [&shot_counts, total_shots_requested](auto &p) {
                            p = p * shot_counts[0] / total_shots_requested;
                          });

    mpi_manager.receive_from_others<serialisation::ProbabilitiesType>(
        MessageTags::PROBABILITIES,
        [&out_probs, total_shots_requested, &shot_counts](
            int32_t id, std::span<serialisation::ProbabilitiesType> buffer) {
          // Rescale probabililties calculated by the other process based on the
          // number of shots the process ran vs the total number of configured
          // shots and add the rescaled probability to the already rescaled
          // probabilities of this process
          const int32_t shots_for_process = shot_counts[id];
          const Probability shot_scaling_factor =
              static_cast<Probability>(shots_for_process) /
              total_shots_requested;
          std::ranges::transform(
              *out_probs, buffer, out_probs->begin(),
              [shot_scaling_factor](double supervisor_process_n,
                                    double other_process_n) {
                const double sum = supervisor_process_n +
                                   other_process_n * shot_scaling_factor;
                return supervisor_process_n +
                       other_process_n * shot_scaling_factor;
              });
        });
  }

  // Out gradients
  if (out_prob_gradients) {
    // Rescale gradients calculated by this process (supervisor)
    std::ranges::for_each(
        out_prob_gradients->get(),
        [&shot_counts, total_shots_requested](auto &row) {
          std::ranges::for_each(
              row, [&shot_counts, total_shots_requested](auto &p) {
                p = p * shot_counts[0] / total_shots_requested;
              });
        });

    mpi_manager.receive_from_others<serialisation::GradientsType>(
        MessageTags::PROBABILITY_GRADIENTS,
        [&out_prob_gradients, &shot_counts, total_shots_requested](
            int32_t id, std::span<serialisation::GradientsType> buffer) {
          // Unpack
          auto unpacked_gradients = serialisation::unpack_gradients(buffer);

          // Rescale probability gradients calculated by the other process based
          // on the number of shots the process ran vs the total number of
          // configured shots and add the rescaled probability gradient to the
          // already rescaled probability gradients of this process
          const auto shots_for_process = shot_counts[id];
          const auto shot_scaling_factor =
              static_cast<Probability>(shots_for_process) /
              total_shots_requested;
          auto out_prob_gradients_unpacked_gradients = ::ranges::views::zip(
              out_prob_gradients->get(), unpacked_gradients);
          std::ranges::for_each(
              out_prob_gradients_unpacked_gradients,
              [shot_scaling_factor](const auto &row_unpacked_row) {
                auto &[row, unpacked_row] = row_unpacked_row;
                std::ranges::transform(
                    row, unpacked_row, row.begin(),
                    [shot_scaling_factor](auto n, auto unpacked_n) {
                      return n + unpacked_n * shot_scaling_factor;
                    });
              });
        });
  }
}

} // namespace qristal::mpi
