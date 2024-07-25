// Copyright (c) 2023 Quantum Brilliance Pty Ltd
#ifndef _QB_BENCHMARK_CIRCUITFIDELITY_
#define _QB_BENCHMARK_CIRCUITFIDELITY_

#define ZIP_VIEW_INJECT_STD_VIEWS_NAMESPACE //to add zip to the std namespace
#include "qb/core/tools/zip_tool.hpp"
#include "qb/core/benchmark/Serializer.hpp"
#include "qb/core/benchmark/DataLoaderGenerator.hpp"
#include "qb/core/primitives.hpp"

namespace qb
{
    namespace benchmark
    {

        /**
        * @brief Circuit fidelity metric evaluation class templated for arbitrary @tparam ExecutableWorkflow workflows.
        *
        * @details This class may be used to evaluate the circuit fidelity metric for arbitrary templated @tparam
        * ExecutableWorkflow workflows. Compatible workflows need to be able to generate and serialize (i) measured
        * bit string counts, (ii) ideal bit string counts, and (iii) qb::session information.
        */
        template <ExecutableWorkflow WORKFLOW>
        requires CanStoreMeasuredCounts<WORKFLOW> && CanStoreIdealCounts<WORKFLOW> && CanStoreSessionInfos<WORKFLOW>
        class CircuitFidelity {
            public:
                /**
                * @brief Constructor for the circuit fidelity metric evaluation class.
                *
                * Arguments:
                * @param workflow the templated workflow object of type @tparam WORKFLOW to evaluated.
                *
                * @return ---
                */
                CircuitFidelity( WORKFLOW & workflow ) : workflow_(workflow) {}
                /**
                * @brief Evaluate the circuit fidelity for the given workflow.
                *
                * Arguments:
                * @param force_new optional boolean flag forcing a new execution of the workflow. Defaults to false.
                *
                * @return std::map<std::time_t, std::vector<double>> of calculated circuit fidelities mapped to the corresponding time stamp of the workflow execution.
                *
                * @details This member function initializes and utilizes a DataLoaderGenerator object to find already
                * serialized workflow execution results. The user may then choose to evaluate the circuit fidelities for
                * the already generated results or generate new results to evaluate. Consecutively, the circuit fidelities
                * for all workflow circuits is evaluated and returned in a std::map to the corresponding timestamp of
                * execution.
                */
                std::map< std::time_t, std::vector<double> > evaluate(const bool force_new = false) const;

            private:
                WORKFLOW& workflow_;
                const std::vector<Task> tasks_{Task::MeasureCounts, Task::IdealCounts, Task::Session};
        };

        /**
        * @brief Helper function to evalute the classical fidelity of two given shot count distributions.
        *
        * Arguments:
        * @param p Shot count distribution 1
        * @param q Shot count distribution 2.
        *
        * @return double The classical fidelity f(p,q).
        *
        * @details This function evaluates the classical fidelity f(p,q) = (sum_i sqrt(p(i)q(i)))^2 for
        * bit string probabilities p(i), q(i) of bit string i.
        */
        template <typename Bitstring, typename Value>
        double classical_fidelity(const std::map<Bitstring, Value>& p, const std::map<Bitstring, Value>& q) {
            double result = 0.0;
            //calculate total number of shots
            double n_p = sumMapValues(p);
            double n_q = sumMapValues(q);

            //calculate fidelity
            for (auto const & [bitstring, counts] : p) {
                if ( q.contains(bitstring) ) {
                    result += std::sqrt(static_cast<double>(counts)) * std::sqrt(static_cast<double>(q.at(bitstring)));
                }
            }
            return result * result / (n_p * n_q);
        }
        /**
        * @brief Helper function to evalute the classical fidelity of a given shot count distribution to an ideal uniform distribution.
        *
        * Arguments:
        * @param p Shot count distribution
        * @param n_qubits the number of qubits.
        *
        * @return double The classical fidelity f(p,u) to an ideal uniform distribution u.
        *
        * @details This function evaluates the classical fidelity f(p,u) = (sum_i sqrt(p(i) u(i)))^2 for
        * bit string probabilities p(i) and uniform distribution probabilities u(i) of bit string i.
        */
        template <typename Bitstring, typename Value>
        double classical_fidelity_to_uni(const std::map<Bitstring, Value>& p, const size_t n_qubits) {
            double result = 0.0;
            double uni_prob = 1.0 / std::pow(2, n_qubits);
            double n_p = sumMapValues(p);
            for (const auto & [ _, counts ] : p ) {
                result += std::sqrt(counts * uni_prob);
            }
            return result * result / n_p;
        }


