#include <string>
#include <iostream>

#include "job_constants.hpp"

// from xacc libs
#include "Utils.hpp"
#include "xacc.hpp"
#include "xacc_service.hpp"
#include "xacc_observable.hpp"
#include "ObservableTransform.hpp"


namespace qb::vqee {

/// Data container for VQE optimizer problems. Everything is constant, except theta and value which should be updated during iterations
struct Params { 
  std::shared_ptr<xacc::CompositeInstruction> ansatz{};
  std::string         circuitString{};
  std::string         pauliString{};
  std::string         acceleratorName{"qpp"};
  double              tolerance{1E-6};
  double              optimalValue{};
  std::vector<double> energies{};
  std::vector<double> theta{};
  int                 nQubits{1};
  int                 nShots{1};
  int                 maxIters{50};
  int                 nWorker{1};
  int                 nThreadsPerWorker{1};
  bool                isDeterministic{true};
  bool                partitioned{false};
};

// print ansatz to string
std::string ansatzToString(const std::shared_ptr<xacc::CompositeInstruction> ansatz);

// create ansatz from string
std::shared_ptr<xacc::CompositeInstruction> stringToAnsatz(const std::string ansatz);

// make hardware efficient ansatz: only near neighbour connections, single rotations, not and cnot gates
std::string HEA_String(const std::size_t nQubits, const std::size_t vqeDepth);
std::shared_ptr<xacc::CompositeInstruction> HEA_Ansatz(const std::size_t nQubits, const std::size_t vqeDepth);

// UCCSD ansatz
std::shared_ptr<xacc::CompositeInstruction> UCCSD_Ansatz(const int nQubits, const int nElectrons);

// ASWAP ansatz
std::shared_ptr<xacc::CompositeInstruction> ASWAP_Ansatz(const int nQubits, const int nParticles, const bool timeReversalSymmetry=true);

enum class AnsatzID {HEA, UCCSD, ASWAP};
std::string getEnumName(AnsatzID ansatzID);
AnsatzID getEnumFromName(std::string ansatzIDstr);

// sets ansatz in params according to ansatzID, sets its circuit string and returns number of optimization parameters in ansatz 
std::size_t setAnsatz(Params& params, const AnsatzID ansatzID, const int nQubits, const int nDEP, const bool TRS=true);

// generate Pauli string from molecule geometry using pyscf with sto-3g basis and Jordan Wigner transformation 
// Geometry string: e.g., H 0.0 0.0 0.0; H 0.0 0.0 0.735
// Unit: ANGSTROM
std::string pauliStringFromGeometry(const std::string& geometry, const std::string& basis = "sto-3g");

// generates geometry string for hydrogen chain with 1.4 bohr distance between atoms
std::string hydrogenChainGeometry(const std::size_t nHydrogen);

// list of available examples
enum class JobID {H2_explicit, H1_HEA, H2_UCCSD, H2_ASWAP, H5_UCCSD};

// generates predefined example case setup
Params makeJob(JobID jobID);

} // end of namespace qb::vqee

