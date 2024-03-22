// Copyright (c) 2023 Quantum Brilliance Pty Ltd
#ifndef _QB_BENCHMARK_SERIALIZER_
#define _QB_BENCHMARK_SERIALIZER_

#include <cereal/archives/binary.hpp>
#include "cereal/types/map.hpp"
#include "cereal/types/vector.hpp"
#include "cereal/types/string.hpp"
#include "cereal/types/tuple.hpp"
#include "cereal/types/complex.hpp"

#include "qb/core/typedefs.hpp"
#include "qb/core/session.hpp"

#include <string> 
#include <ctime>
#include <concepts>
#include <fstream>
#include <Eigen/Dense>
#include <complex>


namespace qb
{

    namespace benchmark
    {

        namespace SerializerConstants {
            static const std::string INTERMEDIATE_RESULTS_FOLDER_NAME = "intermediate_benchmark_results";
        }

    // - - - - - - - - - - free functions to save and load serialized data - - - - - - - - - - //

        using ArchiveIn = cereal::BinaryInputArchive;
        using ArchiveOut = cereal::BinaryOutputArchive;

        /**
        * @brief Concept for serializable container @tparam Container into archives @tparam Archives 
        * 
        * @details Each serializable data object needs to be able to load from and save to a serialized archive. This concept enforces the corresponding member functions load, save, and dump
        */
        template <typename Container, typename Archive>
        concept Serializable = requires(Container c, Archive ar ) {
            {c.template load(ar)} -> std::same_as<void>;
            {c.template save(ar)} -> std::same_as<void>;
            {c.dump()};
        };

        /**
        * @brief Templated function to load data from a serialized container into a payload data structure
        * 
        * Arguments: 
        * @param identifier the unique string identifier of the executed workflow 
        * @param specifier the unique string specifier of the serialized data 
        * @param timestamps a std::vector of time stamps std::time_t to load.

        * @return std::vector<Payload> a std::vector of the loaded data structures in the form of Payload objects.
        * 
        * @details This member function will assemble filenames for each requested timestamp and read in the stored and serialized data from std::ifstream using the templated load function. 
        */
        template< typename Container, typename Payload >
        requires Serializable<Container, ArchiveIn>
        inline std::vector<Payload> load_data( const std::string& identifier, const std::string& specifier, const std::vector<std::time_t>& timestamps) { 
            std::vector<Payload> data;
            for ( const auto& ts : timestamps ) { //for each timestamp
                //(1) assemble filename from identifier and timestamp 
                std::stringstream ss;
                ss << SerializerConstants::INTERMEDIATE_RESULTS_FOLDER_NAME << "/" << identifier << specifier << ts << ".bin";

                //(2) read in data
                std::ifstream in; 
                in.open(ss.str());
                ArchiveIn input(in); 
                Container dataPoint{};
                dataPoint.template load< ArchiveIn >(input);
                in.close(); 

                //(3) store 
                data.push_back(dataPoint.dump());
            }
            return data;
        }

        /**
        * @brief Templated function to save data (in the form of a Payload) to a serialized container
        * 
        * Arguments: 
        * @param identifier the unique string identifier of the executed workflow 
        * @param specifier the unique string specifier of the serialized data+
        * @param payload the data to be stored 
        * @param time the std::time_t time stampe associated with the creation of the payload.

        * @return ---
        * 
        * @details This member function will assemble a filename based on the provided identifier, specifier, and timestamp, and write the stored data, i.e., the payload, to a serialized archive via std::ofstream. 
        */
        template< typename Container, typename Payload >
        requires Serializable<Container, ArchiveOut>
        inline void save_data( const std::string& identifier, const std::string& specifier, const Payload& payload, const std::time_t time ) { 
            std::stringstream ss;
            ss << SerializerConstants::INTERMEDIATE_RESULTS_FOLDER_NAME << "/" << identifier << specifier << time << ".bin";
            std::ofstream file; 
            file.open(ss.str());
            ArchiveOut output(file); 
            Container data(payload);
            data.template save< ArchiveOut >(output);
            file.close(); 
        }


    // - - - - - - - - - - add serialization wrapper around data structs - - - - - - - - - - //

