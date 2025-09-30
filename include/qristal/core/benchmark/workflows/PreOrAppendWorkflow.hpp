// Copyright (c) Quantum Brilliance Pty Ltd

#pragma once

#include <qristal/core/benchmark/Serializer.hpp> // contains <qristal/core/session.hpp> & typedefs
#include <qristal/core/benchmark/Concepts.hpp>
#include <qristal/core/benchmark/Task.hpp>
#include <qristal/core/primitives.hpp>

namespace qristal
{
    namespace benchmark {

        /**
        * @brief Pure virtual python bindings helper class not used in the C++ implementation.
        */
        class PreOrAppendWorkflowPythonBase {
            public:
                virtual ~PreOrAppendWorkflowPythonBase() = default;
                virtual std::time_t execute(const std::vector<Task>& tasks) = 0;
                virtual std::vector<qristal::CircuitBuilder> get_circuits() const = 0;
                virtual const std::string& get_identifier() const = 0;
        };

        enum class Placement {
            Prepend, Append
        };

        /**
        * @brief Wrapping workflow to pre- or append arbitrary circuits or circuit convertible symbols to the 
        * workflow circuits. 
        */
        template <CircuitConstructingWorkflow WORKFLOW> 
        class PreOrAppendWorkflow : public virtual PreOrAppendWorkflowPythonBase {
            public:
                /**
                * @brief Constructor for workflow wrapper to pre- or append arbitrary circuits. 
                *
                * Arguments:
                * @param workflow: the wrapped workflow.
                * @param circuits: the quantum circuits to pre- or append.
                * @param placement: Wether or not to prepend the given circuits. Defaults to Placement::Prepend.
                * 
                * @return ---
                */
                PreOrAppendWorkflow(
                    WORKFLOW& workflow, 
                    const std::vector<qristal::CircuitBuilder>& circuits, 
                    const Placement placement = Placement::Prepend
                ) : workflow_(workflow), 
                    circuits_(circuits), 
                    identifier_(std::string("PreOrAppend") + workflow_.get_identifier()), 
                    placement_(placement)
                {}

                /**
                * @brief Overloaded constructor for workflow wrapper to pre- or append a single circuit. 
                *
                * Arguments:
                * @param workflow: the wrapped workflow.
                * @param circuit: the quantum circuit to pre- or append.
                * @param placement: Wether or not to prepend the given circuits. Defaults to Placement::Prepend.
                * 
                * @return ---
                */
                PreOrAppendWorkflow(
                    WORKFLOW& workflow,
                    const qristal::CircuitBuilder& circuit,
                    const Placement placement = Placement::Prepend
                ) : PreOrAppendWorkflow(
                        workflow,
                        std::vector<qristal::CircuitBuilder>{circuit},
                        placement
                ) {}

                /**
                * @brief Templated constructor for workflow wrapper to pre- or append arbitrary circuits given as 
                * circuit convertible symbols (e.g. Paulis or BlochSphereUnitStates).
                *
                * Arguments:
                * @param workflow: the wrapped workflow.
                * @param circuits: the quantum circuits to pre- or append given as a vector of vectors of circuit convertible symbols.
                * @param placement: Wether or not to prepend the given circuits. Defaults to Placement::Prepend.
                * 
                * @return ---
                */
                template <typename Symbol>
                requires CircuitAppendable<Symbol>
                PreOrAppendWorkflow(
                    WORKFLOW& workflow, 
                    const std::vector<std::vector<Symbol>>& circuits, 
                    const Placement placement = Placement::Prepend
                ) : workflow_(workflow), 
                    identifier_(std::string("PreOrAppend") + workflow_.get_identifier()), 
                    placement_(placement)
                {
                    for (const auto& symbols: circuits) {
                        qristal::CircuitBuilder cb;
                        for (size_t q = 0; q < symbols.size(); ++q) {
                            symbols[q].append_circuit(cb, q);
                        }
                        circuits_.push_back(cb);
                    }
                }

                /**
                * @brief Overloaded templated constructor for workflow wrapper to pre- or an individual circuit given as 
                * a vector of circuit convertible symbols (e.g. Paulis or BlochSphereUnitStates).
                *
                * Arguments:
                * @param workflow: the wrapped workflow.
                * @param circuit: the quantum circuit to pre- or append given as a vector of circuit convertible symbols.
                * @param placement: Wether or not to prepend the given circuits. Defaults to Placement::Prepend.
                * 
                * @return ---
                */
                template <typename Symbol>
                requires CircuitAppendable<Symbol>
                PreOrAppendWorkflow(
                    WORKFLOW& workflow, 
                    const std::vector<Symbol>& circuit, 
                    const Placement placement = Placement::Prepend
                ) : PreOrAppendWorkflow(
                        workflow,
                        std::vector<std::vector<Symbol>>{circuit},
                        placement
                ) {}

                /**
                * @brief Construct all pre- or appended quantum circuits of the wrapped workflow.
                *
                * Arguments: ---
                *
                * @return std::vector<qristal::CircuitBuilder> a vector of quantum circuits in the form of qristal::CircuitBuilder objects
                */
                std::vector<qristal::CircuitBuilder> get_circuits() const {
                    std::vector<qristal::CircuitBuilder> circuits; 
                    for (const auto& workflow_circuit : workflow_.get_circuits()) {
                        for (const auto& pre_or_appended_circuit : circuits_) {
                            qristal::CircuitBuilder cb; 
                            if (placement_ == Placement::Prepend) {
                                cb = pre_or_appended_circuit.copy();
                            }
                            cb.append(workflow_circuit);
                            if (placement_ == Placement::Append) {
                                cb.append(pre_or_appended_circuit);
                            }
                            circuits.push_back(cb);
                        }
                    }
                    return circuits;
                }

