// Copyright (c) Quantum Brilliance Pty Ltd

#pragma once

#include <string>
#include <vector>
#include <filesystem>
#include <map>
#include <sstream>

#include <qristal/core/benchmark/workflows/QuantumStateTomography.hpp>

namespace qristal
{
    namespace benchmark
    {
        /**
        * @brief Calculate the Hilbert-Schmidt inner product of two given complex matrices.
        *
        * Arguments:
        * @param a the left complex Eigen::Matrix passed as a constant reference
        * @param b the right complex Eigen::Matrix passed as a constant reference.
        *
        * @return std::complex<double> the inner product, i.e., tr(a' * b).
        */
        inline std::complex<double> HilbertSchmidtInnerProduct(const Eigen::Matrix<std::complex<double>, Eigen::Dynamic, Eigen::Dynamic>& a, const Eigen::Matrix<std::complex<double>, Eigen::Dynamic, Eigen::Dynamic>& b) {
            return (a.adjoint() * b).trace();
        }

        /**
        * @brief Pure virtual python bindings helper class not used in the C++ implementation.
        */
        class QuantumProcessTomographyPythonBase {
            public:
                virtual ~QuantumProcessTomographyPythonBase() = default;
                virtual std::time_t execute(const std::vector<Task>& tasks) = 0;
                virtual std::time_t execute_all() = 0;
                virtual const std::string& get_identifier() const = 0;
                virtual std::vector<ComplexMatrix> assemble_processes(const std::vector<ComplexMatrix>& densities) = 0;
        };

        /**
        * @brief Standard quantum process tomography workflow templated for arbitrary wrapped quantum state tomography workflows and input state bases.
        *
        * @details This workflow class may be used to execute standard quantum process tomography experiments. It is templated and wrapped around arbitrary quantum state tomography workflows @tparam QSTWorkflow and
        * input state bases @tparam StateSymbol. Compatible input state bases need to be (i) matrix translatable and (ii) circuit appendable. The workflow may be used in metric evaluations that require measured bit
        * string counts and ideal quantum process matrices if provided by the wrapped workflow. Beware that producing ideal process matrices is not a requirement of the quantum process tomography workflow
        * (this workflow) but possibly by a consecutively calculated metric like, e.g., the quantum process fidelity.
        */
        template <QSTWorkflow QSTWORKFLOW, typename StateSymbol = BlochSphereUnitState>
        requires MatrixTranslatable<StateSymbol> && CircuitAppendable<StateSymbol>
        class QuantumProcessTomography : public virtual QuantumProcessTomographyPythonBase
        {
            public:

                using QSTWorkflowType = QSTWORKFLOW; //expose wrapped QST type

                /**
                * @brief Constructor for standard quantum process tomography workflows on a specific set of qubits.
                *
                * Arguments:
                * @param quantum state tomography workflow @tparam QSTWORKFLOW the quantum process tomography is acted upon.
                * @param states list of the input one qubit state symbols, defaults to BlochSphereUnitStates Z+, Z-, X+, and Y-.
                *
                * @return ---
                */
                QuantumProcessTomography(
                    QSTWORKFLOW& qstworkflow,
                    const std::vector<StateSymbol>& states = std::vector<BlochSphereUnitState>{BlochSphereUnitState::Symbol::Zp, BlochSphereUnitState::Symbol::Zm, BlochSphereUnitState::Symbol::Xp, BlochSphereUnitState::Symbol::Ym}
                ) :
                    qstworkflow_(qstworkflow),
                    identifier_(std::string("QPT") + qstworkflow.get_identifier()),
                    states_(states)
                {}

