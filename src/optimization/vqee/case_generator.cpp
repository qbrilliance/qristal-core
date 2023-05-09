// Copyright (c) 2022 Quantum Brilliance Pty Ltd

#include "qb/core/optimization/vqee/case_generator.hpp"
#include <exception>
#include <iomanip>

namespace qb::vqee {

// print ansatz to string
std::string ansatzToString(const std::shared_ptr<xacc::CompositeInstruction> ansatz) {
  return ansatz->toString();
}

// create ansatz from string
std::shared_ptr<xacc::CompositeInstruction> stringToAnsatz(const std::string ansatz) {
  xacc::qasm(ansatz);
  return xacc::getCompiled("ansatz");
}

// make Hardware Efficient Ansatz: only near neighbour connections, single rotations, not and cnot gates
std::string HEA_String(const std::size_t nQubits, const std::size_t vqeDepth) {
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
  // std::cout << outString << std::endl;
  return outString;
}

std::shared_ptr<xacc::CompositeInstruction> HEA_Ansatz(const std::size_t nQubits, const std::size_t vqeDepth) {
  xacc::qasm(HEA_String(nQubits, vqeDepth));
  return xacc::getCompiled("ansatz");
}

std::shared_ptr<xacc::CompositeInstruction> UCCSD_Ansatz(const int nQubits, const int nElectrons) {
  auto tmp = xacc::getService<xacc::Instruction>("UCCSD");
  auto ansatz = std::dynamic_pointer_cast<xacc::CompositeInstruction>(tmp);
  ansatz->expand({ std::make_pair("ne", nElectrons), std::make_pair("nq", nQubits) });
  return ansatz;
}

std::shared_ptr<xacc::CompositeInstruction> ASWAP_Ansatz(const int nQubits, const int nParticles, const bool timeReversalSymmetry) {
  auto tmp = xacc::getService<xacc::Instruction>("ASWAP");
  auto ansatz = std::dynamic_pointer_cast<xacc::CompositeInstruction>(tmp);
  ansatz->expand({ {"nbQubits",nQubits},  {"nbParticles",nParticles},  {"timeReversalSymmetry",timeReversalSymmetry} });
  return ansatz;
}

std::string getEnumName(AnsatzID ansatzID){
  switch(ansatzID){
    case AnsatzID::HEA:   return "HEA";
    case AnsatzID::UCCSD: return "UCCSD";
    case AnsatzID::ASWAP: return "ASWAP";
    default: throw std::invalid_argument("\nNo valid enum val!\n");
  }
}

AnsatzID getEnumFromName(std::string ansatzIDstr){
  if      (ansatzIDstr == "HEA")   { return AnsatzID::HEA;   }
  else if (ansatzIDstr == "UCCSD") { return AnsatzID::UCCSD; }
  else if (ansatzIDstr == "ASWAP") { return AnsatzID::ASWAP; }
  else { throw std::invalid_argument("\nNo valid enum val!\n"); }
}

// sets ansatz in params according to ansatzID, sets its circuit string and returns number of optimization parameters in ansatz 
std::size_t setAnsatz(Params& params, const AnsatzID ansatzID, const int nQubits, const int nDEP, const bool TRS){
  if ( nQubits <= 0 || nDEP <= 0 ) { 
      throw std::invalid_argument( "parameters must be greater zero" );
  }

  switch (ansatzID) {
    case AnsatzID::HEA: { 
      params.ansatz = HEA_Ansatz(nQubits, nDEP);
      break;
    }
    case AnsatzID::UCCSD: { 
      params.ansatz = UCCSD_Ansatz(nQubits, nDEP);
      break;
    }
    case AnsatzID::ASWAP: { 
      params.ansatz = ASWAP_Ansatz(nQubits, nDEP, TRS);
      break;
    }
    default: { 
      throw std::invalid_argument( "received unknown ansatz" );
    }
  }
  params.circuitString = ansatzToString(params.ansatz);
  return params.ansatz->getVariables().size();
}


std::string pauliStringFromGeometry(const std::string& geometry, const std::string& basis){
  // Geometry strings, e.g., H 0.0 0.0 0.0; H 0.0 0.0 0.735
  // Unit: ANGSTROM
  //requires xacc::external::load_external_language_plugins();
  auto Ham = xacc::quantum::getObservable("pyscf", {{"basis", basis}, {"geometry", geometry}});
  auto Pauli = xacc::getService<xacc::ObservableTransform>("jw")->transform(Ham);
  return Pauli->toString();
}

std::string hydrogenChainGeometry(const std::size_t nHydrogen){
  const double distance = 0.7408481486; // regular distance between H atoms in Angstrom
  std::stringstream chain{};
  chain << std::setprecision(16);
  for (std::size_t i=0; i<nHydrogen; i++) {
    chain << "H 0 0 " << i*distance << "; ";
  }
  return(chain.str());
}

Params makeJob(JobID jobID){
  double thetaDefault = 0.1;
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
        .theta = std::vector<double>(1, thetaDefault)};
      return p;
    }
    case JobID::H1_HEA: { 
      //std::cout << "H-1 chain 2 Qbits, 6 parameters and 4 Pauli terms with HWE ansatz:" << std::endl;
      Params p{
        .ansatz=HEA_Ansatz(2, 1), // (nQubits, vqeDepth) 
        .pauliString="-0.2729303635773008 + 0.03963943879866322 Z0 + 0.03963943879866322 Z1 + 0.19365148597997445 Z0Z1",
        .theta = std::vector<double>(6, thetaDefault), // size(P) = 3*nQubits*vqeDepth
        .nQubits = 2};
      return p;
    }
    case JobID::H2_UCCSD: {
      //std::cout << "H-2 chain 4 Qbits, 3 parameters, 15 Pauli terms (UCCSD ansatz):" << std::endl;
      auto ansatz = UCCSD_Ansatz(4, 2); // (#Qbits, #electrons)
      const std::string geometry = "H 0.0 0.0 0.0; H 0.0 0.0 0.7408481486";

      Params p{
        .ansatz=ansatz,
        .pauliString=pauliStringFromGeometry(geometry), //qb::vqee::pauli::H2,
        .theta=std::vector<double>(ansatz->getVariables().size(), thetaDefault), //3
        .nQubits=4};
      return p;
    }
    case JobID::H2_ASWAP: {
      //std::cout << "H-2 chain 4 Qbits, 5 parameters, 15 Pauli terms (ASWAP ansatz):" << std::endl;
      auto ansatz = ASWAP_Ansatz(4,2,true); // (#Qbits, #Particles, timeReversalSymmetry)
      Params p{
        .ansatz=ansatz,
        //.pauliString=qb::vqee::pauli::H2,
        .pauliString=pauliStringFromGeometry(hydrogenChainGeometry(2)),
        .theta=std::vector<double>(ansatz->getVariables().size(), thetaDefault), // 5
        .nQubits=4,
        .maxIters=75};
      return p;
    }
    case JobID::H5_UCCSD: { // uccsd H5 #Hydrogen atoms:  5, electrons:  [3, 2], circuit depth:  5160
      //std::cout << "H-5 chain 10 Qbits, 54 parameters, 444 Pauli terms (UCCSD ansatz):" << std::endl;
      std::string geometry = "H 0.0 0.0 0.0; H 0.0 0.0 0.7408481486";
      auto ansatz = UCCSD_Ansatz(10, 5); // (#Qbits, #electrons)

      Params p{
        .ansatz=ansatz,
        //.pauliString=qb::vqee::pauli::H5,
        .pauliString=pauliStringFromGeometry(hydrogenChainGeometry(5)),
        .theta=std::vector<double>(ansatz->getVariables().size(), thetaDefault), // 54
        .nQubits=10};
      return p;
    }
    default:
      throw std::invalid_argument("\nNo valid example selected!\n");
  }
}

} // end of namespace qb::vqee