        // - - - SessionInfo - - - //
        /**
        * @brief Container object for qb::session
        * 
        * @details This class wraps around qb::session and stores the relevant information, i.e., accelerator names, noise mitigation models, number of qubits, and number of shots. It provides save, load, and dump member functions as required by the Serializable concept.
        */
        class SessionInfo
        {
            public: 
                SessionInfo() {}
                SessionInfo(const qb::session& session) : 
                    accs_(session.get_accs()),
                    noise_mitigations_(session.get_noise_mitigations()), 
                    qns_(session.get_qns()), 
                    sns_(session.get_sns())
                {
                    //store noise model info as json string
                    for ( const auto& vecnm : session.get_noise_models() ) {
                        std::vector<std::string> temp;
                        for (const auto& nm : vecnm) {
                            temp.push_back(nm.to_json());
                        }
                        noise_models_.push_back(temp);
                    }
                }

                qb::VectorString accs_, noise_mitigations_; 
                std::vector<std::vector<std::string>> noise_models_; //stored in json format
                qb::VectorN qns_, sns_;

                /**
                * @brief Dump function to copy SessionInfo object
                * 
                * Arguments: ---
                * 
                * @return SessionInfo copy of *this 
                */
                SessionInfo dump() const { return *this; }

                /**
                * @brief Store SessionInfo to templated @tparam Archive
                * 
                * Arguments:
                * @param ar cereal archive where information is stored.
                */
                template <typename Archive>
                void save( Archive& ar ) const {
                    //store important session information, no results!
                    ar(accs_, noise_models_, noise_mitigations_, qns_, sns_);
                }

                /**
                * @brief Load SessionInfo from templated @tparam Archive
                * 
                * Arguments:
                * @param ar cereal archive from which information is read in.
                */
                template <typename Archive>
                void load( Archive& ar ) {
                    //load important session information, no results!
                    ar(accs_, noise_models_, noise_mitigations_, qns_, sns_);
                }
        };

        template void SessionInfo::save<ArchiveOut>( ArchiveOut& ) const; //explicitly instantiate
        template void SessionInfo::load<ArchiveIn >( ArchiveIn& );


        // - - - BitCounts - - - //
        /**
        * @brief Helper function to convert from bit string counts from std::string to std::map<std::string, size_t>
        * 
        * Arguments:
        * @param bitstrings bit string counts of type std::string
        * @param n_qubits number of qubits.
        * 
        * @return Converted bit string counts in std::map<std::string, size_t> container.
        */
        std::map<std::string, size_t> convert_to_counts_map(const std::string& bitstrings, const size_t n_qubits); 
        /**
        * @brief Helper function to convert from bit string counts from std::string to std::map<size_t, size_t>
        * 
        * Arguments:
        * @param bitstrings bit string counts of type std::string.
        * @param n_qubits number of qubits.
        * @param r2l_ordered assumed qubit ordering. If true, least significant bit is assumed to be on the very right.
        * 
        * @return Converted bit string counts in std::map<size_t, size_t> container.
        */
        std::map<size_t, size_t> convert_to_counts_map(const std::string& bitstrings, const size_t n_qubits, const bool r2l_ordered); 
        /**
        * @brief Helper function to convert list of bit string counts from qb::String (aka std::vector<std::string>) to std::vector<std::map<std::string, size_t>>
        * 
        * Arguments:
        * @param list_of_bitstrings collection of bit string counts of type qb::String as returned by qb::session.
        * @param n_qubits number of qubits.
        * 
        * @return Converted bit string counts in std::vector<std::map<std::string, size_t>> container.
        */
        std::vector<std::map<std::string, size_t>> convert_to_count_maps(const qb::String& list_of_bitstrings, const size_t n_qubits);
        /**
        * @brief Helper function to convert list of bit string counts from qb::String (aka std::vector<std::string>) to std::vector<std::map<size_t, size_t>>
        * 
        * Arguments:
        * @param list_of_bitstrings collection of bit string counts of type qb::String as returned by qb::session.
        * @param n_qubits number of qubits.
        * @param r2l_ordered assumed qubit ordering. If true, least significant bit is assumed to be on the very right.
        * 
        * @return Converted bit string counts in std::vector<std::map<size_t, size_t>> container.
        */
        std::vector<std::map<size_t, size_t>> convert_to_count_maps(const qb::String& list_of_bitstrings, const size_t n_qubits, const bool r2l_ordered);

