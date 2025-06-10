// Copyright (c) Quantum Brilliance Pty Ltd

#pragma once

// Qristal
#include <qristal/core/benchmark/Serializer.hpp> // contains <qristal/core/session.hpp> & typedefs
#include <qristal/core/benchmark/Concepts.hpp>
#include <qristal/core/primitives.hpp>

// STL
#include <string>
#include <vector>
#include <filesystem>
#include <map>
#include <ranges>

// range v3
#include <range/v3/view/zip.hpp>


namespace qristal
{
    namespace benchmark
    {
        /**
        * @brief Pure virtual python bindings helper class not used in the C++ implementation.
        */
        class QuantumStateTomographyPythonBase {
            public:
                virtual ~QuantumStateTomographyPythonBase() = default;
                virtual std::time_t execute(const std::vector<Task>& tasks) = 0;
                virtual std::time_t execute_all() = 0;
                virtual const std::vector<Pauli>& get_basis() const = 0;
                virtual const std::string& get_identifier() const = 0;
                virtual const std::set<size_t>& get_qubits() const = 0;
                virtual void set_maximum_likelihood_estimation(
                    const size_t n_MLE_iterations,
                    const double MLE_conv_threshold,
                    const std::map<Pauli, std::vector<ComplexMatrix>>& mBasisSymbols_to_Projectors
                ) = 0;
                virtual std::vector<ComplexMatrix> assemble_densities(
                    const std::vector<std::map<std::vector<bool>, int>>& measurement_counts
                ) const = 0;
        };

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
        class QuantumStateTomography : public virtual QuantumStateTomographyPythonBase
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
                * @param perform_maximum_likelihood_estimation : optional bool to enable MLE in the QST protocol. Defaults to false.
                * @param basis list of the measured one qubit basis symbols (excluding the identity). Defaults to Pauli X, Y, and Z.
                * @param use_for_identity the basis symbol used to resolve the identity in the QST protocol. Defaults to Pauli::Z.
                *
                * @return ---
                */
                QuantumStateTomography(
                    EXECWORKFLOW& workflow,
                    const std::set<size_t>& qubits,
                    const bool perform_maximum_likelihood_estimation = false,
                    const std::vector<SYMBOL>& basis = std::vector<Pauli>{Pauli::Symbol::X, Pauli::Symbol::Y, Pauli::Symbol::Z},
                    const SYMBOL& use_for_identity = Pauli::Symbol::Z
                ) : workflow_(workflow),
                    identifier_(std::string("QST") + workflow_.get_identifier()),
                    qubits_(qubits),
                    perform_maximum_likelihood_estimation_(perform_maximum_likelihood_estimation),
                    basis_(basis),
                    use_for_identity_(use_for_identity)
                {
                  if (perform_maximum_likelihood_estimation_) {
                      set_maximum_likelihood_estimation(); //set default options
                  }
                }

                /**
                * @brief Constructor for standard quantum state tomography workflows for all involved qubits.
                *
                * Arguments:
                * @param workflow the wrapped @tparam ExecutableWorkflow the quantum state tomography is acted upon.
                * @param perform_maximum_likelihood_estimation : optional bool to enable MLE in the QST protocol. Defaults to false.
                * @param basis list of the measured one qubit basis symbols (excluding the identity), defaults to Pauli X, Y, and Z.
                * @param use_for_identity the basis symbol used to resolve the identity in the QST protocol. Defaults to Pauli::Z.
                *
                * @return ---
                */
                QuantumStateTomography(
                    EXECWORKFLOW& workflow,
                    const bool perform_maximum_likelihood_estimation = false,
                    const std::vector<SYMBOL>& basis = std::vector<Pauli>{Pauli::Symbol::X, Pauli::Symbol::Y, Pauli::Symbol::Z},
                    const SYMBOL& use_for_identity = Pauli::Symbol::Z
                ) : workflow_(workflow),
                    identifier_(std::string("QST") + workflow_.get_identifier()),
                    perform_maximum_likelihood_estimation_(perform_maximum_likelihood_estimation),
                    basis_(basis),
                    use_for_identity_(use_for_identity)
                {
                    for (size_t i = 0; i < workflow_.get_session().qn; ++i) {
                        qubits_.insert(i);
                    }
                    if (perform_maximum_likelihood_estimation_) {
                        set_maximum_likelihood_estimation(); //set default options
                    }
                }

