// Copyright (c) Quantum Brilliance Pty Ltd
#ifndef _QB_BENCHMARK_WORKFLOWADDINS_
#define _QB_BENCHMARK_WORKFLOWADDINS_

#include <qristal/core/benchmark/Serializer.hpp>
#include <qristal/core/benchmark/Task.hpp>
#include <qristal/core/benchmark/workflows/QuantumProcessTomography.hpp>

#include <boost/dynamic_bitset.hpp>

namespace qristal {
  namespace benchmark {

    /**
    * @brief Helper function to obtain the ideal state vector of a given circuit.
    *
    * Arguments:
    * @param circuit the quantum circuit to measure given as a qristal::Circuitbuilder object.
    * @param n_qubits the number of qubits in the circuit.
    * @param msb optional boolean flag to change bit string ordering. Defaults to msb = false (lsb ordering).
    *
    * @return std::vector<std::complex<double>> the ideal state vector. 
    */
    inline std::vector<std::complex<double>> obtain_ideal_statevec(qristal::CircuitBuilder& circuit, const size_t n_qubits, const bool msb = false) {
        //(1) Define session (to obtain ideal state vector)
        qristal::session sim(msb); //use LSB by default: 00, 10, 01, 11
        sim.qn = n_qubits;
        sim.sn = 1;
        sim.acc = "qpp";
        sim.nooptimise = true;
        sim.calc_state_vec = true; 

        //(2) Run circuit and retrieve ideal state vector 
        sim.irtarget = circuit.get();
        sim.run(); 
        return sim.state_vec();
    }

    /**
    * @brief Helper function to convert a state vector to a density matrix.
    *
    * Arguments:
    * @param statevec the complex-valued state vector.
    *
    * @return ComplexMatrix: the converted complex-valued density matrix.
    */
    inline ComplexMatrix statevec2density(const std::vector<std::complex<double>>& statevec) {
        //convert state vector to Eigen vector  
        Eigen::VectorXcd statevec_eigen(statevec.size());
        for (size_t i = 0; i < statevec.size(); ++i) {
            statevec_eigen(i) = statevec[i];
        }
        //compute density from outer product: rho = |psi⟩⟨psi|
        ComplexMatrix density = statevec_eigen * statevec_eigen.adjoint(); 
        return density;
    }

    /**
    * @brief Templated class prototype for workflow addins from ideal simulations. 
    */
    template <typename WORKFLOW, Task task>
    class AddinFromIdealSimulation : public WORKFLOW {};

    /**
    * @brief Specialized workflow addin for adding ideal bit string counts from ideal simulation.
    */
    template <typename WORKFLOW>
    class AddinFromIdealSimulation<WORKFLOW, Task::IdealCounts> : public WORKFLOW {
        public: 
            /**
            * @brief Constructor for workflow wrapper to add in ideal bit count generation from ideal simulations. 
            *
            * Arguments:
            * @param workflow the wrapped workflow.
            * @return ---
            */
            AddinFromIdealSimulation(const WORKFLOW& workflow) : WORKFLOW(workflow) {}

            /**
            * @brief Run wrapped workflow and store results for specific tasks.
            *
            * Arguments:
            * @param tasks a selection of Tasks to be executed.
            *
            * @return std::time_t the time stamp of the successful execution.
            *
            * @details This member function is used to execute specific tasks of the wrapped workflow contained in AddinFromIdealSimulation. 
            * These include the generation of ideal bit string counts as well as all the compatible tasks of the wrapped workflow. 
            */
            std::time_t execute(const std::vector<Task>& tasks) {
                return executeWorkflowTasks<AddinFromIdealSimulation<WORKFLOW, Task::IdealCounts>>(*this, tasks);
            }

            /**
            * @brief Serialization method for ideal bit string counts
            *
            * Arguments:
            * @param counts the ideal bit string counts
            * @param time the time stamp of execution
            *
            * @return ---
            */
            void serialize_ideal_counts(const std::vector<std::map<std::vector<bool>, int>>& counts, const std::time_t time) const {
                save_data<BitCounts, std::vector<std::map<std::vector<bool>, int>>>(this->get_identifier(), "_ideal_", counts, time);
            }
    };

