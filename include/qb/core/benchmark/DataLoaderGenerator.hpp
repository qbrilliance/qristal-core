// Copyright (c) 2023 Quantum Brilliance Pty Ltd
#ifndef _QB_BENCHMARK_DATALOADERGENERATOR_
#define _QB_BENCHMARK_DATALOADERGENERATOR_

#include <string>
#include <filesystem>
#include <iostream>
#include <regex>
#include <ranges>

#include "qb/core/benchmark/Serializer.hpp" // contains session & typedefs
#include "qb/core/benchmark/Concepts.hpp"

namespace qb
{
    namespace benchmark
    {    

        /**
        * @brief Handler to retrieve and generate (workflow) data from serialized file storage.
        * 
        * @details Upon execution, workflows will store data in serialized archive files to be evaluated by (multiple) 
        * metric calculations. This class is used to (i) check available data files, (ii) load the data back 
        * into memory, and (iii) trigger additional workflow execution(s) in case of missing data. 
        * Any metric.evaluate() call will internally use DataLoaderGenerator. 
        */
        class DataLoaderGenerator
        {
            public:
                /**
                * @brief DataLoaderGenerator constructor.
                * 
                * Arguments: 
                * @param workflow_identifier a unique string workflow identifier set and stored in each workflow
                * @param metric_tasks a std::vector of required tasks for successful metric evaluation (e.g., quantum state fidelity evaluation requires measured and ideal densities while classical circuit fidelities require ideal and measured bitstring counts only)
                * (optional) @param force_new a boolean flag to force a new workflow execution. This is used in unit tests and omits checking already generated files to retrieve data. Defaults to false
                * (optional) @param verbose a boolean flag to print verbose messages to std::cout. Defaults to true
                * 
                * @return ---
                * 
                * @details Fills member @param metric_regex_ an std::vector of std::regex objects with regular file name expressions to look for each task and workflow  
                */
                DataLoaderGenerator(const std::string& workflow_identifier, const std::vector<Task>& metric_tasks, const bool force_new = false, const bool verbose = true) 
                : workflow_identifier_(workflow_identifier), metric_tasks_(metric_tasks), force_new_(force_new), verbose_(verbose) {
                    //build regex for each metric identifier (to be checked when calling load_available_timestamps)
                    for ( auto const & t : metric_tasks )
                        metric_regex_.push_back(std::regex("[a-zA-Z]+_" + get_identifier(t) + "_([0-9]+)\\.bin"));
                }

                /**
                * @brief Load all available timestamps of past workflow executions.
                * 
                * Arguments: ---
                * 
                * @return std::unordered_map<std::time_t, std::vector<std::string>> an unordered map container mapping avaible timestamps 
                * to the corresponding filenames.
                * 
                * @details This member function will check if folder @param SerializerConstants::INTERMEDIATE_RESULTS_FOLDER_NAME (static, defined in Seralizer.hpp) exists 
                * and will create it if not. All files within @param SerializerConstants::INTERMEDIATE_RESULTS_FOLDER_NAME are then checked against the stored 
                * regular expressions for the initialized workflow identifier.
                */
                std::unordered_map<std::time_t, std::vector<std::string>> loadAvailableTimestamps() const;

                /**
                * @brief Filter workflow timestamps of past executions by metric constraints.
                * 
                * Arguments: available_timestamps an unordered map of std::time_t timestamps mapped to an std::vector of std::string filenames as returned by loadAvailableTimestamps().
                * 
                * @return std::vector<time_t> a std::vector of compatible timestamps std::time_t.
                * 
                * @details This member function will filter all filenames associated with the identified available timestamps and 
                * return a vector of only the metric compatible ones.
                */
                std::vector<std::time_t> filterTimestamps(const std::unordered_map<std::time_t, std::vector<std::string>>& available_timestamps) const;

                /**
                * @brief Process user choice for the metric evaluation of compatible, already stored workflow results.
                * 
                * Arguments: 
                * @param matching_timestamps a std::vector of std::time_t timestamps compatible with the requested metric evaluation as returned by filterTimestamps().
                * 
                * @return bool Returns true if the user requested a new workflow execution, false otherwise.
                * 
                * @details The user is presented with options to either evaluate metrics for a specific time stamp, for a set of time stamps, or to 
                * generate new data by executing a new benchmark.
                */
                bool processUserInput(const std::vector<std::time_t>& matching_timestamps);

                /**
                * @brief Execute workflow and generate new serialized data.
                * 
                * Arguments: 
                * @tparam workflow a reference handle to an arbitrary templated workflow. 
                * 
                * @return ---
                * 
                * @details This member function will envoke the execute function of the passed workflow given the passed tasks required by the metric.
                */
                template <typename WORKFLOW>
                void executeWorkflow(WORKFLOW& workflow) {
                    if (verbose_) {
                        std::cout << "Executing workflow now." << std::endl; 
                    }
                    std::time_t time = workflow.execute(metric_tasks_); 
                    timestamps_.push_back(time);
                    if (verbose_) {
                        std::cout << "Done!" << std::endl;
                    }
                }

