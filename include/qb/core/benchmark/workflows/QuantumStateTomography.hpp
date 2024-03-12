// Copyright (c) 2023 Quantum Brilliance Pty Ltd
#ifndef _QB_BENCHMARK_QUANTUMSTATETOMOGRAPHY_
#define _QB_BENCHMARK_QUANTUMSTATETOMOGRAPHY_

#include <string>
#include <vector>
#include <filesystem>
#include <map>
#include <ranges>

#define ZIP_VIEW_INJECT_STD_VIEWS_NAMESPACE //to add zip to the std namespace
#include "qb/core/tools/zip_tool.hpp"
#include "qb/core/benchmark/Serializer.hpp" // contains "qb/core/session.hpp" & typedefs
#include "qb/core/benchmark/Concepts.hpp"
#include "qb/core/primitives.hpp"

namespace qb
{
    namespace benchmark
    {   
        /**
        * @brief Standard quantum state tomography workflow templated for arbitrary wrapped workflows and measurement bases
        * 
        * @details This workflow class may be used to execute standard quantum state tomography experiments. It is templated and wrapped around arbitrary @tparam ExecutableWorkflow objects and measurement bases @tparam Symbol. 
        * Compatible measurement bases are required to be (i) matrix translatable, (ii) circuit appendable, and (iii) posess an identity. The latter restriction helps to reduce the computational overhead. The workflow may be 
        * used in metric evaluations that require measured bit string counts and ideal quantum state densities if provided by the wrapped workflow. Beware that producing ideal state densities is not a requirement of the 
        * quantum state tomography workflow (this workflow) but possibly by a consecutively calculated metric like, e.g., the quantum state fidelity.  
        */
        template <ExecutableWorkflow EXECWORKFLOW, typename SYMBOL = Pauli>
            requires MatrixTranslatable<SYMBOL> && CircuitAppendable<SYMBOL> && HasIdentity<SYMBOL>
        class QuantumStateTomography 
        { 
            public:
                using Symbol = SYMBOL; //expose measurement basis symbol type
                using ExecutableWorkflowType = EXECWORKFLOW; //expose wrapped executable workflow type

                /**
                * @brief Constructor for standard quantum state tomography workflows on a specific set of qubits.
                * 
                * Arguments: 
                * @param workflow the wrapped @tparam ExecutableWorkflow the quantum state tomography is acted upon.
                * @param qubits the list of qubit indices that are measured
                * @param basis list of the measured one qubit basis symbols (excluding the identity). Defaults to Pauli X, Y, and Z. 
                * @param use_for_identity the basis symbol used to resolve the identity in the QST protocol. Defaults to Pauli::Z.
                * 
                * @return ---
                */
                QuantumStateTomography( 
                    EXECWORKFLOW& workflow, 
                    const std::vector<size_t>& qubits, 
                    const std::vector<SYMBOL>& basis = std::vector<Pauli>{Pauli::Symbol::X, Pauli::Symbol::Y, Pauli::Symbol::Z}, 
                    const SYMBOL& use_for_identity = Pauli::Symbol::Z
                ) : workflow_(workflow), identifier_(std::string("QST") + workflow_.get_identifier()), qubits_(qubits), basis_(basis), use_for_identity_(use_for_identity) {}

                /**
                * @brief Constructor for standard quantum state tomography workflows for all involved qubits. 
                * 
                * Arguments: 
                * @param workflow the wrapped @tparam ExecutableWorkflow the quantum state tomography is acted upon.
                * @param basis list of the measured one qubit basis symbols (excluding the identity), defaults to Pauli X, Y, and Z.
                * @param use_for_identity the basis symbol used to resolve the identity in the QST protocol. Defaults to Pauli::Z. 
                * 
                * @return ---
                */
                QuantumStateTomography( 
                    EXECWORKFLOW& workflow, 
                    const std::vector<SYMBOL>& basis = std::vector<Pauli>{Pauli::Symbol::X, Pauli::Symbol::Y, Pauli::Symbol::Z}, 
                    const SYMBOL& use_for_identity = Pauli::Symbol::Z
                ) : workflow_(workflow), identifier_(std::string("QST") + workflow_.get_identifier()), basis_(basis), use_for_identity_(use_for_identity) {
                    qubits_ = std::vector<size_t>(workflow_.get_session().get_qns()[0][0]); 
                    std::iota(qubits_.begin(), qubits_.end(), 0);
                }

