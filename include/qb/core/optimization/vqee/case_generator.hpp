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

/**
 * @brief A structure for organising and visualising VQE iterations
*/
struct vqe_iteration_data
{
  double energy;
  std::vector<double> params;
  bool operator==(const vqe_iteration_data& r) const {
    return (energy == r.energy and params == r.params);
  }
};

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
  /// @brief  iterationData : a structure for per-iteration energy and theta
  std::vector<vqe_iteration_data> iterationData{};
  int                 nQubits{1};
  int                 nShots{1};
  int                 maxIters{50};
  int                 nWorker{1};
  int                 nThreadsPerWorker{1};
  bool                isDeterministic{true};
  bool                partitioned{false};
  /// @brief  enableVis : when set to true, the vis member will be filled
  /// with ASCII bar graphs.  These provide a visual cue of the VQE convergence 
  /// from a text-only interface.  
  bool                enableVis{false};
  /// @brief showTheta : when set to true, selected elements of theta are added to the visualisation
  bool                showTheta{false};
  /// @brief limitThetaN : limit to this number of elements of theta to visualise. 0 => no limit
  size_t              limitThetaN{0};
  /// @brief  tail : visualise the last n=tail iterations only
  size_t              tail{0};
  /// @brief  plain : when set to true, no colour codes are output in vis
  bool                plain{false};
  /// @brief  blocked : when set to true, all elements in any given iteration are visualised in a single block.
  bool                blocked{false};
  /// @brief  vis : a visualisation of energy and each element of theta, at selected iterations of VQE.
  std::string         vis{};
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

