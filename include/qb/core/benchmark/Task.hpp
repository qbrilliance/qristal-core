#ifndef _QB_BENCHMARK_TASK_
#define _QB_BENCHMARK_TASK_

#include "qb/core/circuit_builder.hpp"

namespace qb {
    namespace benchmark {

        /**
        * @brief This container holds all tasks executable (and serializable) by benchmarking workflows
        *
        * @details Depending on the metric, benchmarked workflows need to be able to execute certain tasks and thereby 
        * produce storable information to be processed by the metric. These specific tasks are collected in this 
        * struct.
        */
        enum struct Task
        {
            MeasureCounts, IdealCounts, IdealDensity, IdealProcess, Session
        };

        /**
        * @brief Static function to convert a given Task to its identifier string.
        */
        static std::string get_identifier(Task const & t)
        {
            switch (t) {
                case Task::MeasureCounts:
                    return "measured";
                case Task::IdealCounts: 
                    return "ideal";
                case Task::IdealDensity:
                    return "densities";
                case Task::IdealProcess:
                    return "processes";
                case Task::Session:
                    return "session";
            }
        }

        /**
        * @brief Templated functor class handling Task execution by arbitrary workflow objects.
        * 
        * @details This class may be (partially or fully) specialized for specific tasks and workflows to implement workflow/task specific behaviour. E.g., 
        * different workflows might have different simplified techniques to obtain the ideal quantum state densitites (see, e.g., specialization for SPAMBenchmark)
        */
        template <typename WORKFLOW, Task task>
        class executeWorkflowTask {
            public:
                /**
                * @brief Virtual executor function for a templated workflow.
                * 
                * Arguments: 
                * @param workflow Templated workflow reference
                * @param timestamp std::time_t time stamp of execution.
                * 
                * @return --- 
                * 
                * @details This executor function is the main Task execution function. Specialize this for every workflow/task combination deviating from the templated 
                * implementation below.
                */
                void operator()(WORKFLOW& workflow, std::time_t timestamp) const {}
            private:
        };

        /**
        * @brief Partially specialized functor class handling Task::MeasureCounts execution by arbitrary workflow objects.
        */
        template <typename WORKFLOW> 
        class executeWorkflowTask<WORKFLOW, Task::MeasureCounts> {
            public: 
                /**
                * @brief Partially specialized executor function for Task::MeasureCounts and arbitrary templated workflow.
                * 
                * Arguments: 
                * @param workflow Templated workflow reference
                * @param timestamp std::time_t time stamp of execution.
                * 
                * @return --- 
                * 
                * @details This is the default implementation of the Task::MeasureCounts task. It will generate all workflow circuits via the get_circuits() member, 
                * iterate through them, add measurements to all qubits, set the intermediate representation target of the workflow's session object, run the circuit, 
                * and store the qb::String results in a std::vector container, which is serialized in the final step.
                */
                void operator()(WORKFLOW& workflow, std::time_t timestamp) const {
                    qb::String measured_results;
                    std::vector<qb::CircuitBuilder> circuits = workflow.get_circuits();
                    for ( auto& circuit : circuits ) {
                        circuit.MeasureAll(workflow.get_session().get_qns()[0][0]); 
                        workflow.set_session().set_irtarget_m(circuit.get());
                        workflow.set_session().run();
                        measured_results.push_back(workflow.get_session().get_out_raws()[0][0]);
                    }
                    workflow.serialize_measured_counts(measured_results, timestamp);
                }
        };

        /**
        * @brief Partially specialized functor class handling Task::Session execution by arbitrary workflow objects.
        */
        template <typename WORKFLOW>
        class executeWorkflowTask<WORKFLOW, Task::Session> {
            public:
                /**
                * @brief Partially specialized executor function for Task::Session and arbitrary templated workflow.
                * 
                * Arguments: 
                * @param workflow Templated workflow reference
                * @param timestamp std::time_t time stamp of execution.
                * 
                * @return --- 
                * 
                * @details This is the default implementation of the Task::Session task. It will serialize the workflow's session info.
                */
                void operator()(WORKFLOW& workflow, std::time_t timestamp) const {
                    workflow.serialize_session_infos(timestamp);
                }
        };

        /**
        * @brief Function handle to call from arbitrary workflows executing a collection of Tasks.
        * 
        * Arguments: 
        * @param workflow Templated workflow reference 
        * @param tasks A std::vector of Task objects to be executed.
        * 
        * @return std::time_t The time stamp of execution. 
        * 
        * @details General execution of a collection of Tasks using an arbitrary templated workflow. This function will generate a time stamp, iterate 
        * through all given tasks, and call the respective (and potentially specialized) executeWorkflowTask functor. 
        */
        template <typename WORKFLOW>
        std::time_t executeWorkflowTasks(WORKFLOW& workflow, const std::vector<Task>& tasks) {
            std::time_t t = std::time(nullptr); //get timestamp of execution
            for (const auto& task : tasks) {
                switch (task) {
                    case Task::MeasureCounts: {
                        executeWorkflowTask<WORKFLOW, Task::MeasureCounts>()(workflow, t); 
                        break;
                    }
                    case Task::IdealCounts: {
                        executeWorkflowTask<WORKFLOW, Task::IdealCounts>()(workflow, t); 
                        break;
                    }
                    case Task::IdealDensity: {
                        executeWorkflowTask<WORKFLOW, Task::IdealDensity>()(workflow, t); 
                        break;
                    }
                    case Task::IdealProcess: {
                        executeWorkflowTask<WORKFLOW, Task::IdealProcess>()(workflow, t); 
                        break;
                    }
                    case Task::Session: {
                        executeWorkflowTask<WORKFLOW, Task::Session>()(workflow, t); 
                        break;
                    }
                }
            }
            return t;
        }


    }
}

#endif