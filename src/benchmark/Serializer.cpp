#include "qb/core/benchmark/Serializer.hpp"

namespace qb
{
    namespace benchmark
    {
        
        std::map<std::string, size_t> convert_to_counts_map(const std::string& bitstrings, const size_t n_qubits) {
            std::map<std::string, size_t> counts_map; 
            std::stringstream ss(bitstrings);
            std::string temp; 
            while (std::getline(ss, temp, '\n')) {
                if (temp[0] == '{' || temp[0] == '}') {
                    continue; //skip opening and closing bracket lines
                }
                std::string bitstring = temp.substr(temp.find_first_of('\"') + 1, n_qubits); 
                size_t counts = std::stoull(temp.substr(temp.find_first_of(":") + 2));
                counts_map[bitstring] = counts;
            }
            return counts_map;
        }

        std::map<size_t, size_t> convert_to_counts_map(const std::string& bitstrings, const size_t n_qubits, const bool r2l_ordered) {
            std::map<size_t, size_t> counts_map; 
            std::stringstream ss(bitstrings);
            std::string temp; 
            while (std::getline(ss, temp, '\n')) {
                if (temp[0] == '{' || temp[0] == '}') {
                    continue; //skip opening and closing bracket lines
                }
                std::string bitstring = temp.substr(temp.find_first_of('\"') + 1, n_qubits); 
                if (!r2l_ordered) {
                    std::reverse(bitstring.begin(), bitstring.end());
                }
                size_t bitnumber = std::stoi(bitstring, 0, 2);
                size_t counts = std::stoull(temp.substr(temp.find_first_of(":") + 2));
                counts_map[bitnumber] = counts;
            }
            return counts_map;
        }

        std::vector<std::map<std::string, size_t>> convert_to_string_count_maps(const std::vector<std::string>& list_of_bitstrings, const size_t n_qubits) {
            std::vector<std::map<std::string, size_t>> count_maps; 
            for (const auto& i : list_of_bitstrings) {
                count_maps.push_back(convert_to_counts_map(i, n_qubits));
            }
            return count_maps;
        }

        std::vector<std::map<size_t, size_t>> convert_to_count_maps(const std::vector<std::string>& list_of_bitstrings, const size_t n_qubits, const bool r2l_ordered) {
            std::vector<std::map<size_t, size_t>> count_maps; 
            for (const auto& i : list_of_bitstrings) {
                count_maps.push_back(convert_to_counts_map(i, n_qubits, r2l_ordered));
            }
            return count_maps;
        }


    } // namespace benchmark
    
} // namespace qb