                /**
                * @brief Run pre- or appended wrapped workflow and store results for specific tasks.
                *
                * Arguments:
                * @param tasks: a selection of Tasks to be executed.
                *
                * @return std::time_t: the time stamp of the successful execution.
                */
                std::time_t execute(const std::vector<Task>& tasks) {
                    return executeWorkflowTasks<PreOrAppendWorkflow>(*this, tasks);
                }

                /**
                * @brief Return a constant reference to the unique std::string identifier of the PreOrAppend workflow
                */
                const std::string& get_identifier() const {return identifier_;}
                /**
                * @brief Return a constant reference to the assigned qristal::session
                */
                const qristal::session& get_session() const {return workflow_.get_session();}
                /**
                * @brief Return a reference to the assigned qristal::session
                */
                qristal::session& set_session() const {return workflow_.set_session();}

                /**
                * @brief Serialization method for measured bit string counts
                *
                * Arguments:
                * @param counts the measured bit string counts returned by qristal::session
                * @param time the time stamp of execution
                *
                * @return ---
                */
                void serialize_measured_counts(const std::vector<std::map<std::vector<bool>, int>>& counts, const std::time_t time ) const {
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

            private:
                WORKFLOW& workflow_;
                std::vector<qristal::CircuitBuilder> circuits_;
                const Placement placement_;

                const std::string identifier_;
        };

        /**
        * @brief The type-erased PreOrAppendWorkflow handle exposed in the python bindings.
        */
        class PreOrAppendWorkflowPython {
            public:
                //constructor overloads for plain workflows 
                template <CircuitConstructingWorkflow WORKFLOW> 
                PreOrAppendWorkflowPython(
                    WORKFLOW& workflow, 
                    const std::vector<qristal::CircuitBuilder>& circuits,
                    const Placement placement = Placement::Prepend
                ) : workflow_ptr_(std::make_unique<PreOrAppendWorkflow<WORKFLOW>>(workflow, circuits, placement))
                {}

                template <CircuitConstructingWorkflow WORKFLOW> 
                PreOrAppendWorkflowPython(
                    WORKFLOW& workflow, 
                    const qristal::CircuitBuilder& circuit, 
                    const Placement placement = Placement::Prepend
                ) : workflow_ptr_(std::make_unique<PreOrAppendWorkflow<WORKFLOW>>(workflow, circuit, placement))
                {}

                template <CircuitConstructingWorkflow WORKFLOW, typename Symbol>
                requires CircuitAppendable<Symbol>
                PreOrAppendWorkflowPython(
                    WORKFLOW& workflow, 
                    const std::vector<std::vector<Symbol>>& circuits, 
                    const Placement placement = Placement::Prepend
                ) : workflow_ptr_(std::make_unique<PreOrAppendWorkflow<WORKFLOW>>(workflow, circuits, placement))
                {}

                template <CircuitConstructingWorkflow WORKFLOW, typename Symbol>
                requires CircuitAppendable<Symbol>
                PreOrAppendWorkflowPython(
                    WORKFLOW& workflow, 
                    const std::vector<Symbol>& circuit, 
                    const Placement placement = Placement::Prepend
                ) : workflow_ptr_(std::make_unique<PreOrAppendWorkflow<WORKFLOW>>(workflow, circuit, placement))
                {}
                
                //recursive overloads for wrapped PreOrAppendWorkflowPython
                PreOrAppendWorkflowPython(
                    PreOrAppendWorkflowPython& workflow, 
                    const std::vector<qristal::CircuitBuilder>& circuits, 
                    const Placement placement = Placement::Prepend
                );
                PreOrAppendWorkflowPython(
                    PreOrAppendWorkflowPython& workflow, 
                    const qristal::CircuitBuilder& circuit, 
                    const Placement placement = Placement::Prepend
                );
                //Symbol specializations
                //avoid templates here to keep this header separated from the other workflows
                PreOrAppendWorkflowPython(
                    PreOrAppendWorkflowPython& workflow, 
                    const std::vector<std::vector<qristal::Pauli>>& circuits, 
                    const Placement placement = Placement::Prepend
                );
                PreOrAppendWorkflowPython(
                    PreOrAppendWorkflowPython& workflow, 
                    const std::vector<qristal::Pauli>& circuit, 
                    const Placement placement = Placement::Prepend
                );
                PreOrAppendWorkflowPython(
                    PreOrAppendWorkflowPython& workflow, 
                    const std::vector<std::vector<qristal::BlochSphereUnitState>>& circuits, 
                    const Placement placement = Placement::Prepend
                );
                PreOrAppendWorkflowPython(
                    PreOrAppendWorkflowPython& workflow, 
                    const std::vector<qristal::BlochSphereUnitState>& circuit, 
                    const Placement placement = Placement::Prepend
                );

                std::time_t execute(const std::vector<Task>& tasks) {
                    return workflow_ptr_->execute(tasks);
                }

                const std::string& get_identifier() const {
                    return workflow_ptr_->get_identifier();
                }

                std::vector<qristal::CircuitBuilder> get_circuits() const {
                    return workflow_ptr_->get_circuits();
                }

                const std::unique_ptr<PreOrAppendWorkflowPythonBase>& get() const {
                    return workflow_ptr_;
                }

            private:
                std::unique_ptr<PreOrAppendWorkflowPythonBase> workflow_ptr_;
        };

    } //namespace benchmark
} // namespace qristal
