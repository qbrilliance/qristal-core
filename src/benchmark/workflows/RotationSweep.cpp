// Copyright (c) 2023 Quantum Brilliance Pty Ltd
#include <numbers>
#include <cmath>
#include <complex>
#include <boost/dynamic_bitset.hpp>
#include <unsupported/Eigen/KroneckerProduct>

#define ZIP_VIEW_INJECT_STD_VIEWS_NAMESPACE //to add zip to the std namespace
#include "qb/core/tools/zip_tool.hpp"
#include "qb/core/benchmark/workflows/RotationSweep.hpp"


namespace qb
{
    namespace benchmark
    {

        RotationSweep::RotationSweep( const std::vector<char>& rotations_per_qubit, //either X, Y, Z or I (do nothing) for each qubit
                                      const int& start_degree,
                                      const int& end_degree,
                                      const size_t& n_points, //number of data points to collect for each sweep
                                      qb::session& session ) : 
        rotations_per_qubit_(rotations_per_qubit), 
        start_degree_(start_degree),
        end_degree_(end_degree),
        n_points_(n_points),
        session_(session) 
        {
            assert( (start_degree < end_degree) && "start_degree has to be smaller than end_degree!");
            assert( (n_points > 1) && "Specify at least two points!");
            session_.set_qn(rotations_per_qubit_.size());
        }

        std::vector<qb::CircuitBuilder> RotationSweep::get_circuits() const 
        {
            std::vector<qb::CircuitBuilder> circuits; 
            for (double current_rad = start_rad(); current_rad <= end_rad(); current_rad += step())
            {
                //hotfix for xacc error for very small rotation angles
                if (fabs(current_rad) <= 1e-6)
                    current_rad = 0.0;

                qb::CircuitBuilder cb;
                for (size_t q = 0; q < rotations_per_qubit_.size(); ++q)
                {
                    switch (rotations_per_qubit_[q])
                    {
                        case 'X':
                        case 'x':
                            cb.RX(q, current_rad);
                            break;
                        case 'Y':
                        case 'y':
                            cb.RY(q, current_rad);
                            break;
                        case 'Z':
                        case 'z':
                            cb.RZ(q, current_rad);
                            break;
                        default:
                            //for "I" or anything else, do nothing
                            break;
                    }
                }
                circuits.push_back(cb);
            }
            return circuits;
        }

        void executeWorkflowTask<RotationSweep, Task::IdealCounts>::operator()(RotationSweep & workflow, std::time_t timestamp) const {
            size_t n_shots = workflow.get_session().get_sns()[0][0];
            size_t n_bitstrings = std::pow(2, workflow.get_rotations_per_qubit().size());
            qb::String ideal_results;
            //no need to build the circuits at all! Just calculate the probabilities and set exact counts 
            //Rx(theta) |0> = cos(theta/2)|0> - isin(theta/2)|1>  ---> p(0) = cos(theta/2)^2 ; p(1) = sin(theta/2)^2
            //Ry(theta) |0> = cos(theta/2)|0> +  sin(theta/2)|1>  ---> p(0) = cos(theta/2)^2 ; p(1) = sin(theta/2)^2
            //Rz(theta) |0> = |0> ---> p(0) + 1 ; p(1) = 0
            for (double current_rad = workflow.start_rad(); current_rad <= workflow.end_rad(); current_rad += workflow.step())
            {
                std::vector<double> probs{
                    cos(current_rad / 2.0) * cos(current_rad / 2.0),
                    sin(current_rad / 2.0) * sin(current_rad / 2.0)
                };
                std::stringstream current_result;
                for ( std::size_t bitstring = 0; bitstring < n_bitstrings; ++bitstring ) 
                {
                    boost::dynamic_bitset<> current(workflow.get_rotations_per_qubit().size(), bitstring);
                    std::string s; 
                    boost::to_string(current, s);
                    std::reverse(s.begin(), s.end()); //boost::dynamic_bitset uses left to right bit ordering!
                    double combined_prob = 1.0; 
                    for ( size_t i = 0; i < current.size(); ++i )
                    {
                        if (workflow.get_rotations_per_qubit()[i] == 'X' || workflow.get_rotations_per_qubit()[i] == 'x' || workflow.get_rotations_per_qubit()[i] == 'Y' || workflow.get_rotations_per_qubit()[i] == 'y' )
                            combined_prob *= probs[current[i]];
                        else 
                            combined_prob *= (current[i] == 0 ? 1.0 : 0.0); //for Z or I
                    }
                    if ( fabs(combined_prob) > 1e-16 ) { //only include non-zero probabilities
                        std::stringstream ss; 
                        current_result << "\"" << s << "\": " << round(combined_prob * n_shots) << std::endl;
                    }
                }
                ideal_results.push_back(current_result.str());
            }
            workflow.serialize_ideal_counts(ideal_results, timestamp);
        }
        