                /**
                * @brief Change default options for maximum likelihood estimation (MLE) in the assemble_density function.
                *
                * Arguments:
                * @param n_MLE_iterations the maximum number of iterations used in the iterative MLE procedure, defaults to 100.
                * @param MLE_conv_threshold the convergence threshold, defaults to 1e-3.
                * @param mBasisSymbols_to_Projectors a std::map from the used basis symbols in the QST protocoll to their corresponding complex-valued 2x2 projector matrices for
                * bit results 0 and 1 stored in a std::vector<ComplexMatrix>. By default, the mapping from the standard Pauli basis to Bloch sphere unit states, i.e., the Clifford states, are used.
                *
                * @return ---
                */
                void set_maximum_likelihood_estimation(
                    const size_t n_MLE_iterations = 100,
                    const double MLE_conv_threshold = 1e-3,
                    const std::map<SYMBOL, std::vector<ComplexMatrix>>& mBasisSymbols_to_Projectors = std::map<Pauli, std::vector<ComplexMatrix>>{
                        {Pauli::Symbol::X, std::vector<ComplexMatrix>{BlochSphereUnitState(BlochSphereUnitState::Symbol::Xp).get_matrix(), BlochSphereUnitState(BlochSphereUnitState::Symbol::Xm).get_matrix()}},
                        {Pauli::Symbol::Y, std::vector<ComplexMatrix>{BlochSphereUnitState(BlochSphereUnitState::Symbol::Yp).get_matrix(), BlochSphereUnitState(BlochSphereUnitState::Symbol::Ym).get_matrix()}},
                        {Pauli::Symbol::Z, std::vector<ComplexMatrix>{BlochSphereUnitState(BlochSphereUnitState::Symbol::Zp).get_matrix(), BlochSphereUnitState(BlochSphereUnitState::Symbol::Zm).get_matrix()}}
                    }
                )
                {
                    mBasisSymbols_to_Projectors_ = mBasisSymbols_to_Projectors;
                    n_MLE_iterations_ = n_MLE_iterations;
                    MLE_conv_threshold_ = MLE_conv_threshold;
                }