                std::vector<qb::CircuitBuilder> append_measurement_bases(qb::CircuitBuilder & workflow_circuit) const {
                    std::vector<qb::CircuitBuilder> circuits;
                    size_t n_qubit_basis_states = std::pow(basis_.size(), qubits_.size());
                    for (size_t basis_index = 0; basis_index < n_qubit_basis_states; ++basis_index) //there are (#basis states)^n_qubits combinations!
                    {
                        //create empty circuit and append workflow 
                        qb::CircuitBuilder cb; 
                        cb.append(workflow_circuit);
                        //convert i to x-nary number of length qubits.size() to find out which basis rotation to apply on which qubit
                        std::vector<size_t> indices = convert_decimal(basis_index, basis_.size(), qubits_.size());
                        for (const auto& [xnary_basis_index, qubit_index] : std::ranges::views::zip(indices, qubits_)) {
                            basis_[xnary_basis_index].append_circuit(cb, qubit_index); //append basis rotation gate(s) on ith qubit 
                        }
                        circuits.push_back(cb);
                    }
                    return circuits;
                }

                /**
                * @brief Run quantum state tomography workflow and store results for specific tasks.
                * 
                * Arguments: 
                * @param tasks a selection of Tasks to be executed using the initialized quantum state tomography workflow.
                * 
                * @return std::time_t the time stamp of the successful execution.
                * 
                * @details This member function is used to execute specific tasks the quantum state tomography workflow is capable of. These include storing 
                * (i) the measured bit string counts after circuit execution of the wrapped (and basis transformed) workflow circuits, 
                * (ii) the ideal quantum state densities for each quantum circuit of the wrapped workflow,
                * (iii) the relevant information contained in the passed qb::session. 
                * Beware that an actual circuit execution is only triggered for task (i) and that task (ii) will delegate IdealDensity tasks to the wrapped workflow only. 
                */
                std::time_t execute(const std::vector<Task>& tasks) 
                {
                    return executeWorkflowTasks<QuantumStateTomography<EXECWORKFLOW, SYMBOL>>(*this, tasks);
                }
                /**
                * @brief Run quantum state tomography workflow and store results for all possible tasks.
                * 
                * Arguments: ---
                * 
                * @return std::time_t the time stamp of the successful execution.
                * 
                * @details This member function is used to execute all available tasks the quantum state tomography workflow is capable of. These include storing 
                * (i) the measured bit string counts after circuit execution of the wrapped (and basis transformed) workflow circuits, 
                * (ii) the ideal quantum state densities for each quantum circuit of the wrapped workflow,
                * (iii) the relevant information contained in the passed qb::session. 
                * Beware that an actual circuit execution is only triggered for task (i) and that task (ii) will delegate IdealDensity tasks to the wrapped workflow only. 
                */
                std::time_t execute_all() 
                {
                    std::time_t t = execute(std::vector<Task>{Task::MeasureCounts, Task::IdealDensity, Task::Session});
                    return t;
                }