                /**
                * @brief Prepend a given quantum circuit by all state initialization circuits required for the standard QPT workflow.
                *
                * Arguments:
                * @param workflow_circuit reference to quantum circuit to be prepended.
                *
                * @return std::vector<qristal::CircuitBuilder> a std::vector of all prepended circuits.
                */
                std::vector<qristal::CircuitBuilder> prepend_state_initializations(qristal::CircuitBuilder & workflow_circuit) const {
                    std::vector<qristal::CircuitBuilder> circuits;
                    size_t n_qubits = qstworkflow_.get_qubits().size();
                    size_t n_states = states_.size();
                    size_t n_full_n_qubit_states = std::pow(n_states, n_qubits);
                    for (size_t n_qubit_state_index = 0; n_qubit_state_index < n_full_n_qubit_states; ++n_qubit_state_index) {
                        std::vector<size_t> indices = convert_decimal(n_qubit_state_index, n_states, n_qubits); //convert to x-nary number
                        qristal::CircuitBuilder cb;
                        //add initial state
                        for (const auto& [xnary_index, qubit_index] : ::ranges::views::zip(indices, qstworkflow_.get_qubits())) {
                            states_[xnary_index].append_circuit(cb, qubit_index);
                        }
                        //add workflow_circuit
                        cb.append(workflow_circuit);
                        circuits.push_back(cb);
                    }
                    return circuits;
                }

                /**
                * @brief Run quantum process tomography workflow and store results for specific tasks.
                *
                * Arguments:
                * @param tasks a selection of Tasks to be executed using the initialized quantum process tomography workflow.
                *
                * @return std::time_t the time stamp of the successful execution.
                *
                * @details This member function is used to execute specific tasks the quantum process tomography workflow is capable of. These include storing
                * (i) the measured bit string counts of the wrapped standard quantum state tomography protocol of the wrapped (input state prepended) workflow circuits,
                * (ii) the ideal quantum process matrices for each quantum circuit of the doubly wrapped workflow inside the templated quantum state tomography workflow,
                * (iii) the relevant information contained in the passed qristal::session.
                * Beware that an actual circuit execution is only triggered for task (i). Task (ii) will delegate IdealProcess tasks to the doubly wrapped ExecutableWorkflow.
                */
                std::time_t execute(const std::vector<Task>& tasks)
                {
                    return executeWorkflowTasks<QuantumProcessTomography<QSTWORKFLOW, StateSymbol>>(*this, tasks);
                }
                /**
                * @brief Run quantum process tomography workflow and store results for all possible tasks.
                *
                * Arguments:
                * @param tasks a selection of Tasks to be executed using the initialized quantum process tomography workflow.
                *
                * @return std::time_t the time stamp of the successful execution.
                *
                * @details This member function is used to execute all available tasks the quantum process tomography workflow is capable of. These include storing
                * (i) the measured bit string counts of a standard quantum state tomography protocol of the wrapped (input state prepended) workflow circuits,
                * (ii) the ideal quantum process matrices for each quantum circuit of the doubly wrapped workflow inside the templated quantum state tomography workflow,
                * (iii) the relevant information contained in the passed qristal::session.
                * Beware that an actual circuit execution is only triggered for task (i). Task (ii) will delegate IdealProcess tasks to the doubly wrapped ExecutableWorkflow.
                */
                std::time_t execute_all()
                {
                    std::time_t t = execute(std::vector<Task>{Task::MeasureCounts, Task::IdealProcess, Task::Session});
                    return t;
                }