                /**
                * @brief Execute full DataLoaderGenerator suite checking if data for a specific workflow has been generated and stored and retrieve the execution timestamps
                * 
                * Arguments: 
                * @param workflow of templated type @tparam WORKFLOW the specific workflow to be checked.

                * @return true if timestamps were loaded succesfully, false if not. 
                * 
                * @details This member function will load available timestamps for the passed workflow, filter them based on the passed metric tasks, 
                * gather user input which stored results (if any) to use in the metric evaluation, and store the corresponding timestamps. With 
                * the force_new_ flag, a new execution (data generation) may be forced every time.
                */
                template <typename WORKFLOW>
                void execute(WORKFLOW& workflow )  {
                    //check if folder SerializerConstants::INTERMEDIATE_RESULTS_FOLDER_NAME exists, create if not
                    if ( std::filesystem::exists(std::filesystem::path(SerializerConstants::INTERMEDIATE_RESULTS_FOLDER_NAME)) == false ){
                        std::filesystem::create_directory(std::filesystem::path(SerializerConstants::INTERMEDIATE_RESULTS_FOLDER_NAME));
                    }

                    bool generate_new = true;
                    if (force_new_ == false) {
                        //load all available timestamps for the already stored intermediate results
                        auto available_timestamps = loadAvailableTimestamps();
                        //filter timestamps by metric constraints
                        auto matching_timestamps = filterTimestamps(available_timestamps);
                        //ask for user input which stored results (if any) should be processed, returns false for new execution
                        generate_new = processUserInput(matching_timestamps);
                    }

                    if (force_new_ || generate_new) {
                        if (verbose_) {
                            std::cout << (force_new_ ? "Forced new execution!\n" : ""); 
                        }
                        executeWorkflow<WORKFLOW>(workflow);
                    }
                }

                /**
                * @brief Deserialize measured bitstring count data from archived files for all stored time stamps
                * 
                * Arguments: ---

                * @return std::vector<std::vector<std::string>> a vector of bit strings (as std::string) for each stored time stamp.
                */
                std::vector<std::vector<std::string>> obtain_measured_counts() const { 
                    return load_data<BitCounts, std::vector<std::string>>(workflow_identifier_, "_measured_", timestamps_); 
                }
                /**
                * @brief Deserialize ideal bitstring count data from archived files for all stored time stamps
                * 
                * Arguments: ---

                * @return std::vector<std::vector<std::string>> a vector of bit strings (as std::string) for each stored time stamp.
                */
                std::vector<std::vector<std::string>> obtain_ideal_counts() const { 
                    return load_data<BitCounts, std::vector<std::string>>(workflow_identifier_, "_ideal_"   , timestamps_);
                }
                /**
                * @brief Deserialize ideal density data from archived files for all stored time stamps
                * 
                * Arguments: ---

                * @return std::vector<std::vector<Eigen::Matrix<std::complex<double>, Eigen::Dynamic, Eigen::Dynamic>>> a vector of complex matrices for each stored time stamp.
                */
                std::vector<std::vector<ComplexMatrix>> obtain_ideal_densities() const {
                    return load_data<ComplexMatrices, std::vector<ComplexMatrix>>(workflow_identifier_, "_densities_", timestamps_);
                }   
                /**
                * @brief Deserialize ideal process matrix data from archived files for all stored time stamps
                * 
                * Arguments: ---

                * @return std::vector<std::vector<Eigen::Matrix<std::complex<double>, Eigen::Dynamic, Eigen::Dynamic>>> a vector of complex matrices for each stored time stamp.
                */ 
                std::vector<std::vector<ComplexMatrix>> obtain_ideal_processes() const {
                    return load_data<ComplexMatrices, std::vector<ComplexMatrix>>(workflow_identifier_, "_processes_", timestamps_);
                }  
                /**
                * @brief Deserialize session information data from archived files for all stored time stamps
                * 
                * Arguments: ---

                * @return std::vector<SessionInfo> relevant restart (session) information for each stored time stamp.
                */
                std::vector<SessionInfo> obtain_session_infos() const { 
                    return load_data<SessionInfo, SessionInfo>(workflow_identifier_, "_session_" , timestamps_); 
                }

                /**
                * @brief Setter for member @param timestamps_.
                * 
                * Arguments: @param timestamps a vector of time stamps (std::time_t).

                * @return DataLoaderGenerator& a reference to updated this. 
                */
                void set_timestamps(const std::vector<std::time_t>& timestamps) {
                    timestamps_ = timestamps; 
                }
                /**
                * @brief Getter for member @param timestamps_.
                * 
                * Arguments: ---

                * @return const std::vector<std::time_t>& the content of @param timestamps_. 
                */
                const std::vector<std::time_t>& get_timestamps() const {return timestamps_;}
                /**
                * @brief Set forced workflow execution without checking compatible time stamps. 
                * 
                * Arguments: ---

                * @return --- 
                */
                void force_new_execution() {
                    force_new_ = true;
                }

            private:
                const std::string workflow_identifier_;
                const std::vector<Task> metric_tasks_;
                bool force_new_; 
                bool verbose_;

                std::vector<std::regex> metric_regex_; //to check all metric identifiers required by metric evaluation (e.g., "measured", "ideal" for CircuitFidelity)
                std::vector<std::time_t> timestamps_; //time stamps to read in (filled by load_available_timestamps)
        };


    } // namespace qb::benchmark
    
} // namespace qb

#endif 
