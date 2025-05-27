#pragma once

#include <qristal/core/mpi/mpi_manager.hpp>
#include <qristal/core/mpi/results_types.hpp>

#include <cstdint>

namespace qristal::mpi {

/**
 * @brief The current process is one of potentially many MPI processes. To
 * parallelise shots across other MPI processes, this function determines the
 * number of shots a given process in an MPI pool will run. Unevenly divisible
 * shot counts are distributed across all worker processes other than the
 * supervisor. The supervisor has synchronisation overhead and will always
 * finish last when running the same number of shots as worker processes (when
 * processes are run across systems that perform the same).
 *
 * @param total_processes The total number of processes in the MPI pool
 * @param total_shots The total number of shots configured for the session
 * @param mpi_process_id This process's MPI ID
 * @return int32_t The number of shots this process has been partitioned to run
 */
int32_t shots_for_mpi_process(int32_t total_processes, int32_t total_shots,
                              int32_t mpi_process_id);

/**
 * @brief Sends results to the supervisor MPI process from an MPI worker process
 * @note This function is designed to be called from worker processes only
 *
 * @param mpi_manager The process's MPI manager
 * @param results The process's results map
 * @param results_native The native results map. Optional as not all session
 * instances calculate this output.
 * @param out_counts The out counts for the calculation. Optional as not all
 * session instances calculate this output.
 * @param out_probs The out probs for the calculation. Optional as not all
 * session instances calculate this output.
 * @param out_prob_gradients The out prob gradients for the calculation.
 * Optional as not all session instances calculate this output.
 */
void send_results_to_supervisor(
    MpiManager &mpi_manager, ResultsMap &results,
    std::optional<std::reference_wrapper<ResultsMap>> results_native,
    std::optional<std::span<Count>> out_counts,
    std::optional<std::span<Probability>> out_probs,
    std::optional<std::reference_wrapper<OutProbabilityGradients>>
        out_prob_gradients);

/**
 * @brief Receives results from all MPI worker processes and combines them with
 * the supervisor process's results. Result combination is different for each
 * result output type.
 * @note This function is designed to be called from the supervisor process only
 *
 * @param mpi_manager The process's MPI manager
 * @param total_shots_requested The total shot count for the session
 * @param supervisor_shot_count The number of shots the supervisor ran
 * @param results The process's results map
 * @param results_native The native results map. Optional as not all session
 * instances calculate this output.
 * @param out_counts The out counts for the calculation. Optional as not all
 * session instances calculate this output.
 * @param out_probs The out probs for the calculation. Optional as not all
 * session instances calculate this output.
 * @param out_prob_gradients The out prob gradients for the calculation.
 * Optional as not all session instances calculate this output.
 */
void collect_results_from_mpi_processes(
    MpiManager &mpi_manager, int32_t total_shots_requested,
    int32_t supervisor_shot_count, ResultsMap &results,
    std::optional<std::reference_wrapper<ResultsMap>> results_native,
    std::optional<std::span<Count>> out_counts,
    std::optional<std::span<Probability>> out_probs,
    std::optional<std::reference_wrapper<OutProbabilityGradients>>
        out_prob_gradients);
}
