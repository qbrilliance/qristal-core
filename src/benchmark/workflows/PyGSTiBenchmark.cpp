#include <fstream>

#include "qb/core/benchmark/workflows/PyGSTiBenchmark.hpp"

namespace qb
{
    namespace benchmark
    {
        PyGSTiBenchmark::PyGSTiBenchmark(std::istream&& instream, qb::session& session) : 
        session_(session) {
            std::string line; 
            while (std::getline(instream, line)) {
                pyGSTi_circuit_strings_.push_back(line);
            }
        }

        std::vector<qb::CircuitBuilder> PyGSTiBenchmark::get_circuits() const {
            std::vector<qb::CircuitBuilder> circuits;
            for (const auto& circuit_string : pyGSTi_circuit_strings_) {
                qb::CircuitBuilder circuit;
                //find all gates in the circuit string representation by scanning through all Gate regex
                for (std::sregex_iterator it = std::sregex_iterator(circuit_string.begin(), circuit_string.end(), gate_regex_); it != std::sregex_iterator(); ++it) {
                    std::smatch match = *it; 
                    //read in qubit numbers 
                    std::vector<size_t> qubits;
                    size_t index = match.position(0) + match.str(0).size(); 
                    while (circuit_string[index] == ':') {
                        //read in qubit index as long as the the next char is a number 
                        std::stringstream ss;
                        while (isdigit(circuit_string[++index])) {
                            ss << circuit_string[index];
                        }
                        size_t qubit_index; 
                        ss >> qubit_index;
                        qubits.push_back(qubit_index);
                    }
                    //add gate 
                    pyGSTistring2appendcircuitfunc_.at(match.str(0))(circuit, qubits);
                }
                circuits.push_back(circuit);
            }
            return circuits;
        }


    } // namespace benchmark
    
} // namespace qb