    /**
    * @brief Specialized workflow addin for adding ideal densities from ideal simulation.
    */
    template <typename WORKFLOW>
    class AddinFromIdealSimulation<WORKFLOW, Task::IdealDensity> : public WORKFLOW {
        public: 
            /**
            * @brief Constructor for workflow wrapper to add in ideal density generation from ideal simulations. 
            *
            * Arguments:
            * @param workflow the wrapped workflow.
            * @return ---
            */
            AddinFromIdealSimulation(const WORKFLOW& workflow) : WORKFLOW(workflow) {}

            /**
            * @brief Run wrapped workflow and store results for specific tasks.
            *
            * Arguments:
            * @param tasks a selection of Tasks to be executed.
            *
            * @return std::time_t the time stamp of the successful execution.
            *
            * @details This member function is used to execute specific tasks of the wrapped workflow contained in AddinFromIdealSimulation. 
            * These include the generation of ideal densities as well as all the compatible tasks of the wrapped workflow. 
            */
            std::time_t execute(const std::vector<Task>& tasks) {
                return executeWorkflowTasks<AddinFromIdealSimulation<WORKFLOW, Task::IdealDensity>>(*this, tasks);
            }

            /**
            * @brief Serialization method for ideal quantum state densities
            *
            * Arguments:
            * @param densities the ideal quantum state densities
            * @param time the time stamp of execution
            *
            * @return ---
            */
            void serialize_ideal_densities(const std::vector<ComplexMatrix>& densities, const std::time_t time) const {
                save_data<ComplexMatrices, std::vector<ComplexMatrix>>(this->get_identifier(), "_densities_", densities, time);
            }
    };

    /**
    * @brief Specialized workflow addin for adding ideal process matrices from ideal simulation.
    */
    template <typename WORKFLOW>
    class AddinFromIdealSimulation<WORKFLOW, Task::IdealProcess> : public WORKFLOW {
        public: 
            /**
            * @brief Constructor for workflow wrapper to add in ideal process matrix generation from ideal simulations. 
            *
            * Arguments:
            * @param workflow the wrapped workflow.
            * @return ---
            */
            AddinFromIdealSimulation(const WORKFLOW& workflow) : WORKFLOW(workflow) {}

            /**
            * @brief Run wrapped workflow and store results for specific tasks.
            *
            * Arguments:
            * @param tasks a selection of Tasks to be executed.
            *
            * @return std::time_t the time stamp of the successful execution.
            *
            * @details This member function is used to execute specific tasks of the wrapped workflow contained in AddinFromIdealSimulation. 
            * These include the generation of ideal process matrices as well as all the compatible tasks of the wrapped workflow. 
            */
            std::time_t execute(const std::vector<Task>& tasks) {
                return executeWorkflowTasks<AddinFromIdealSimulation<WORKFLOW, Task::IdealProcess>>(*this, tasks);
            }

            /**
            * @brief Serialization method for ideal quantum process matrices
            *
            * Arguments:
            * @param processes the ideal quantum process matrices
            * @param time the time stamp of execution
            *
            * @return ---
            */
            void serialize_ideal_processes( const std::vector<ComplexMatrix>& processes, const std::time_t time ) const {
                save_data<ComplexMatrices, std::vector<ComplexMatrix>>(this->get_identifier(), "_processes_", processes, time);
            }
    };


