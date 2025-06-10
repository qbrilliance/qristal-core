// Copyright (c) Quantum Brilliance Pty Ltd
#ifndef _QB_BENCHMARK_RUNTIMEANALYZER_
#define _QB_BENCHMARK_RUNTIMEANALYZER_

#include <string>

#include <cppuprofile/uprofile.h>
#include <cppuprofile/monitors/nvidiamonitor.h>

#include <qristal/core/benchmark/Serializer.hpp> // contains <qb/core/session.hpp> & typedefs
#include <qristal/core/benchmark/Concepts.hpp>

namespace qristal
{
    namespace benchmark
    {
        /**
        * @brief Workflow wrapper to measure and store runtime information during execution.
        *
        * @details This workflow wraps around an arbitrary executable workflow and initializes the cppuprofile profiler to measure
        * runtime information in terms of CPU, RAM (process and system), and GPU utilization.
        */
        template <ExecutableWorkflow EXECWORKFLOW>
        class RuntimeAnalyzer : public EXECWORKFLOW
        {
            public:
                using ExecutableWorkflowType = EXECWORKFLOW; //expose wrapped executable workflow type

                /**
                * @brief Constructor for runtime profiling workflow wrapper
                *
                * Arguments:
                * @param workflow the wrapped executable workflow for which all executed tasks shall be monitored
                * @param sleep unsigned integer specifying the monitoring interval in ms.

                * @return ---
                */
                RuntimeAnalyzer(
                    EXECWORKFLOW& workflow,
                    const size_t& sleep = 1000
                ) : EXECWORKFLOW(workflow), sleep_(sleep), identifier_(workflow.get_identifier()) {}

                /**
                * @brief Run wrapped workflow, profile runtime data, and serialize wrapped workflow results
                *
                * Arguments:
                * @param tasks a selection of Tasks to be executed using the wrapped workflow.
                *
                * @return std::time_t the time stamp of the successful execution
                *
                * @details This member function is used to execute specific tasks the wrapped workflow is capable of. These could include storing
                * (i) the measured bit string counts after circuit execution,
                * (ii) the ideal (noise-free) bit string counts,
                * (iii) the ideal quantum state densities for each quantum circuit,
                * (iv) the ideal quantum process matrices for each quantum circuit, and
                * (v) the relevant information contained in the passed qb::session.
                * Beware that an actual circuit execution is only triggered for task (i).
                */
                std::time_t execute(const std::vector<Task>& tasks)
                {
                    std::time_t t = std::time(nullptr); //get timestamp of execution
                    for (const auto& task : tasks) {
                        std::cout << "Executing and profiling task " << get_identifier(task) << std::endl;
                        std::stringstream ss;
                        ss << SerializerConstants::INTERMEDIATE_RESULTS_FOLDER_NAME << "/" << identifier_ << "_runtime_" << get_identifier(task) << "_" << t << ".log";
                        uprofile::start(ss.str().c_str());
                        uprofile::startProcessMemoryMonitoring(sleep_);
                        uprofile::startSystemMemoryMonitoring(sleep_);
                        uprofile::startCPUUsageMonitoring(sleep_);
                        #ifdef GPU_MONITOR_NVIDIA
                            uprofile::addGPUMonitor(new uprofile::NvidiaMonitor);
                            uprofile::startGPUMemoryMonitoring(sleep_);
                            uprofile::startGPUUsageMonitoring(sleep_);
                        #endif
                        switch (task) {
                            case Task::MeasureCounts: {
                                executeWorkflowTask<EXECWORKFLOW, Task::MeasureCounts>()(*this, t);
                                break;
                            }
                            case Task::IdealCounts: {
                                executeWorkflowTask<EXECWORKFLOW, Task::IdealCounts>()(*this, t);
                                break;
                            }
                            case Task::IdealDensity: {
                                executeWorkflowTask<EXECWORKFLOW, Task::IdealDensity>()(*this, t);
                                break;
                            }
                            case Task::IdealProcess: {
                                executeWorkflowTask<EXECWORKFLOW, Task::IdealProcess>()(*this, t);
                                break;
                            }
                            case Task::Session: {
                                executeWorkflowTask<EXECWORKFLOW, Task::Session>()(*this, t);
                                break;
                            }
                        }
                        uprofile::stop();
                        std::cout << "Finished!" << std::endl;
                    }
                    return t;
                }

            private:

                size_t sleep_ = 1000; //waiting time until next measurement in ms
                const std::string identifier_;
        };

    } // namespace qb::benchmark
} // namespace qb

#endif