                /**
                * @brief Calculate process matrices from measured quantum state densities of the quantum process
                * tomography workflow by the standard quantum state tomography protocol.
                *
                * Arguments:
                * @param densities the measured densities obtained via the wrapped quantum state tomography object of this class.
                *
                * @return std::vector<ComplexMatrix> the calculated process matrices.
                *
                * @details This member function executes the standard quantum process tomography protocol to calculate
                * quantum process matrices. The essential steps include
                * (i) Calculating the inverse overlap matrix of the input state basis (to take care of non-orthogonal projections)
                * (ii) Calculating the inverse B matrix with elements B(mn, ij) = <rho_j | Em rho_i En> for input state densities rho and measurement basis unitaries E
                * (iii) Projecting (non-orthogonal!) the n-qubit input state densities onto the measured densities to calculate lambda_kl = <rho_l | eps(rho_k) > for measured density eps(rho_k) for input state rho_k
                * (iv) Obtaining the final process matrices via reshaping B^-1 * lambda.
                */
                std::vector<ComplexMatrix> assemble_processes(const std::vector<ComplexMatrix>& densities)
                {
                    std::vector<ComplexMatrix> processes;
                    size_t n_qubits = qstworkflow_.get_qubits().size();
                    size_t full_n_qubit_state_size = std::pow(states_.size(), n_qubits);

                    //(1) Calculate overlap matrix and invert ({Zp, Zm, Xp, Y-} is non-orthogonal w.r.t. Hilbert-Schmidt inner product!) if not already computed!
                    if (invS_.rows() == 0) {
                        invS_ = Eigen::Matrix<std::complex<double>, Eigen::Dynamic, Eigen::Dynamic>::Zero(full_n_qubit_state_size, full_n_qubit_state_size);
                        for (size_t i = 0; i < full_n_qubit_state_size; ++i) //there are (#input states)^n_qubits combinations!
                        {
                            auto bra = build_up_matrix_by_Kronecker_product<StateSymbol>(i, states_, n_qubits);
                            //only calculate upper triangle of overlap matrix!
                            for (size_t j = i; j < full_n_qubit_state_size; ++j) {
                                auto ket = build_up_matrix_by_Kronecker_product<StateSymbol>(j, states_, n_qubits);
                                //calculate inner product and assign S
                                invS_(i,j) = HilbertSchmidtInnerProduct(bra, ket);
                                invS_(j,i) = invS_(i,j);
                            }
                        }
                        //finally invert matrix
                        invS_ = invS_.inverse();
                    }

                    //(2) Calculate B matrix (2^(4N) x 2^(4N)) with B(mn, ij) = <rho_j | Em rho_i En> for densities rho_i, rho_j, and measurement operators Em, En (these are already generated in QST object)
                    //    But beware of non-orthogonality if not already computed
                    if (invB_.rows() == 0) {
                        auto basis_with_identity = std::vector<typename QSTWORKFLOW::Symbol>{get_identity<typename QSTWORKFLOW::Symbol>()};
                        for (const auto& i : qstworkflow_.get_basis()) {
                            basis_with_identity.push_back(i);
                        }
                        size_t full_n_qubit_basis_size = std::pow(basis_with_identity.size(), n_qubits);
                        invB_ = Eigen::Matrix<std::complex<double>, Eigen::Dynamic, Eigen::Dynamic>::Zero(std::pow(2, 4 * n_qubits), std::pow(2, 4 * n_qubits));
                        for (size_t m = 0; m < full_n_qubit_basis_size; ++m) {
                            auto Em = build_up_matrix_by_Kronecker_product<typename QSTWORKFLOW::Symbol>(m, basis_with_identity, n_qubits);
                            for (size_t n = 0; n < full_n_qubit_basis_size; ++n) {
                                size_t mn = m*full_n_qubit_basis_size + n;
                                auto En = build_up_matrix_by_Kronecker_product<typename QSTWORKFLOW::Symbol>(n, basis_with_identity, n_qubits);
                                for (size_t i = 0; i < full_n_qubit_state_size; ++i) {
                                    auto rho_i = build_up_matrix_by_Kronecker_product<StateSymbol>(i, states_, n_qubits);
                                    Eigen::Matrix<std::complex<double>, Eigen::Dynamic, Eigen::Dynamic> temp_j = Eigen::Matrix<std::complex<double>, Eigen::Dynamic, Eigen::Dynamic>::Zero(full_n_qubit_state_size, 1);
                                    for (size_t j = 0; j < full_n_qubit_state_size; ++j) {
                                        auto rho_j = build_up_matrix_by_Kronecker_product<StateSymbol>(j, states_, n_qubits);
                                        //set element in temp_j
                                        temp_j(j, 0) = HilbertSchmidtInnerProduct(rho_j, Em * rho_i * En.adjoint());
                                    }
                                    //do not forget about non-orthogonal projection!
                                    temp_j  = invS_ * temp_j;
                                    //finally set elements in B matrix:
                                    for (size_t j = 0; j < temp_j.rows(); ++j) {
                                        size_t ij = i*full_n_qubit_state_size + j;
                                        invB_(ij, mn) = temp_j(j, 0);
                                    }
                                }
                            }
                        }
                        //finally invert matrix
                        invB_ = invB_.inverse();
                    }

                    //each superoperator requires 4^nqubits densities
                    for (size_t experiment = 0; experiment < densities.size(); experiment += full_n_qubit_state_size) {
                        //(3) Calculate lambda vector with lambda_kl = <rho_l | eps(rho_k) > for measured density eps(rho_k)
                        Eigen::Matrix<std::complex<double>, Eigen::Dynamic, Eigen::Dynamic> lambda = Eigen::Matrix<std::complex<double>, Eigen::Dynamic, Eigen::Dynamic>::Zero(std::pow(states_.size(), 2*n_qubits), 1);
                        for (size_t k = 0; k < full_n_qubit_state_size; ++k) {
                            Eigen::Matrix<std::complex<double>, Eigen::Dynamic, Eigen::Dynamic> temp_lambda = Eigen::Matrix<std::complex<double>, Eigen::Dynamic, Eigen::Dynamic>::Zero(full_n_qubit_state_size, 1);
                            for (size_t l = 0; l < full_n_qubit_state_size; ++l) {
                                auto rho_l = build_up_matrix_by_Kronecker_product<StateSymbol>(l, states_, n_qubits);
                                //project onto measured density and set temp_lambda
                                temp_lambda(l, 0) = HilbertSchmidtInnerProduct(rho_l, densities[k + experiment]);
                            }
                            //don't forget about the non-orthogonal projection!
                            temp_lambda = invS_ * temp_lambda;
                            //finally set elements in lambda
                            for (size_t l = 0; l < temp_lambda.rows(); ++l) {
                                size_t kl = k * full_n_qubit_state_size + l;
                                lambda(kl, 0) = temp_lambda(l, 0);
                            }
                        }

                        //(4) Obtain quantum process from applying B^-1 and reshaping
                        ComplexMatrix result = invB_ * lambda;
                        result.resize(full_n_qubit_state_size, full_n_qubit_state_size);
                        processes.push_back(result);
                    }

                    return processes;
                }