        template <ExecutableWorkflow WORKFLOW>
        requires CanStoreMeasuredCounts<WORKFLOW> && CanStoreIdealCounts<WORKFLOW> && CanStoreSessionInfos<WORKFLOW>
        std::map<std::time_t, std::vector<double>> CircuitFidelity<WORKFLOW>::evaluate(const bool force_new) const {
            std::map<std::time_t, std::vector<double>> timestamp2fidelities;
            std::cout << "Evaluating circuit fidelities" << std::endl;

            //(1) initialize DataLoaderGenerator to either read in already stored results or generate new ones
            DataLoaderGenerator dlg(workflow_.get_identifier(), tasks_, force_new);
            dlg.execute(workflow_);

            //(2) obtain session info, ideal, and measured bitcounts
            std::vector<SessionInfo> session_infos = dlg.obtain_session_infos();
            std::vector<std::vector<std::map<std::vector<bool>, int>>> measured_bitcounts_collection = dlg.obtain_measured_counts();
            std::vector<std::vector<std::map<std::vector<bool>, int>>> ideal_bitcounts_collection = dlg.obtain_ideal_counts();
            std::vector<std::time_t> timestamps = dlg.get_timestamps();

            //(3) evaluate fidelity for circuit in each timestamp
            //f_circ(p_ideal, p_meas) = max { (Fc(p_ideal, p_meas) - Fc(p_ideal, p_uni)) / (1 - Fc(p_ideal, p_uni)), 0 }
            //where p_ideal and p_meas are given by measured_bitcounts and ideal_bitcounts, and p_uni is the uniform distribution
            //with p_uni(x) = p_uni(y) for all x,y
            //Fc is the classical fidelity given by Fc(P, Q) = (sum_x sqrt(P(x)Q(x)))^2
            for (const auto & [session_info, measured_bitcounts, ideal_bitcounts, timestamp] : std::ranges::views::zip(session_infos, measured_bitcounts_collection, ideal_bitcounts_collection, timestamps)) {
                std::vector<double> fidelities;
                size_t n_qubits = session_info.qns_[0][0];
                for (const auto & [measured_bitcount, ideal_bitcount] : std::ranges::views::zip(measured_bitcounts, ideal_bitcounts)) {
                    double fc_ideal_meas = classical_fidelity(measured_bitcount, ideal_bitcount);
                    double fc_ideal_uni = classical_fidelity_to_uni(ideal_bitcount, n_qubits);
                    //Circuit fidelity is not well-defined for ideal targets close to the uniform distribution!
                    if (fabs(fc_ideal_uni - 1.0) < 1e-6) {
                        std::cout << "Beware! The ideal probability distribution of your quantum circuit is too close to the uniform distribution! This will yield a 0.0 circuit fidelity!" << std::endl;
                        fidelities.push_back(0.0);
                        continue;
                    }
                    double fcirc = (fc_ideal_meas - fc_ideal_uni) / (1.0 - fc_ideal_uni);
                    fidelities.push_back(fcirc > 0.0 ? fcirc : 0.0);
                }
                timestamp2fidelities.insert(std::make_pair(timestamp, fidelities));
            }
            return timestamp2fidelities;
        }
    }
}

#endif
