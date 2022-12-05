#include <string>
#include <iostream>

#include "job_constants.hpp"
#include "xacc.hpp"
#include "xacc_service.hpp"


namespace qb::vqee {
// make hardware efficient ansatz: only near neighbour connections, single rotations, not and cnot gates
std::string pennylaneCircuit(const int& nQubits, const int& vqeDepth) {
  std::stringstream circuit;
  circuit << ".compiler xasm\n" << ".circuit ansatz\n" << ".parameters P\n" << ".qbit q\n";
  for (std::size_t i = 0; i < nQubits / 2; i++){
    circuit << "X(q[" << i << "]);\n";
  }
  for (std::size_t d = 0; d < vqeDepth; d++) {
    for (std::size_t i = 0; i < nQubits; i++) {
      const size_t ii = 3*(d*nQubits + i); 
      circuit << "Rz(q[" << i << "], P[" << ii+2 << "]);\n"; 
      circuit << "Rx(q[" << i << "], 0.5*pi);\n"; 
      circuit << "Rz(q[" << i << "], P[" << ii   << "]);\n"; 
      circuit << "Rx(q[" << i << "], -0.5*pi);\n";
      circuit << "Rz(q[" << i << "], P[" << ii+1 << "]);\n";
    }
    for (std::size_t i = 0; i < nQubits - 1; i++){
      circuit << "CNOT(q[" << i << "], q[" << i+1 << "]);\n";
    }
  }
  std::string outString = circuit.str();
  std::cout << outString << std::endl;
  return outString;
}


// Data container for VQE optimizer problems. Everything is constant, except theta and value which should be updated during iterations
struct Params { 
  std::shared_ptr<xacc::CompositeInstruction> ansatz{};
  std::string 		    circuitString{};
  std::string 		    pauliString{};
  double 				      tolerance{1E-6};
  double 				      optimalValue{};
  std::vector<double> energies{};
  std::vector<double> theta{};
  int 				        nQubits{1};
  int 				        nShots{1};
  int 				        maxIters{50};
  int                 nWorker{1};
  int                 nThreadsPerWorker{1};
  bool 				        isDeterministic{true};
  bool                partitioned{false};
};

enum class JobID {H2_explicit, H1_HEA, H2_UCCSD, H2_ASWAP, H5_UCCSD};

Params makeJob(JobID jobID){
  switch (jobID) {
    case JobID::H2_explicit: { 
      //std::cout << "H2, 1.4 Bohr distance between cores in basis STO-3G:" << std::endl;
      // For one iteration starting with theta = 0, <H> should be -1.116714325063 – 1.0/1.4
      // Convergence at theta = -0.22591103 with final energy -1.137275943617 – 1.0/1.4
      Params p{
		.circuitString=R"(
          .compiler xasm
          .circuit ansatz
          .parameters theta
          .qbit q
          Ry(q[0], theta);
        )",
        .pauliString = "-1.04235464570829 + 0.18125791479311 X0 + -0.78864539363997 Z0",
        .theta = std::vector<double>(1, 0.0)};
      return p;
    }
    case JobID::H1_HEA: { 
      //std::cout << "H-1 chain 2 Qbits, 6 parameters and 4 Pauli terms with HWE ansatz:" << std::endl;
      Params p{
        .circuitString=pennylaneCircuit(2, 1), // (nQubits, vqeDepth) 
        .pauliString="-0.2729303635773008 + 0.03963943879866322 Z0 + 0.03963943879866322 Z1 + 0.19365148597997445 Z0Z1",
        .theta = std::vector<double>(6, 0.0), // size(P) = 3*nQubits*vqeDepth
        .nQubits = 2};
      return p;
    }
    case JobID::H2_UCCSD: {
      //std::cout << "H-2 chain 4 Qbits, 3 parameters, 15 Pauli terms (UCCSD ansatz):" << std::endl;
      auto tmp = xacc::getService<xacc::Instruction>("UCCSD");
      auto ansatz = std::dynamic_pointer_cast<xacc::CompositeInstruction>(tmp);
      ansatz->expand({std::make_pair("ne", 2), std::make_pair("nq", 4)}); // ne = #electrons,  nq = #Qbits 
      
      Params p{
        .ansatz=ansatz, 
        //.circuitString=qb::vqee::curcuits::uccsd_H2, // the ansatz returned from python helper is not correct
        .pauliString=qb::vqee::pauli::H2,
        .theta=std::vector<double>(3, 0.0),
        .nQubits=4};
      return p;
    }
    case JobID::H2_ASWAP: {
      //std::cout << "H-2 chain 4 Qbits, 5 parameters, 15 Pauli terms (ASWAP ansatz):" << std::endl;
      auto tmp = xacc::getService<xacc::Instruction>("ASWAP");
      auto ansatz = std::dynamic_pointer_cast<xacc::CompositeInstruction>(tmp);
      ansatz->expand({{"nbQubits",4},  {"nbParticles",2},  {"timeReversalSymmetry",true}});

      Params p{
        .ansatz=ansatz,
        .pauliString=qb::vqee::pauli::H2,
        .theta=std::vector<double>(5, 0.0),
        .nQubits=4};
      return p;
    }
    case JobID::H5_UCCSD: { // uccsd H5 #Hydrogen atoms:  5, electrons:  [3, 2], circuit depth:  5160
      //std::cout << "H-5 chain 10 Qbits, 54 parameters, 444 Pauli terms (UCCSD ansatz):" << std::endl;
      auto tmp = xacc::getService<xacc::Instruction>("UCCSD");
      auto ansatz = std::dynamic_pointer_cast<xacc::CompositeInstruction>(tmp);
      ansatz->expand({std::make_pair("ne", 5), std::make_pair("nq", 10)}); // ne = #electrons,  nq = #Qbits 

      Params p{
        .ansatz=ansatz,
        .pauliString=qb::vqee::pauli::H5,
        .theta=std::vector<double>(54, 0.0),
        .nQubits=10};
      return p;
    }
    default:
      throw std::invalid_argument("\nNo valid example selected!\n");
  }
}
}