                /**
                * @brief Return a constant reference to the quantum state tomography workflow wrapped in the quantum process tomography workflow.
                */
                const QSTWORKFLOW& get_qst() const {return qstworkflow_;}
                /**
                * @brief Return a reference to the quantum state tomography object used in the quantum process tomography workflow.
                */
                QSTWORKFLOW& set_qst() {return qstworkflow_;}
                /**
                * @brief Return a constant reference to the unique workflow identifier.
                */
                const std::string& get_identifier() const {return identifier_;}
                /**
                * @brief Serialization method for measured bit string counts
                *
                * Arguments:
                * @param counts the measured bit string counts returned by qristal::session
                * @param time the time stamp of execution
                *
                * @return ---
                */
                void serialize_measured_counts( const std::vector<std::map<std::vector<bool>, int>>& counts, const std::time_t time ) const {
                    save_data<BitCounts, std::vector<std::map<std::vector<bool>, int>>>(identifier_, "_measured_", counts, time);
                }
                /**
                * @brief Serialization method for the assigned qristal::session
                *
                * Arguments:
                * @param time the time stamp of execution
                *
                * @return ---
                */
                void serialize_session_infos(const std::time_t time) const {
                    save_data<SessionInfo, SessionInfo>(identifier_, "_session_" , qstworkflow_.get_wrapped_workflow().get_session(), time);
                }

            private:

                QSTWORKFLOW& qstworkflow_;
                const std::string identifier_;
                std::vector<StateSymbol> states_;

                Eigen::Matrix<std::complex<double>, Eigen::Dynamic, Eigen::Dynamic> invS_, invB_;
        };

        /**
        * @brief The type-erased QuantumProcessTomography handle exposed in the python bindings.
        */
        class QuantumProcessTomographyPython {
            public:
                //Due to the doubly nested template, it is necessary to implement a new constructor which takes in the python exposed type QuantumStateTomographyPython
                //This requires a runtime check (via dynamic_cast) for compatible workflows, to construct the right QuantumProcessTomography objects.
                //To not pollute the standalone header, this was moved to a cpp file.
                QuantumProcessTomographyPython(
                    QuantumStateTomographyPython& qstpython,
                    const std::vector<BlochSphereUnitState>& states = std::vector<BlochSphereUnitState>{BlochSphereUnitState::Symbol::Zp, BlochSphereUnitState::Symbol::Zm, BlochSphereUnitState::Symbol::Xp, BlochSphereUnitState::Symbol::Ym}
                );

                std::time_t execute(const std::vector<Task>& tasks) {
                    return workflow_ptr_->execute(tasks);
                }

                std::time_t execute_all() {
                    return workflow_ptr_->execute_all();
                }