                std::vector<qristal::CircuitBuilder> append_measurement_bases(qristal::CircuitBuilder & workflow_circuit) const {
                    std::vector<qristal::CircuitBuilder> circuits;
                    size_t n_qubit_basis_states = std::pow(basis_.size(), qubits_.size());
                    for (size_t basis_index = 0; basis_index < n_qubit_basis_states; ++basis_index) //there are (#basis states)^n_qubits combinations!
                    {
                        //create empty circuit and append workflow
                        qristal::CircuitBuilder cb;
                        cb.append(workflow_circuit);
                        //convert i to x-nary number of length qubits.size() to find out which basis rotation to apply on which qubit
                        std::vector<size_t> indices = convert_decimal(basis_index, basis_.size(), qubits_.size());
                        for (const auto& [xnary_basis_index, qubit_index] : ::ranges::views::zip(indices, qubits_)) {
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
                * (iii) the relevant information contained in the passed qristal::session.
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
                * (iii) the relevant information contained in the passed qristal::session.
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
                * @param measurement_counts the measured bit string counts as serialized by the execute function. Contains n * 3^q bit string histograms for n wrapped workflow
                * circuits and q measured qubits.
                *
                * @return std::vector<ComplexMatrix> the quantum state density matrices obtained through the quantum state tomography experiments.
                *
                * @details This member function will iterate over all sets of 3^q (for q measured qubits) measured bit string counts and calculate one complex density matrix
                * for each set. If the member function `set_maximum_likelihood_estimation` was called, it will
                * (i) iterate over all measured bases and each measured bitstring and assemble measured frequencies f_j as well as corresponding projectors E_j,
                * (ii) initialize a density matrix to rho_1 = 1/2^q * I (for q qubits), and
                * (iii) iteratively update rho through rho_k+1 = R(rho_k) * rho_k * R(rho_k), where R(rho_k) = sum_j f_j / Tr(E_j*rho_k) * E_j until convergence or the maximum
                * number of iterations was reached. This technique will guarantee that the assembled density matrices are unit-traced, hermitian, and positive semi-definite.
                * In case no maximum likelihood estimation was set, this function will perform a standard QST protocoll by
                * (i) reconstructing the measurement basis that was used for a given set of measured bit string counts,
                * (ii) augmenting the original measurement basis to the accessible basis strings by resolving all identities with the chosen symbol,
                * (iii) evaluating the measured expectation values for each basis string
                * (iv) adding the corresponding contribution to the individual (zero initialized) complex density matrices.
                * Beware! The latter will guarantee that the assembled density are only unit-traced and hermitian!
                */
                std::vector<ComplexMatrix> assemble_densities(const std::vector<std::map<std::vector<bool>, int>>& measurement_counts) const
                {
                    std::vector<ComplexMatrix> densities;
                    size_t density_dimension = std::pow(2, qubits_.size());
                    size_t n_qubit_basis_size = std::pow(basis_.size(), qubits_.size());
                    size_t task_step = std::pow(3, qubits_.size());
                    for (size_t task = 0; task < measurement_counts.size(); task += task_step) { //create one density matrix for each task aka workflow circuit

                        //(1) initialize empty density matrix
                        ComplexMatrix density = ComplexMatrix::Zero(density_dimension, density_dimension);

                        //(2) either obtain density through iterative MLE (this ensures hermiticity, positive semi-definiteness, and unit-trace, cf. https://arxiv.org/abs/quant-ph/0311097)
                        if (perform_maximum_likelihood_estimation_) {
                            //(2.1) initialize density to identity matrix
                            for (size_t i = 0; i < density_dimension; ++i) {
                                density(i,i) = 1.0 / static_cast<double>(density_dimension);
                            }

                            //(2.2) iterate over all measurement bases and obtain projectors and measured frequencies
                            std::vector<ComplexMatrix> projections;
                            std::vector<double> measured_frequencies;
                            for (size_t measurement = 0; measurement < n_qubit_basis_size; ++measurement) {
                                //(2.2.1) extract corresponding symbols vector (i.e., the Pauli basis)
                                std::vector<SYMBOL> basis;
                                for (auto const & i : convert_decimal(measurement, basis_.size(), qubits_.size())) {
                                    basis.push_back(basis_[i]);
                                }

                                //(2.2.2) Now iterate over all measured counts
                                const std::map<std::vector<bool>, int>& counts = measurement_counts.at(task + measurement);
                                const size_t n_shots = sumMapValues(counts);
                                for (auto const & [bitstring, count] : counts) {
                                    //store the measured frequencies
                                    measured_frequencies.push_back(static_cast<double>(count) / static_cast<double>(n_shots));

                                    //and evaluate the corresponding projector (in reverse order!)
                                    ComplexMatrix proj = ComplexMatrix::Ones(1,1);
                                    for (int i = bitstring.size() - 1; i >= 0; --i) {
                                        proj = Eigen::kroneckerProduct(proj, mBasisSymbols_to_Projectors_.at(basis[i])[bitstring[i]]).eval();
                                    }
                                    projections.push_back(proj);
                                }
                            }

                            //(2.3) finally assemble the density matrix through iterative transform
                            for (size_t iter = 1; iter <= n_MLE_iterations_; ++iter) {
                                //(2.3.1) first build up full transform operator from measured frequencies and evaluated expectation values of the current density
                                ComplexMatrix R = ComplexMatrix::Zero(density_dimension, density_dimension);
                                for (auto const & [projection, frequency] : ::ranges::views::zip(projections, measured_frequencies)) {
                                    double p = (projection.adjoint() * density).trace().real();
                                    R += frequency / p * projection;
                                }
                                //(2.3.2) then transform density
                                ComplexMatrix new_density = R*density*R;
                                new_density *= 1.0 / new_density.trace();
                                //(2.3.3) check for convergence
                                if (new_density.isApprox(density, MLE_conv_threshold_)) {
                                    density = new_density;
                                    break;
                                }
                                density = new_density;
                            }
                        }
                        //(2) or through standard QST protocol (linear inversion ensuring only hermiticity and unit-trace)
                        else {
                            for (size_t measurement = 0; measurement < n_qubit_basis_size; ++measurement) { //Each circuit was measured 3^n times
                                const std::map<std::vector<bool>, int>& counts = measurement_counts.at(task + measurement); //extract relevant counts map
                                const size_t n_shots = sumMapValues(counts);
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
                                std::vector<double> temp_exp_values(accessible_bases.size(), 0.0); //initialize zero expectation values for each basis
                                for (const auto& [bitstring, count] : counts) {
                                    for (const auto& [exp_value, accessible_base] : ::ranges::views::zip(temp_exp_values, accessible_bases)) {
                                        //evaluate sign with which the measured bitstring contributes to all basis expectation values
                                        int sign = evaluate_sign(bitstring, accessible_base);
                                        exp_value += static_cast<double>(sign) * static_cast<double>(count) / static_cast<double>(n_shots);
                                    }
                                }

                                //and finally update density matrix
                                for (const auto & [exp_value, basis] : ::ranges::views::zip(temp_exp_values, accessible_bases)) {
                                    density += exp_value * calculate_Kronecker_product(basis);
                                }
                            }
                            density *= 1.0 / pow(2, static_cast<double>(qubits_.size())); //normalization
                        }

                        densities.push_back(density); //push normalized density to collection
                    }

                    return densities;
                }

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
                const std::set<size_t>& get_qubits() const {return qubits_;}
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
                * @param bitstring the measured bit string given as std::vector<bool>,
                * @param basis the measurement basis string given as a std::vector of basis symbols.
                *
                * @returns int the calculated sign value.
                */
                int evaluate_sign(const std::vector<bool>& bitstring, const std::vector<SYMBOL>& basis) const {
                    int sign = 1;
                    for (const auto& [bit, base] : ::ranges::views::zip(bitstring, basis)) {
                        if (bit && base != get_identity<SYMBOL>()) {
                            sign *= -1; //if 1 was measured and basis was not the identity, multiply -1
                        }
                    }
                    return sign;
                }

                EXECWORKFLOW& workflow_;
                const std::string identifier_;
                std::set<size_t> qubits_;
                const std::vector<SYMBOL> basis_;
                const SYMBOL use_for_identity_;

                //used for maximum likelihood estimation only
                const bool perform_maximum_likelihood_estimation_;
                size_t n_MLE_iterations_;
                double MLE_conv_threshold_;
                std::map<SYMBOL, std::vector<ComplexMatrix>> mBasisSymbols_to_Projectors_;
        };

        /**
        * @brief The type-erased QuantumStateTomography handle exposed in the python bindings.
        */
        class QuantumStateTomographyPython {
            public:
                template <ExecutableWorkflow EXECWORKFLOW, typename SYMBOL = Pauli>
                requires MatrixTranslatable<SYMBOL> && CircuitAppendable<SYMBOL> && HasIdentity<SYMBOL>
                QuantumStateTomographyPython(
                    EXECWORKFLOW& workflow,
                    const std::set<size_t>& qubits,
                    const bool perform_maximum_likelihood_estimation = false,
                    const std::vector<SYMBOL>& basis = std::vector<Pauli>{Pauli::Symbol::X, Pauli::Symbol::Y, Pauli::Symbol::Z},
                    const SYMBOL& use_for_identity = Pauli::Symbol::Z
                ) : workflow_ptr_(std::make_unique<QuantumStateTomography<EXECWORKFLOW, SYMBOL>>(workflow, qubits, perform_maximum_likelihood_estimation, basis, use_for_identity))
                {}

                template <ExecutableWorkflow EXECWORKFLOW, typename SYMBOL = Pauli>
                requires MatrixTranslatable<SYMBOL> && CircuitAppendable<SYMBOL> && HasIdentity<SYMBOL>
                QuantumStateTomographyPython(
                    EXECWORKFLOW& workflow,
                    const bool perform_maximum_likelihood_estimation = false,
                    const std::vector<SYMBOL>& basis = std::vector<Pauli>{Pauli::Symbol::X, Pauli::Symbol::Y, Pauli::Symbol::Z},
                    const SYMBOL& use_for_identity = Pauli::Symbol::Z
                ) : workflow_ptr_(std::make_unique<QuantumStateTomography<EXECWORKFLOW, SYMBOL>>(workflow, perform_maximum_likelihood_estimation, basis, use_for_identity))
                {}

                template <typename SYMBOL>
                requires MatrixTranslatable<SYMBOL> && CircuitAppendable<SYMBOL> && HasIdentity<SYMBOL>
                void set_maximum_likelihood_estimation(
                    const size_t n_MLE_iterations = 100,
                    const double MLE_conv_threshold = 1e-3,
                    const std::map<SYMBOL, std::vector<ComplexMatrix>>& mBasisSymbols_to_Projectors = std::map<Pauli, std::vector<ComplexMatrix>>{
                        {Pauli::Symbol::X, std::vector<ComplexMatrix>{BlochSphereUnitState(BlochSphereUnitState::Symbol::Xp).get_matrix(), BlochSphereUnitState(BlochSphereUnitState::Symbol::Xm).get_matrix()}},
                        {Pauli::Symbol::Y, std::vector<ComplexMatrix>{BlochSphereUnitState(BlochSphereUnitState::Symbol::Yp).get_matrix(), BlochSphereUnitState(BlochSphereUnitState::Symbol::Ym).get_matrix()}},
                        {Pauli::Symbol::Z, std::vector<ComplexMatrix>{BlochSphereUnitState(BlochSphereUnitState::Symbol::Zp).get_matrix(), BlochSphereUnitState(BlochSphereUnitState::Symbol::Zm).get_matrix()}}
                    }
                ) {
                  workflow_ptr_->set_maximum_likelihood_estimation(n_MLE_iterations, MLE_conv_threshold, mBasisSymbols_to_Projectors);
                }

                std::time_t execute(const std::vector<Task>& tasks) {
                  return workflow_ptr_->execute(tasks);
                }

                std::time_t execute_all() {
                  return workflow_ptr_->execute_all();
                }

                std::vector<ComplexMatrix> assemble_densities(const std::vector<std::map<std::vector<bool>, int>>& measurement_counts) const {
                  return workflow_ptr_->assemble_densities(measurement_counts);
                }

                template <typename SYMBOL>
                requires MatrixTranslatable<SYMBOL> && CircuitAppendable<SYMBOL> && HasIdentity<SYMBOL>
                const std::vector<SYMBOL>& get_basis() const {
                  return workflow_ptr_->get_basis();
                }

                const std::string& get_identifier() const {
                  return workflow_ptr_->get_identifier();
                }

                const std::set<size_t>& get_qubits() const {
                  return workflow_ptr_->get_qubits();
                }

                const std::unique_ptr<QuantumStateTomographyPythonBase>& get() const {
                  return workflow_ptr_;
                }

            private:
                std::unique_ptr<QuantumStateTomographyPythonBase> workflow_ptr_;
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
                * the workflow's qristal::session object, and serialize them.
                */
                void operator()(QuantumStateTomography<EXECWORKFLOW, SYMBOL>& workflow, std::time_t timestamp) const {
                    std::vector<std::map<std::vector<bool>, int>> measured_results;
                    for (qristal::CircuitBuilder& circuit: workflow.get_wrapped_workflow().get_circuits()) { //for each workflow circuit
                        for (qristal::CircuitBuilder& qst_circuit : workflow.append_measurement_bases(circuit)) { //for each appended basis measurement
                            //add measurements
                            for (auto const & qubit : workflow.get_qubits())
                                qst_circuit.Measure(qubit);
                            //add target to session and push results
                            workflow.get_wrapped_workflow().set_session().irtarget = qst_circuit.get();
                            workflow.get_wrapped_workflow().set_session().run();
                            measured_results.push_back(workflow.get_wrapped_workflow().get_session().results());
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

    }
}