                /**
                * @brief Calculate density matrices from measured bit string counts of the quantum state tomography workflow.
                * 
                * Arguments:
                * @param measurement_counts the measured bit string counts as serialized by the execute function. Contains n * 3^q bit string histograms for n wrapped workflow circuits and q measured qubits. 
                * 
                * @return std::vector<ComplexMatrix> the quantum state density matrices obtained through the quantum state tomography experiments.
                * 
                * @details This member function will iterate over all sets of 3^q (for q measured qubits) measured bit string counts and calculate one complex density matrix for each set by 
                * (i) reconstructing the measurement basis that was used for a given set of measured bit string counts, 
                * (ii) augmenting the original measurement basis to the accessible basis strings by resolving all identities with the chosen symbol, 
                * (iii) evaluating the measured expectation values for each basis string 
                * (iv) adding the corresponding contribution to the individual (zero initialized) complex density matrices. 
                */
                std::vector<ComplexMatrix> assemble_densities(const std::vector<std::string>& measurement_counts) const
                {
                    std::vector<ComplexMatrix> densities; 
                    size_t density_dimension = std::pow(2, qubits_.size()); 
                    size_t n_qubit_basis_size = std::pow(basis_.size(), qubits_.size());
                    size_t task_step = std::pow(3, qubits_.size());
                    for (size_t task = 0; task < measurement_counts.size(); task += task_step) { //create one density matrix for each task aka workflow circuit
                        ComplexMatrix density = ComplexMatrix::Zero(density_dimension, density_dimension);
                        for (size_t measurement = 0; measurement < n_qubit_basis_size; ++measurement) { //Each circuit was measured 3^n times 
                            const std::map<std::string, size_t> counts = convert_to_counts_map(measurement_counts[task + measurement], qubits_.size()); //convert string to counts map
                            std::vector<std::vector<SYMBOL>> accessible_bases; // collect all accessible bases for the given measurement (e.g., IX and ZX from ZX)
                            //convert i to x-nary number of length qubits.size() to find out which basis rotation to apply on which qubit
                            std::vector<size_t> indices = convert_decimal(measurement, basis_.size(), qubits_.size());
                            //handle the first symbol explicitly
                            accessible_bases.push_back(std::vector<SYMBOL>{basis_[indices[0]]}); 
                            if (basis_[indices[0]] == use_for_identity_) {
                                accessible_bases.push_back(std::vector<SYMBOL>{get_identity<Symbol>()});
                            }
                            //handle the remaining ones by iterating over all already found bases and augmenting
                            for (size_t q = 1; q < indices.size(); ++q) {
                                std::vector<std::vector<SYMBOL>> new_bases; 
                                for (const auto& basis : accessible_bases) {
                                    new_bases.push_back(basis); 
                                    new_bases.back().push_back(basis_[indices[q]]); 
                                    if (basis_[indices[q]] == use_for_identity_) {
                                        new_bases.push_back(basis); 
                                        new_bases.back().push_back(get_identity<Symbol>());
                                    }
                                }
                                accessible_bases = new_bases;
                            }
                            //now go through all measured counts
                            std::vector<double> exp_values(accessible_bases.size(), 0.0); //initialize zero expectation values for each basis  
                            for (const auto& [bitstring, count] : counts) {
                                for (const auto& [exp_value, accessible_base] : std::ranges::views::zip(exp_values, accessible_bases)) {
                                    //evaluate sign with which the measured bitstring contributes to all basis expectation values 
                                    int sign = evaluate_sign(bitstring, accessible_base); 
                                    exp_value += static_cast<double>(sign) * static_cast<double>(count) / static_cast<double>(workflow_.get_session().get_sns()[0][0]);
                                }
                            }
                            //finally go through all expectation values, build full matrix representation of basis and add to density matrix 
                            for (const auto & [exp_value, accessible_base] : std::ranges::views::zip(exp_values, accessible_bases)) {
                                density += exp_value * calculate_Kronecker_product(accessible_base);
                            }
                        }
                        densities.push_back(1.0 / pow(2, static_cast<double>(qubits_.size())) * density); //push normalized density to collection
                    } 

                    return densities; 
                }

                /**
                * @brief Serialization method for measured bit string counts
                * 
                * Arguments: 
                * @param counts the measured bit string counts returned by qb::session 
                * @param time the time stamp of execution
                * 
                * @return ---
                */
                void serialize_measured_counts( const qb::String& counts, const std::time_t time ) const { 
                    save_data<BitCounts, qb::String>(identifier_, "_measured_", counts, time); 
                }
                /**
                * @brief Serialization method for the assigned qb::session
                * 
                * Arguments: 
                * @param time the time stamp of execution
                * 
                * @return ---
                */
                void serialize_session_infos( const std::time_t time ) const { 
                    save_data<SessionInfo, SessionInfo>(identifier_, "_session_" , workflow_.get_session(), time); 
                }

                /**
                * @brief Return a constant reference to the one qubit measurement basis.
                */
                const std::vector<SYMBOL>& get_basis() const {return basis_;}
                /**
                * @brief Return a constant reference to the unique workflow identifier.
                */
                const std::string& get_identifier() const {return identifier_;}
                /**
                * @brief Return a constant reference to qubit indices to be measured.
                */
                const std::vector<size_t>& get_qubits() const {return qubits_;}
                /**
                * @brief Return a constant reference to wrapped workflow.
                */
                const EXECWORKFLOW& get_wrapped_workflow() const {return workflow_;}
                /**
                * @brief Return a reference to wrapped workflow.
                */
                EXECWORKFLOW& set_wrapped_workflow() {return workflow_;}