                std::vector<ComplexMatrix> assemble_processes(const std::vector<ComplexMatrix>& densities) {
                    return workflow_ptr_->assemble_processes(densities);
                }

                const std::string& get_identifier() const {
                    return workflow_ptr_->get_identifier();
                }

                const std::unique_ptr<QuantumProcessTomographyPythonBase>& get() const {
                  return workflow_ptr_;
                }

            private:
                std::unique_ptr<QuantumProcessTomographyPythonBase> workflow_ptr_;
        };


        /**
        * @brief Fully specialized execute functor for the Task::MeasureCounts task of the templated QuantumProcessTomography workflow.
        */
        template <QSTWorkflow QSTWORKFLOW, typename StateSymbol>
        class executeWorkflowTask<QuantumProcessTomography<QSTWORKFLOW, StateSymbol>, Task::MeasureCounts> {
            public:
                /**
                * @brief Specialized member function generating and serializing the measured bit string counts of the QuantumProcessTomography workflow.
                *
                * Arguments:
                * @param workflow A templated QuantumProcessTomography reference
                * @param timestamp The time stamp of execution.
                *
                * @return ---
                *
                * @details This member function will iterate over all underlying workflow circuits, prepend all required initial state preparation gates, and append
                * all required basis rotation gates. For each circuit, the workflow's session object is used to generate measured bit string results. These are collected
                * in a std::vector of std::string objects and then serialized.
                */
                void operator()(QuantumProcessTomography<QSTWORKFLOW, StateSymbol>& qpt, std::time_t timestamp) const {
                    std::vector<std::map<std::vector<bool>, int>> measured_results;
                    auto workflow_circuits = qpt.get_qst().get_wrapped_workflow().get_circuits();
                    for (auto & w : workflow_circuits) {
                        for (auto& iw : qpt.prepend_state_initializations(w)) {
                            for (auto& iwb : qpt.get_qst().append_measurement_bases(iw)) {
                                //add measurements
                                for (auto const & qubit : qpt.get_qst().get_qubits()) {
                                    iwb.Measure(qubit);
                                }
                                //add target to session and push results
                                qpt.get_qst().get_wrapped_workflow().set_session().irtarget = iwb.get();
                                qpt.get_qst().get_wrapped_workflow().set_session().run();
                                measured_results.push_back(qpt.get_qst().get_wrapped_workflow().get_session().results());
                            }
                        }
                    }
                    qpt.serialize_measured_counts(measured_results, timestamp);
                }
        };

        /**
        * @brief Fully specialized execute functor for the Task::IdealProcess task of the templated QuantumProcessTomography workflow.
        */
        template <QSTWorkflow QSTWORKFLOW, typename StateSymbol>
        class executeWorkflowTask<QuantumProcessTomography<QSTWORKFLOW, StateSymbol>, Task::IdealProcess> {
            public:
                /**
                * @brief Specialized member function generating and serializing the ideal quantum process matrices of the workflow wrapped within the QuantumProcessTomography object.
                *
                * Arguments:
                * @param workflow A templated QuantumProcessTomography reference
                * @param timestamp The time stamp of execution.
                *
                * @return ---
                *
                * @details This member function will delegate the execute call to the doubly wrapped workflow within the QuantumProcessTomography object to generate ideal quantum process
                * matrices. To enable the DataLoaderGenerator to find the serialized data, a symbolic link with the unique QuantumProcessTomography identifier will be created.
                */
                void operator()(QuantumProcessTomography<QSTWORKFLOW, StateSymbol>& workflow, std::time_t timestamp) const {
                    //call wrapped workflow to execute just the ideal processes
                    std::time_t t2 = workflow.set_qst().set_wrapped_workflow().execute(std::vector<Task>{Task::IdealProcess});
                    //beware! This will serialize them with the wrapped workflow's identifier
                    //therefore, create a symbolic link to allow DataLoaderGenerator to find the correct file
                    std::stringstream link, target;
                    link << "intermediate_benchmark_results/" << workflow.get_identifier() << "_processes_" << timestamp << ".bin";
                    target << workflow.get_qst().get_wrapped_workflow().get_identifier() << "_processes_" << t2 << ".bin";
                    std::filesystem::create_symlink(target.str(), link.str());
                }
        };


    }
}
