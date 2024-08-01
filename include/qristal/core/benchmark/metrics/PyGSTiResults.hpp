// Copyright (c) 2024 Quantum Brilliance Pty Ltd
#ifndef _QB_BENCHMARK_PYGSTIRESULTS_
#define _QB_BENCHMARK_PYGSTIRESULTS_

#include <ranges>
#include <fstream>
#include <boost/dynamic_bitset.hpp>

#define ZIP_VIEW_INJECT_STD_VIEWS_NAMESPACE //to add zip to the std namespace
#include "qristal/core/tools/zip_tool.hpp"
#include "qristal/core/benchmark/Serializer.hpp"
#include "qristal/core/benchmark/DataLoaderGenerator.hpp"

namespace qristal
{
    namespace benchmark
    {

        /**
        * @brief Helper function writing a given pyGSTi compatible list of circuit results to file.
        *
        * Arguments:
        * @param pyGSTi_results the pyGSTi compatible list of circuit results given as a std::vector of std::string's.
        * @param filename a filename to store the results in.
        *
        * @return ---
        */
        inline void write_pyGSTi_results_to_file(const std::vector<std::string>& pyGSTi_results, const std::string& filename) {
            std::ofstream out(filename);
            for (const auto& line : pyGSTi_results) {
                out << line << std::endl;
            }
            out.close();
        }

        /**
        * @brief pyGSTi results metric evaluation class templated for arbitrary @tparam PyGSTiWorkflow workflows.
        *
        * @details This class may be used to print circuit results from a given templated @tparam PyGSTiWorkflow workflow
        * to a pyGSTi compatible format including the (single line) string representations of each circuit and their
        * measured counts. Compatible workflows need to be able to generate and serialize (i) measured bit string counts,
        * and (ii) qristal::session information.
        */
        template <PyGSTiWorkflow WORKFLOW>
        requires CanStoreMeasuredCounts<WORKFLOW> && CanStoreSessionInfos<WORKFLOW>
        class PyGSTiResults {
            public:
                /**
                * @brief Constructor for the PyGSTiResults metric evaluation class.
                *
                * Arguments:
                * @param workflow the templated workflow object of type @tparam WORKFLOW to be evaluated.
                *
                * @return ---
                */
                PyGSTiResults( WORKFLOW & workflow ) : workflow_(workflow) {}

                /**
                * @brief Compile a list of pyGSTi readable circuit results for the given workflow.
                *
                * Arguments:
                * @param force_new optional boolean flag forcing a new execution of the workflow. Defaults to false.
                * @param verbose optional boolean flag to print verbose messages to std::cout. Defaults to true
                *
                * @return std::map<std::time_t, std::vector<std::string>> of pyGSTi readable circuit results (line by line) mapped to the corresponding time stamp of the workflow execution.
                *
                * @details This member function initializes and utilizes a DataLoaderGenerator object to find already
                * serialized workflow execution results. The user may then choose to compile pyGSTi results for the
                * already generated results or generate new results to evaluate. Consecutively, a list of pyGSTi compatible
                * circuit results for all workflow circuits is compiled and returned in a std::map to the corresponding
                * timestamp of execution.
                */
                std::map< std::time_t, std::vector<std::string> > evaluate(const bool force_new = false, const bool verbose = true) const;

            private:
                WORKFLOW& workflow_;
                const std::vector<Task> tasks_{Task::MeasureCounts, Task::Session};
        };


        template <PyGSTiWorkflow WORKFLOW>
        requires CanStoreMeasuredCounts<WORKFLOW> && CanStoreSessionInfos<WORKFLOW>
        std::map<std::time_t, std::vector<std::string>> PyGSTiResults<WORKFLOW>::evaluate(const bool force_new, const bool verbose) const {
            std::map<std::time_t, std::vector<std::string>> timestamp2pyGSTiresults;
            if (verbose) {
                std::cout << "Evaluating pyGSTi compatible results list" << std::endl;
            }

            //(1) initialize DataLoaderGenerator to either read in already stored results or generate new ones
            DataLoaderGenerator dlg(workflow_.get_identifier(), tasks_, force_new, verbose);
            dlg.execute(workflow_);

            //(2) obtain session info, measured bitcounts, and timestamps
            std::vector<SessionInfo> session_infos = dlg.obtain_session_infos();
            std::vector<std::vector<std::map<std::vector<bool>, int>>> measured_bitcounts_collection = dlg.obtain_measured_counts();
            std::vector<std::time_t> timestamps = dlg.get_timestamps();

            //(3) compile list of results in pyGSTi compatible format
            for (const auto& [session_info, measured_bitcounts, timestamp] : std::ranges::views::zip(session_infos, measured_bitcounts_collection, timestamps)) {
                std::vector<std::string> circuit_results;
                const size_t n_qubits = session_info.qns_[0][0];
                size_t limit = std::pow(2, n_qubits);
                //print header (mandatory for pyGSTi)
                std::stringstream header;
                header << "## Columns = ";
                for (size_t i = 0; i < limit; ++i) {
                    header << (i > 0 ? ", " : "") << boost::dynamic_bitset<>(n_qubits, i) << " count";
                }
                circuit_results.push_back(header.str());
                //print circuit results
                for (const auto& [measured_bitcount, circuit_string] : std::ranges::views::zip(measured_bitcounts, workflow_.get_pyGSTi_circuit_strings())) {
                    std::stringstream ss;
                    ss << circuit_string;
                    for (size_t i = 0; i < limit; ++i) {
                        boost::dynamic_bitset<> b(n_qubits, i);
                        std::vector<bool> bitvec(n_qubits);
                        for (uint i=0; i < n_qubits; i++) bitvec[i] = b[i];
                        if (measured_bitcount.find(bitvec) != measured_bitcount.end()) {
                            ss << " " << measured_bitcount.at(bitvec);
                        }
                        else {
                            ss << " 0";
                        }
                    }
                    circuit_results.push_back(ss.str());
                }
                timestamp2pyGSTiresults[timestamp] = circuit_results;
            }

            return timestamp2pyGSTiresults;
        }
    }
}

#endif
