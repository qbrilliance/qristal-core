// Copyright (c) 2023 Quantum Brilliance Pty Ltd
#ifndef _QB_BENCHMARK_PYGSTIBENCHMARK_
#define _QB_BENCHMARK_PYGSTIBENCHMARK_

#include <regex>

#include "qb/core/benchmark/Serializer.hpp" // contains "qb/core/session.hpp" & typedefs
#include "qb/core/benchmark/Task.hpp"
#include "qb/core/circuit_builder.hpp"

namespace qb
{

    namespace benchmark
    {
        /**
        * @brief Wrapper for imported workflows from the python package pyGSTi
        * 
        * @details This workflow class may be used to execute pyGSTi experiment designs. It will read in a 
        * given pyGSTi circuit list (one line string representation) and store it in a std::vector<std::string>
        * container. Upon execution only, the given list is converted to qb::CircuitBuilder objects, and then 
        * executed through the provided qb::session.
        */
        class PyGSTiBenchmark 
        { 
            public:
                /**
                * @brief Constructor for pyGSTi imported workflows using a pyGSTi circuit string list
                * 
                * Arguments: 
                * @param circuit_list a std::vector of std::string (one line) circuit representations used by pyGSTi.
                * @param session a reference to the qb::session where the workflow is supposed to be executed.

                * @return ---
                */
                PyGSTiBenchmark(const std::vector<std::string>& circuit_list, qb::session& session) : pyGSTi_circuit_strings_(circuit_list), session_(session) {}

                /**
                * @brief Constructor for pyGSTi imported workflows using a std::istream
                * 
                * Arguments: 
                * @param instream a std::istream&& from which the std::string (one line) circuit representations used by pyGSTi are read in.
                * @param session a reference to the qb::session where the workflow is supposed to be executed.

                * @return ---
                */
                PyGSTiBenchmark(std::istream&& instream, qb::session& session); 

                /**
                * @brief Constructor for pyGSTi imported workflows using external file (produced by pyGSTi)
                * 
                * Arguments: 
                * @param circuit_list_file the file name of an exported pyGSTi circuit list.
                * @param session a reference to the qb::session where the workflow is supposed to be executed.

                * @return ---
                */
                PyGSTiBenchmark(const std::string& circuit_list_file, qb::session& session) : PyGSTiBenchmark(std::ifstream(circuit_list_file), session) {}

                /**
                * @brief Run pyGSTi workflow and store results for specific tasks
                * 
                * Arguments: 
                * @param tasks a selection of Tasks to be executed using the initialized pyGSTi workflow.
                * 
                * @return std::time_t the time stamp of the successful execution
                * 
                * @details This member function is used to execute specific tasks the pyGSTi workflow is capable of. These include storing 
                * (i) the measured bit string counts after circuit execution, and
                * (ii) the relevant information contained in the passed qb::session. 
                * Beware that an actual circuit execution is only triggered for task (i). 
                */
                std::time_t execute(const std::vector<Task>& tasks) {
                    return executeWorkflowTasks<PyGSTiBenchmark>(*this, tasks);
                }

                /**
                * @brief Run pyGSTi workflow and store results for all possible tasks
                * 
                * Arguments: ---
                * 
                * @return std::time_t the time stamp of the successful execution
                * 
                * @details This member function is used to execute all available tasks the pyGSTi workflow is capable of. These include storing 
                * (i) the measured bit string counts after circuit execution, and
                * (ii) the relevant information contained in the passed qb::session.
                */
                std::time_t execute_all() {
                    std::time_t t = execute(std::vector<Task>{Task::MeasureCounts, Task::Session});
                    return t;
                }
                
                /**
                * @brief Return a constant reference to the unique std::string identifier of the SPAM workflow
                */
                const std::string& get_identifier() const {return identifier_;}

                /**
                * @brief Return a constant reference to the assigned qb::session
                */
                const qb::session& get_session() const {return session_;}

                /**
                * @brief Return a reference to the assigned qb::session
                */
                qb::session& set_session() const {return session_;}

                /**
                * @brief Return a constant reference to the pyGSTi circuit list given as std::vector<std::string>
                */
                const std::vector<std::string>& get_pyGSTi_circuit_strings() const {
                    return pyGSTi_circuit_strings_;
                }

