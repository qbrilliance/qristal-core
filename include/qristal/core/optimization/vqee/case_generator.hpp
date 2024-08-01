#include <string>
#include <iostream>

#include "job_constants.hpp"
#include "yaml-cpp/yaml.h"

// from xacc libs
#include "Utils.hpp"
#include "xacc.hpp"
#include "xacc_service.hpp"
#include "xacc_observable.hpp"
#include "ObservableTransform.hpp"


namespace qristal::vqee {

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
  /// @brief algorithm : sets the classical optimisation algorithm, e.g "nelder-mead", "cobyla", "l-bfgs"
  std::string         algorithm{"cobyla"};
  /// @brief extraOptions : YAML format for options to use with the classical optimiser
  std::string         extraOptions{};
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

/**
 * @brief Selection of classical optimisation algorithms and associated hyperparameters.
 * Override get() in a derived class and provide a definition that validates the relevant hyperparameters there.
 * 
 * Additional hyperparameters ie "extra options" should be stored in the YAML::Node member [m_node_]
 */
class VqeOpt {
  protected:
    /// Name for the C++ optimisation library.  These libraries supply a collection of algorithms through a consistent interface.
    std::string m_provider_;

    /// Name for an algorithm that performs optimisation
    std::string m_algorithm_;

    /// Initial values for theta (the parameters that will be varied to find the optimum)
    std::vector<double> m_initial_parameters_;

    /// Max number of iterations
    int m_maxeval_;

    /// Function tolerance
    double m_ftol_;

    /// Extra options to supply as configuration information to the optimiser
    YAML::Node m_node_;
  public:
    /// Default constructor
    VqeOpt() :
      m_provider_{},
      m_algorithm_{},
      m_initial_parameters_{},
      m_maxeval_{},
      m_ftol_{},
      m_node_{}
    {}

    /// Constructor for the most common use case
    VqeOpt(const std::string& in_provider,
           const std::string& in_algorithm) :
      m_provider_(in_provider),
      m_algorithm_ (in_algorithm)
    {}

    /// Constructor for initialisation of all data members
    VqeOpt(const std::string& in_provider,
           const std::string& in_algorithm,
           const std::vector<double>& in_initial_parameters,
           const int in_maxeval,
           const double in_ftol,
           const YAML::Node& in_node) :
                 m_provider_(in_provider),
                 m_algorithm_(in_algorithm),
                 m_initial_parameters_(in_initial_parameters),
                 m_maxeval_(in_maxeval),
                 m_ftol_(in_ftol),
                 m_node_(in_node)
    {}

    /// Getters
    virtual std::shared_ptr<xacc::Optimizer> get() {}; // provide override in derived class
    
    /**
     * Helper template for converting YAML to XACC options
     * 
     * @param node YAML formatted string of extra options
     * @param keys std::set of key names expected in the YAML that all have a corresponding value of type T
     * @param yaml_to_xacc_keys Lookup from keys in YAML to keys recognised by XACC
     * @param in_xoptions Resulting XACC options 
     * @param debug Debug flag
     */
    template<typename T> 
    static void pass_yaml_to_xacc(const YAML::Node& node, 
                                  const std::set<std::string>& keys, 
                                  const std::map<std::string, std::string>& yaml_to_xacc_keys,
                                  xacc::HeterogeneousMap& in_xoptions,
                                  bool debug = false) {
      for (const std::string& key : keys) {
        // Make sure key has a xacc equivalent
        if (yaml_to_xacc_keys.count(key) != 1) {
          throw std::range_error("No XACC equivalent for " + key);
        }  
        // If the key has been passed, send it on to XACC
        if (node[key]) {
          if (debug) std::cout << "Adding: " << yaml_to_xacc_keys.at(key) << " : " << node[key] << "\n";
          in_xoptions.insert(yaml_to_xacc_keys.at(key), node[key].as<T>());
        }
      }
    }
};

/** @brief ADAptive Momentum (ADAM) estimator algorithm from the mlpack library
 * ADAM is a stochastic gradient descent algorithm often used in machine learning applications.
 */
class AdamMLP : public VqeOpt {
  private:
    /// Extra options accepted by ADAM that will be detected from the YAML string
    /**
     * Step size for each iteration
     *   YAML key: "stepsize"
     *   XACC key: "mlpack-step-size"
     *   Default: [double] 0.5
     * 
     * Exponential decay rate for the first moment estimates
     *   YAML key: "beta1"
     *   XACC key: "mlpack-beta1"
     *   Default: [double] 0.7
     * 
     * Exponential decay rate for the weighted infinity-norm estimates
     *   YAML key: "beta2"
     *   XACC key: "mlpack-beta2"
     *   Default: [double] 0.999
     * 
     * Mean-squared gradient initial value
     *   YAML key: "eps"
     *   XACC key: "mlpack-eps"
     *   Default: [double] 1.0e-8
     * 
     * Momentum
     *   YAML key: "momentum"
     *   XACC key: "mlpack-momentum"
     *   Default: [double] 0.05
     * 
     * Exact objective
     *   YAML key: "exactobjective"
     *   XACC key: "adam-exact-objective"
     *   Default: [boolean] false
     */
    