            private:
                /**
                * @brief Given a bit string and a measurement basis string, evaluate the sign value with which the bit string contributes to the expectation value of the measurement basis string.
                * 
                * Arguments: 
                * @param bitstring the measured bit string given as std::string, 
                * @param basis the measurement basis string given as a std::vector of basis symbols.
                * 
                * @returns int the calculated sign value.
                */
                int evaluate_sign(const std::string& bitstring, const std::vector<SYMBOL>& basis) const {
                    int sign = 1; 
                    for (const auto& [bit, base] : std::ranges::views::zip(bitstring, basis)) {
                        if (bit == '1' && base != get_identity<SYMBOL>()) { 
                            sign *= -1; //if 1 was measured and basis was not the identity, multiply -1
                        }
                    }
                    return sign;
                }

                EXECWORKFLOW& workflow_;
                const std::string identifier_;
                std::vector<size_t> qubits_;
                const std::vector<SYMBOL> basis_;
                const SYMBOL use_for_identity_;
        };

        /**
        * @brief Fully specialized execute functor for the Task::MeasureCounts task of the templated QuantumStateTomography workflow.
        */
        template <ExecutableWorkflow EXECWORKFLOW, typename SYMBOL>
        class executeWorkflowTask<QuantumStateTomography<EXECWORKFLOW, SYMBOL>, Task::MeasureCounts> {
            public: 
                /**
                * @brief Specialized member function generating and serializing the measured bit string counts of the QuantumStateTomography workflow.
                * 
                * Arguments: 
                * @param workflow A templated QuantumStateTomography reference 
                * @param timestamp The time stamp of execution. 
                * 
                * @return ---
                * 
                * @details This member function will iterate over all wrapped workflow circuits, append basis rotation gates, run the circuits using 
                * the workflow's qb::session object, and serialize them.
                */
                void operator()(QuantumStateTomography<EXECWORKFLOW, SYMBOL>& workflow, std::time_t timestamp) const {
                    qb::String measured_results;
                    for (qb::CircuitBuilder& circuit: workflow.get_wrapped_workflow().get_circuits()) { //for each workflow circuit 
                        for (qb::CircuitBuilder& qst_circuit : workflow.append_measurement_bases(circuit)) { //for each appended basis measurement
                            //add measurements 
                            for (auto const & qubit : workflow.get_qubits())
                                qst_circuit.Measure(qubit);
                            //add target to session and push results 
                            workflow.get_wrapped_workflow().set_session().set_irtarget_m(qst_circuit.get());
                            workflow.get_wrapped_workflow().set_session().run();
                            measured_results.push_back(workflow.get_wrapped_workflow().get_session().get_out_raws()[0][0]);
                        }
                    }
                    workflow.serialize_measured_counts(measured_results, timestamp);
                }
        };
        /**
        * @brief Fully specialized execute functor for the Task::IdealDensity task of the templated QuantumStateTomography workflow.
        */
        template <ExecutableWorkflow EXECWORKFLOW, typename SYMBOL>
        class executeWorkflowTask<QuantumStateTomography<EXECWORKFLOW, SYMBOL>, Task::IdealDensity> {
            public: 
                /**
                * @brief Specialized member function generating and serializing the ideal quantum state densities of the workflow wrapped within the QuantumStateTomography object.
                * 
                * Arguments: 
                * @param workflow a templated QuantumStateTomography reference 
                * @param timestamp the time stamp of execution. 
                * 
                * @return ---
                * 
                * @details This member function will delegate the execute call to the wrapped workflow within the QuantumStateTomography object to generate ideal quantum state 
                * densities. To enable the DataLoaderGenerator to find the serialized data, a symbolic link with the unique QuantumStateTomography identifier will be created.
                */
                void operator()(QuantumStateTomography<EXECWORKFLOW, SYMBOL>& workflow, std::time_t timestamp) const {
                    //call wrapped workflow to execute just the ideal densities
                    std::time_t t2 = workflow.set_wrapped_workflow().execute(std::vector<Task>{Task::IdealDensity});
                    //beware! This will serialize them with the wrapped workflow's identifier 
                    //therefore, create a symbolic link to allow DataLoaderGenerator to find the correct file
                    std::stringstream link, target; 
                    link << "intermediate_benchmark_results/" << workflow.get_identifier() << "_densities_" << timestamp << ".bin"; 
                    target << workflow.get_wrapped_workflow().get_identifier() << "_densities_" << t2 << ".bin";
                    std::filesystem::create_symlink(target.str(), link.str());
                }
        };

    } // namespace qb::benchmark
} // namespace qb

#endif