    /**
    * @brief Fully specialized execute functor for the Task::IdealCounts task of the templated AddinFromIdealSimulation workflow addin.
    */
    template <typename WORKFLOW>
    class executeWorkflowTask<AddinFromIdealSimulation<WORKFLOW, Task::IdealCounts>, Task::IdealCounts> {
        public: 
            /**
            * @brief Specialized member function generating and serializing the ideal bit string counts of the workflow wrapped within the AddinFromIdealSimulation object.
            *
            * Arguments:
            * @param workflow A templated AddinFromIdealSimulation reference
            * @param timestamp The time stamp of execution.
            *
            * @return ---
            */
            void operator()(AddinFromIdealSimulation<WORKFLOW, Task::IdealCounts>& workflow, std::time_t timestamp) const {
                std::cout << "Performing an ideal simulation to obtain ideal counts for " << workflow.get_session().sn << " shots" << std::endl;
                std::vector<std::map<std::vector<bool>, int>> ideal_counts; 
                for (qristal::CircuitBuilder& circuit : workflow.get_circuits()) {
                    std::map<std::vector<bool>, int> counts;
                    auto statevec = obtain_ideal_statevec(circuit, workflow.get_session().qn, false);
                    for (size_t i = 0; i < statevec.size(); ++i) {
                        //(1) assemble bit string 
                        boost::dynamic_bitset<> bits(workflow.get_session().qn, i);
                        std::vector<bool> bits_vec(bits.size(), 0);
                        for (std::size_t i = 0; i < bits.size(); ++i) {
                            bits_vec[i] = bits[i];
                        }
                        //(2) add to counts map 
                        counts[bits_vec] = std::round((statevec[i] * std::conj(statevec[i])).real() * workflow.get_session().sn);
                    }
                    ideal_counts.push_back(counts);
                }
                //(3) Serialize ideal counts
                workflow.serialize_ideal_counts(ideal_counts, timestamp);
            }
    };

    /**
    * @brief Fully specialized execute functor for the Task::IdealDensity task of the templated AddinFromIdealSimulation workflow addin.
    */
    template <typename WORKFLOW>
    class executeWorkflowTask<AddinFromIdealSimulation<WORKFLOW, Task::IdealDensity>, Task::IdealDensity> {
        public: 
            /**
            * @brief Specialized member function generating and serializing the ideal densities of the workflow wrapped within the AddinFromIdealSimulation object.
            *
            * Arguments:
            * @param workflow A templated AddinFromIdealSimulation reference
            * @param timestamp The time stamp of execution.
            *
            * @return ---
            */
            void operator()(AddinFromIdealSimulation<WORKFLOW, Task::IdealDensity>& workflow, std::time_t timestamp) const {
                std::cout << "Performing an ideal simulation to obtain the full n-qubit density matrices" << std::endl;
                std::vector<ComplexMatrix> densities; 
                for (qristal::CircuitBuilder& circuit : workflow.get_circuits()) {
                    densities.push_back(statevec2density(obtain_ideal_statevec(circuit, workflow.get_session().qn)));
                }
                workflow.serialize_ideal_densities(densities, timestamp);
            }
    };

    /**
    * @brief Fully specialized execute functor for the Task::IdealProcess task of the templated AddinFromIdealSimulation workflow addin.
    */
    template <typename WORKFLOW>
    class executeWorkflowTask<AddinFromIdealSimulation<WORKFLOW, Task::IdealProcess>, Task::IdealProcess> {
        public: 
            /**
            * @brief Specialized member function generating and serializing the ideal process matrices of the workflow wrapped within the AddinFromIdealSimulation object.
            *
            * Arguments:
            * @param workflow A templated AddinFromIdealSimulation reference
            * @param timestamp The time stamp of execution.
            *
            * @return ---
            */
            void operator()(AddinFromIdealSimulation<WORKFLOW, Task::IdealProcess>& workflow, std::time_t timestamp) const {
                std::cout << "Performing an ideal simulation to obtain the full n-qubit process matrices" << std::endl;
                std::vector<ComplexMatrix> densities;
                QuantumStateTomography<WORKFLOW> qst_workflow(workflow); 
                QuantumProcessTomography<QuantumStateTomography<WORKFLOW>> qpt_workflow(qst_workflow);
                for (auto& w : workflow.get_circuits()) {
                    //(1) build exact density matrices for each of the 4^N qpt circuits
                    for (auto& iw : qpt_workflow.prepend_state_initializations(w)) {
                        densities.push_back(statevec2density(obtain_ideal_statevec(iw, workflow.get_session().qn)));
                    }
                }
                //(2) assemble ideal processes from all exact densities 
                std::vector<ComplexMatrix> processes = qpt_workflow.assemble_processes(densities);

                //(3) Serialize ideal processes
                workflow.serialize_ideal_processes(processes, timestamp);
            }
    };


  }
}


#endif //_QB_BENCHMARK_WORKFLOWADDINS_