        /**
        * @brief Container object for bit string counts from qb::String
        * 
        * @details This class wraps around qb::String and provides save, load, and dump member functions as required by the Serializable concept.
        */
        class BitCounts
        {
            public:
                BitCounts() {}
                BitCounts( const qb::String& results ) : results_(results) {}
                qb::String results_{};

                /**
                * @brief Dump function to copy BitCounts content
                * 
                * Arguments: ---
                * 
                * @return qb::String copy of stored bit string counts. 
                */
                qb::String dump() const { return results_; }

                /**
                * @brief Store BitCounts to templated @tparam Archive
                * 
                * Arguments:
                * @param ar cereal archive where information is stored.
                */
                template <typename Archive>
                void save( Archive& ar ) const {
                    ar(results_);
                }

                /**
                * @brief Load BitCounts from templated @tparam Archive
                * 
                * Arguments:
                * @param ar cereal archive from which information is read in.
                */
                template <typename Archive>
                void load( Archive& ar ) {
                    ar(results_);
                }
        };

        template void BitCounts::save<ArchiveOut>( ArchiveOut& ) const; //explicitly instantiate
        template void BitCounts::load<ArchiveIn >( ArchiveIn& );

        // - - - Density matrices - - - // 
        using ComplexMatrix = Eigen::Matrix<std::complex<double>, Eigen::Dynamic, Eigen::Dynamic>;
        /**
        * @brief Container object for complex matrices.
        * 
        * @details This class wraps around Eigen::Matrix<std::complex<double, Eigen::Dynamic, Eigen::Dynamic>> used for density and process matrices and provides save, load, and dump member functions as required by the Serializable concept.
        */
        class ComplexMatrices 
        {
            public: 
                ComplexMatrices() {}
                ComplexMatrices(const std::vector<ComplexMatrix>& densities) : densities_(densities) {}
                std::vector<ComplexMatrix> densities_; 

                /**
                * @brief Dump function to copy ComplexMatrices content
                * 
                * Arguments: ---
                * 
                * @return std::vector<ComplexMatrix> copy of stored complex matrices. 
                */
                std::vector<ComplexMatrix> dump() const { return densities_; }

                /**
                * @brief Store ComplexMatrices to templated @tparam Archive
                * 
                * Arguments:
                * @param ar cereal archive where information is stored.
                * 
                * @details The stored Eigen matrices are serialized in the following format: Number of matrices, for each matrix: number of rows, number of columns, matrix elements in row-major indexing
                */
                template <typename Archive> 
                void save( Archive& ar ) const {
                    ar(densities_.size());
                    for ( auto const & d : densities_ ) {
                        ar(d.rows());
                        ar(d.cols());
                        for (Eigen::Index row = 0; row < d.rows(); ++row)
                            for (Eigen::Index col = 0; col < d.cols(); ++col)
                                ar(d(row, col));
                    }
                }

                /**
                * @brief Load ComplexMatrices from templated @tparam Archive
                * 
                * Arguments:
                * @param ar cereal archive from which information is read in.
                * 
                * @details Starts by reading in the total number of matrices, then the number of rows and columns for each matrix, initializes an Eigen::Matrix object and fills its content by reading in matrix elements in row-major ordering.
                */
                template <typename Archive> 
                void load( Archive& ar ) {
                    size_t n_matrices; 
                    ar(n_matrices); //read in number of stored density matrices 
                    for (size_t i = 0; i < n_matrices; ++i) {
                        size_t rows, cols; //read in dimensions of the matrix to initialize
                        ar(rows);
                        ar(cols);
                        ComplexMatrix mat(rows, cols);
                        for (size_t row = 0; row < rows; ++row)
                            for (size_t col = 0; col < cols; ++col) {
                                std::complex<double> temp; 
                                ar(temp);
                                mat(row, col) = temp;
                            }
                        densities_.push_back(mat);
                    }

                    
                }
        };

        template void ComplexMatrices::save<ArchiveOut>( ArchiveOut& ) const; //explicitly instantiate
        template void ComplexMatrices::load<ArchiveIn >( ArchiveIn& );
  

        // - - - New wrappers go here - - - //
        // ...
    }
}

#endif