                /**
                * @brief Convert the initialized pyGSTi circuit list to qb::CircuitBuilder objects
                * 
                * Arguments: ---
                * 
                * @return std::vector<qb::CircuitBuilder> the converted list of qb::CircuitBuilder objects.
                * 
                * @details This member function will iterate over all given pyGSTi (one line) circuit string representations, 
                * extract gate names and qubit labels (currently hardcoded for less than 10 qubits!), and create qb::CircuitBuilder 
                * representations using the pyGSTistring2appendcircuitfunc_ member of this class. 
                */
                std::vector<qb::CircuitBuilder> get_circuits() const;

                /**
                * @brief Serialization method for measured bit string counts
                * 
                * Arguments: 
                * @param counts the measured bit string counts returned by qb::session 
                * @param time the time stamp of execution
                * 
                * @return ---
                */
                void serialize_measured_counts(const qb::String& counts, const std::time_t time ) const { 
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
                    save_data<SessionInfo, SessionInfo>(identifier_, "_session_" , session_, time); 
                }

            private:
                std::vector<std::string> pyGSTi_circuit_strings_;
                qb::session& session_;

                const std::string identifier_ = "pyGSTi";

                //each pyGSTi gate contains a capital "G" followed by a specific string for the gate name
                const std::regex gate_regex_ = std::regex("(G[a-z]+[0-9]?)"); 
                
