#include <string>
#include <iostream>

#include "job_constants.hpp"
#include "xacc.hpp"
#include "xacc_service.hpp"


namespace qb::vqee {
// make hardware efficient ansatz: only near neighbour connections, single rotations, not and cnot gates
std::string hardwareEfficientCircuit(const int& nQubits, const int& vqeDepth);

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

// list of available examples
enum class JobID {H2_explicit, H1_HEA, H2_UCCSD, H2_ASWAP, H5_UCCSD};

// generates predefined example case setup
Params makeJob(JobID jobID);

} // end of namespace qb::vqee