    /// Integer-valued fields
    std::set<std::string> integer_valued_fields_{};
    
    /// String-valued fields
    std::set<std::string> string_valued_fields_{};
    
    /// Boolean-valued fields
    std::set<std::string> boolean_valued_fields_{"exactobjective"};
    
    /// Double-valued fields
    std::set<std::string> double_valued_fields_{"stepsize",
                                                "beta1",
                                                "beta2",
                                                "eps",
                                                "momentum"};

    /// Vector of double fields
    std::set<std::string> vector_double_valued_fields_{};

    /// Union of all the above fields. Conversion of keys from YAML -> XACC
    std::map<std::string, std::string> all_valid_fields_yaml_xacc_{
                                                {"exactobjective","adam-exact-objective"},
                                                {"stepsize","mlpack-step-size"},
                                                {"beta1","mlpack-beta1"},
                                                {"beta2","mlpack-beta2"},
                                                {"eps","mlpack-eps"},
                                                {"momentum","mlpack-momentum"}};
              
  public:
    /// Default constructor - calls the base class and sets the provider name and algorithm name
    AdamMLP() : VqeOpt("mlpack", "adam") {};

    /// Constructor that shows all defaults
    AdamMLP(const std::vector<double>& in_initial_parameters,
                  const int in_maxeval = 500000,
                  const double in_ftol = 1.0e-4,
                  const YAML::Node& in_node = YAML::Load("")) : VqeOpt("mlpack", "adam")
    {
      m_initial_parameters_ = in_initial_parameters;
      m_maxeval_ = in_maxeval;
      m_ftol_ = in_ftol;
      m_node_ = in_node;
    }

    /// Getters
    std::shared_ptr<xacc::Optimizer> get() override; 
};

/** @brief CMA-ES - Covariance Matrix Adaptation Evolution Strategy is a stochastic search algorithm from the mlpack C++ library.
 *  It works by estimating a positive definite matrix within an iterative procedure using the covariance matrix.
 *  In this instance, the batchSize is fixed (= 1), and 
 *  SelectionPolicy is also fixed (= FullSelection).
 */
class CmaesMLP : public VqeOpt {
  private:
    /// Extra options accepted by CMA-ES that will be detected from the YAML string
    /**
     * The population size
     *   YAML key: "lambda"
     *   XACC key: "mlpack-cmaes-lambda"
     *   Default: [int] 0
     * 
     * Upper-bound of decision variables
     *   YAML key: "upper"
     *   XACC key: "mlpack-cmaes-upper-bound"
     *   Default: [double] 10.0
     * 
     * Lower-bound of decision variables
     *   YAML key: "lower"
     *   XACC key: "mlpack-cmaes-lower-bound"
     *   Default: [double] -10.0
     */
    
    /// Integer-valued fields
    std::set<std::string> integer_valued_fields_{"lambda"};
    
    /// String-valued fields
    std::set<std::string> string_valued_fields_{};
    
    /// Boolean-valued fields
    std::set<std::string> boolean_valued_fields_{};
    
    /// Double-valued fields
    std::set<std::string> double_valued_fields_{"upper",
                                                "lower"};

    /// Vector of double fields
    std::set<std::string> vector_double_valued_fields_{};

    /// Union of all the above fields. Conversion of keys from YAML -> XACC
    std::map<std::string, std::string> all_valid_fields_yaml_xacc_{
                                                {"lambda","mlpack-cmaes-lambda"},
                                                {"upper","mlpack-cmaes-upper-bound"},
                                                {"lower","mlpack-cmaes-lower-bound"}};
              
  public:
    /// Default constructor - calls the base class and sets the provider name and algorithm name
    CmaesMLP() : VqeOpt("mlpack", "cmaes") {};

    /// Constructor that shows all defaults
    CmaesMLP(const std::vector<double>& in_initial_parameters,
                  const int in_maxeval = 500000,
                  const double in_ftol = 1.0e-4,
                  const YAML::Node& in_node = YAML::Load("")) : VqeOpt("mlpack", "cmaes")
    {
      m_initial_parameters_ = in_initial_parameters;
      m_maxeval_ = in_maxeval;
      m_ftol_ = in_ftol;
      m_node_ = in_node;
    }