                //map pyGSTi gate names from their standard modelpacks to CircuitBuilder operations. Used by the get_circuit() member function. To be appended as needed
                std::map<std::string, std::function<void(qb::CircuitBuilder&, const std::vector<size_t>&)>> pyGSTistring2appendcircuitfunc_{
                    std::make_pair(  "Gxpi2", [](qb::CircuitBuilder& circuit, const std::vector<size_t>& qubits){circuit.RX(qubits[0], std::numbers::pi / 2.0);}), //Rx(pi/2)
                    std::make_pair(  "Gxpi4", [](qb::CircuitBuilder& circuit, const std::vector<size_t>& qubits){circuit.RX(qubits[0], std::numbers::pi / 4.0);}), //Rx(pi/4)
                    std::make_pair(  "Gypi2", [](qb::CircuitBuilder& circuit, const std::vector<size_t>& qubits){circuit.RY(qubits[0], std::numbers::pi / 2.0);}), //Ry(pi/2)
                    std::make_pair(  "Gypi4", [](qb::CircuitBuilder& circuit, const std::vector<size_t>& qubits){circuit.RY(qubits[0], std::numbers::pi / 4.0);}), //Rx(pi/4)
                    std::make_pair(  "Gzpi2", [](qb::CircuitBuilder& circuit, const std::vector<size_t>& qubits){circuit.RZ(qubits[0], std::numbers::pi / 2.0);}), //Rz(pi/2)
                    std::make_pair(  "Gzpi4", [](qb::CircuitBuilder& circuit, const std::vector<size_t>& qubits){circuit.RZ(qubits[0], std::numbers::pi / 4.0);}), //Rz(pi/4)
                    std::make_pair(     "Gn", [](qb::CircuitBuilder& circuit, const std::vector<size_t>& qubits){circuit.RX(qubits[0], std::numbers::pi / 2.0); circuit.RY(qubits[0], sqrt(3.0) / 2.0);}), //Rn = Rx(pi/2) Ry(sqrt(3)/2)
                    std::make_pair(  "Gcnot", [](qb::CircuitBuilder& circuit, const std::vector<size_t>& qubits){circuit.CNOT(qubits[0], qubits[1]);}), //CNOT
                    std::make_pair(    "Gcz", [](qb::CircuitBuilder& circuit, const std::vector<size_t>& qubits){circuit.CZ(qubits[0], qubits[1]);}), //CZ
                    std::make_pair("Gcphase", [](qb::CircuitBuilder& circuit, const std::vector<size_t>& qubits){circuit.CPhase(qubits[0], qubits[1], std::numbers::pi);}), //CPhase
                    std::make_pair(    "Gxx", [](qb::CircuitBuilder& circuit, const std::vector<size_t>& qubits){ //Rxx(pi): no standard gates available in CircuitBuilder -> use native gate decomposition
                        circuit.RY(qubits[0], std::numbers::pi / 2.0); 
                        circuit.X(qubits[0]);
                        circuit.CZ(qubits[0], qubits[1]); 
                        circuit.RX(qubits[1], -1.0 * std::numbers::pi);
                        circuit.CZ(qubits[0], qubits[1]); 
                        circuit.RY(qubits[0], std::numbers::pi / 2.0);
                        circuit.X(qubits[0]);
                    }),
                    std::make_pair( "Gxxpi2", [](qb::CircuitBuilder& circuit, const std::vector<size_t>& qubits){ //Rxx(pi/2): no standard gates available in CircuitBuilder -> use native gate decomposition
                        circuit.RY(qubits[0], std::numbers::pi / 2.0); 
                        circuit.X(qubits[0]);
                        circuit.CZ(qubits[0], qubits[1]); 
                        circuit.RX(qubits[1], std::numbers::pi / 2.0);
                        circuit.CZ(qubits[0], qubits[1]); 
                        circuit.RY(qubits[0], std::numbers::pi / 2.0);
                        circuit.X(qubits[0]);
                    }),
                    std::make_pair(    "Gyy", [](qb::CircuitBuilder& circuit, const std::vector<size_t>& qubits){ //Ryy(pi): no standard gates available in CircuitBuilder -> use native gate decomposition
                        circuit.RX(qubits[0], std::numbers::pi / 2.0); 
                        circuit.RX(qubits[1], -1.0 * std::numbers::pi / 2.0); 
                        circuit.RY(qubits[1], -1.0 * std::numbers::pi / 2.0);
                        circuit.CZ(qubits[0], qubits[1]); 
                        circuit.RX(qubits[1], -1.0 * std::numbers::pi);
                        circuit.CZ(qubits[0], qubits[1]); 
                        circuit.RX(qubits[0], -1.0 * std::numbers::pi / 2.0);
                        circuit.RY(qubits[1], std::numbers::pi / 2.0);
                        circuit.RX(qubits[1], std::numbers::pi / 2.0);
                    }),
                    std::make_pair( "Gyypi2", [](qb::CircuitBuilder& circuit, const std::vector<size_t>& qubits){ //Ryy(pi/2): no standard gates available in CircuitBuilder -> use native gate decomposition
                        circuit.RX(qubits[0], std::numbers::pi / 2.0); 
                        circuit.RX(qubits[1], -1.0 * std::numbers::pi / 2.0); 
                        circuit.RY(qubits[1], -1.0 * std::numbers::pi / 2.0);
                        circuit.CZ(qubits[0], qubits[1]); 
                        circuit.RX(qubits[1], std::numbers::pi / 2.0);
                        circuit.CZ(qubits[0], qubits[1]); 
                        circuit.RX(qubits[0], -1.0 * std::numbers::pi / 2.0);
                        circuit.RY(qubits[1], std::numbers::pi / 2.0);
                        circuit.RX(qubits[1], std::numbers::pi / 2.0);
                    }),
                    std::make_pair(    "Gzz", [](qb::CircuitBuilder& circuit, const std::vector<size_t>& qubits){ //Rzz(pi): no standard gates available in CircuitBuilder -> use native gate decomposition
                        circuit.RY(qubits[1], std::numbers::pi / 2.0); 
                        circuit.X(qubits[1]);
                        circuit.CZ(qubits[0], qubits[1]); 
                        circuit.RX(qubits[1], -1.0 * std::numbers::pi);
                        circuit.CZ(qubits[0], qubits[1]); 
                        circuit.RY(qubits[1], std::numbers::pi / 2.0);
                        circuit.X(qubits[1]);
                    }),
                    std::make_pair( "Gzzpi2", [](qb::CircuitBuilder& circuit, const std::vector<size_t>& qubits){ //Rzz(pi): no standard gates available in CircuitBuilder -> use native gate decomposition
                        circuit.RY(qubits[1], std::numbers::pi / 2.0); 
                        circuit.X(qubits[1]);
                        circuit.CZ(qubits[0], qubits[1]); 
                        circuit.RX(qubits[1], std::numbers::pi / 2.0);
                        circuit.CZ(qubits[0], qubits[1]); 
                        circuit.RY(qubits[1], std::numbers::pi / 2.0);
                        circuit.X(qubits[1]);
                    })
                };
                //Currently missing due to missing functionality in CircuitBuilder: 
                // -> (): Idle gate, I

        };
        
        //no executeWorkflowTask specializations required!


    } // namespace qb::benchmark
} // namespace qb

#endif