        void executeWorkflowTask<RotationSweep, Task::IdealDensity>::operator()(RotationSweep & workflow, std::time_t timestamp) const {
            std::vector<ComplexMatrix> ideal_densities; 
            //no need to build or measure the full circuits. Just Calculate the 1-qubit densities for each rotation and then build tensor product
            //Rx(theta) |0> = cos(theta/2)|0> - isin(theta/2)|1> 
            // --> rho = c^2|0><0| -ics|1><0| + ics|0><1| + s^2|1><1| 
            //Ry(theta) |0> = cos(theta/2)|0> +  sin(theta/2)|1>
            // --> rho = c^2|0><0| + cs|1><0| + cs |0><1| + s^2|1><1|
            //Rz(theta) |0> = I |0> = |0>
            // --> rho = |0><0|
            for (double current_rad = workflow.start_rad(); current_rad <= workflow.end_rad(); current_rad += workflow.step())
            {
                double c = cos(current_rad / 2.0);
                double s = sin(current_rad / 2.0);
                ComplexMatrix total_density(1, 1);
                total_density(0, 0) = std::complex<double>(1.0, 0.0); 
                for ( const auto& q : workflow.get_rotations_per_qubit() )
                {
                    ComplexMatrix curr_density = ComplexMatrix::Zero(2, 2);
                    switch (q)
                    {
                        case 'X':
                        case 'x':
                        {
                            curr_density(0, 0) = std::complex<double>(c*c, 0.0);
                            curr_density(1, 0) = std::complex<double>(0.0, -1.0*c*s);
                            curr_density(0, 1) = std::complex<double>(0.0, c*s);
                            curr_density(1, 1) = std::complex<double>(s*s, 0.0);
                            break;
                        }
                        case 'Y':
                        case 'y':
                        {
                            curr_density(0, 0) = std::complex<double>(c*c, 0.0);
                            curr_density(1, 0) = std::complex<double>(c*s, 0.0);
                            curr_density(0, 1) = std::complex<double>(c*s, 0.0);
                            curr_density(1, 1) = std::complex<double>(s*s, 0.0);
                            break;
                        }
                        default: 
                        {
                            curr_density(0, 0) = std::complex<double>(1.0, 0.0);
                            break;
                        }
                    }
                    total_density = Eigen::kroneckerProduct(total_density, curr_density).eval();
                }
                ideal_densities.push_back(total_density);
            }
            workflow.serialize_ideal_densities(ideal_densities, timestamp);
        }

        void executeWorkflowTask<RotationSweep, Task::IdealProcess>::operator()(RotationSweep & workflow, std::time_t timestamp) const {
            std::vector<ComplexMatrix> ideal_processes; 
            //no need to build and measure the full circuits. Just calculate the 1-qubit processes ror each rotation and then build the tensor product 
            //Since it is R_P = exp(-i theta/2 P) = cos(theta/2) I - i*sin(theta/2) P
            //A general rotation process is R_P rho R_P' = cos(theta/2)^2 I rho I +  i*sin(theta/2)cos(theta/2) I rho P - i*sin(theta/2)cos(theta/2) P rho I + sin(theta/2)^2 P rho P
            for (double current_rad = workflow.start_rad(); current_rad <= workflow.end_rad(); current_rad += workflow.step()) {
                double c = cos(current_rad / 2.0);
                double s = sin(current_rad / 2.0);
                ComplexMatrix process = ComplexMatrix::Ones(1, 1);
                for (const auto& q : workflow.get_rotations_per_qubit()) {
                    ComplexMatrix curr_process = ComplexMatrix::Zero(4, 4);
                    size_t index = 0;
                    switch (q) {
                        case 'X':
                        case 'x': {
                            index = 1;
                            break;
                        }
                        case 'Y':
                        case 'y': {
                            index = 2;
                            break;
                        }
                        case 'Z':
                        case 'z': {
                            index = 3;
                            break;
                        }
                    }
                    curr_process(0, 0) += std::complex<double>(c*c, 0.0);
                    curr_process(0, index) += std::complex<double>(0.0, -1.0*s*c);
                    curr_process(index, 0) += std::complex<double>(0.0, s*c);
                    curr_process(index, index) += std::complex<double>(s*s, 0.0);
                    //merge into total process
                    process = Eigen::kroneckerProduct(curr_process, process).eval();
                }
                ideal_processes.push_back(process);
            }
            workflow.serialize_ideal_processes(ideal_processes, timestamp);
        }

    } // namespace qb::benchmark
} // namespace qb