    /// Getters
    std::shared_ptr<xacc::Optimizer> get() override; 
};

/** @brief L-BFGS algorithm from the mlpack library
 *  L-BFGS is a gradient-based algorithm (quasi-Newton) 
 */
class LbfgsMLP : public VqeOpt {
  private:
    /// Integer-valued fields
    std::set<std::string> integer_valued_fields_{};
    
    /// String-valued fields
    std::set<std::string> string_valued_fields_{};
    
    /// Boolean-valued fields
    std::set<std::string> boolean_valued_fields_{};
    
    /// Double-valued fields
    std::set<std::string> double_valued_fields_{};

    /// Vector of double fields
    std::set<std::string> vector_double_valued_fields_{};

    /// Union of all the above fields. Conversion of keys from YAML -> XACC
    std::map<std::string, std::string> all_valid_fields_yaml_xacc_{};
              
  public:
    /// Default constructor - calls the base class and sets the provider name and algorithm name
    LbfgsMLP() : VqeOpt("mlpack", "l-bfgs") {};

    /// Constructor that shows all defaults
    LbfgsMLP(const std::vector<double>& in_initial_parameters,
                  const int in_maxeval = 500000,
                  const double in_ftol = 1.0e-4,
                  const YAML::Node& in_node = YAML::Load("")) : VqeOpt("mlpack", "cmaes")
    {
      m_initial_parameters_ = in_initial_parameters;
      m_maxeval_ = in_maxeval;
      m_ftol_ = in_ftol;
      m_node_ = in_node;
    }

    /// Getters
    std::shared_ptr<xacc::Optimizer> get() override; 
};



/** @brief NLOpt library with common options for these algorithms:
 *   - Nelder-Mead
 */
class NLO : public VqeOpt {
  private: 
    /// Integer-valued fields
    std::set<std::string> integer_valued_fields_{};
    
    /// String-valued fields
    std::set<std::string> string_valued_fields_{};
    
    /// Boolean-valued fields
    std::set<std::string> boolean_valued_fields_{"maximise",
                                                 "maximize"};
    
    /// Double-valued fields
    std::set<std::string> double_valued_fields_{"stopval"};

    /// Vector of double fields
    std::set<std::string> vector_double_valued_fields_{"upperbounds",
                                                       "lowerbounds"};
    /// Union of all the above fields. Conversion of keys from YAML -> XACC
    std::map<std::string, std::string> all_valid_fields_yaml_xacc_{
                                                {"maximise","maximize"},
                                                {"maximize","maximize"},
                                                {"stopval","nlopt-stopval"},
                                                {"upperbounds","nlopt-upper-bounds"},
                                                {"lowerbounds","nlopt-lower-bounds"}};
              
  public:
    /// Default constructor - calls the base class and sets the provider name and algorithm name
    NLO() : VqeOpt("nlopt", "cobyla") {};

    /// Simple constructor
    NLO(const std::string& in_algorithm) : VqeOpt("nlopt", in_algorithm) {};

    /// Constructor with defaults shown
    NLO(const std::vector<double>& in_initial_parameters,
        const std::string& in_algorithm = "cobyla",
        const int in_maxeval = 1000,
        const double in_ftol = 1.0e-6,
        const YAML::Node& in_node = YAML::Load("")) : VqeOpt("nlopt", in_algorithm)
    {
      m_initial_parameters_ = in_initial_parameters;
      m_maxeval_ = in_maxeval;
      m_ftol_ = in_ftol;
      m_node_ = in_node;
    }

    /// Getters
    std::shared_ptr<xacc::Optimizer> get() override;
    virtual void show_info() {};
};

/** @brief Nelder-Mead algorithm from the nlopt library
 *  Nelder-Mead is a gradient-free algorithm and works 
 *  best when some noise is present.
 */
class NelderMeadNLO : public NLO {
  private:
    std::string information_{"Nelder-Mead algorithm provided by nlopt"};
  public:
    NelderMeadNLO() : NLO("nelder-mead") {};
    
    /// Constructor with defaults shown
    NelderMeadNLO(const std::vector<double>& in_initial_parameters,
        const int in_maxeval = 1000,
        const double in_ftol = 1.0e-6,
        const YAML::Node& in_node = YAML::Load(""),
        const std::string& in_algorithm = "nelder-mead") : NLO(in_algorithm)
    {
      m_initial_parameters_ = in_initial_parameters;
      m_maxeval_ = in_maxeval;
      m_ftol_ = in_ftol;
      m_node_ = in_node;
    }

    /// Print information
    void show_info() override { std::cout << information_ << "\n"; }
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

} // end of namespace qristal::vqee

