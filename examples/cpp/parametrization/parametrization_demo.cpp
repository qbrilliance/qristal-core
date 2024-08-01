#include "qristal/core/circuit_builder.hpp"
#include "qristal/core/session.hpp"

#include <iomanip>

int main() {
  // And we're off!
  std::cout << "Executing parametrized circuit C++ demo..." << std::endl;

  // Make a Qristal session
  auto my_sim = qristal::session(false);

  // Set up sensible default parameters
  my_sim.init();

  // Choose a simulator backend
  my_sim.set_acc("sparse-sim");

  // Choose how many qubits to simulate
  my_sim.set_qn(2);

  // Choose how many 'shots' to run through the circuit
  my_sim.set_sn(10'000);

  // Choose to enable gradient calculations
  my_sim.set_calc_jacobian(true);

  // Define the quantum program to run (aka 'quantum kernel' aka 'quantum circuit')
  auto circ1 = qristal::CircuitBuilder();
  circ1.RX(0, "alpha");
  circ1.RX(0, "beta");
  circ1.Measure(0);

  // Can use a map to define parameter mapping
  std::map<std::string, double> circ1_param_map;
  circ1_param_map["alpha"] = M_PI_2;
  circ1_param_map["beta"] = 2*M_PI/3;
  // Then, convert the parameters to a vector for runtime evaluation
  std::vector<double> circ1_param_vec = circ1.param_map_to_vec(circ1_param_map);

  // Define another quantum program, with different parameters and a different number of qubits
  auto circ2 = qristal::CircuitBuilder();
  circ2.RX(0, "alpha2");
  circ2.RX(1, "beta2");
  circ2.MeasureAll(-1);

  // Can also directly set the parameters as a vector. The parameters will be
  // assigned in order of definition in the circuit (i.e. index 0 of vector
  // mapped to "alpha2", index 1 mapped to "beta2", etc.). If a parameter is
  // used on multiple gates, the index still corresponds to the first definition
  // relative to the other unique parameters.
  std::vector<double> circ2_param_vec = {M_PI/3, 2*M_PI/7};

  // Set the target circuits and parameters accordingly
  my_sim.set_irtarget_ms({{circ1.get()}, {circ2.get()}});
  my_sim.set_parameter_vectors({{circ1_param_vec}, {circ2_param_vec}});

  // Run the circuit 10000 times and count up the results in each of the classical registers
  std::cout << "About to run quantum program..." << std::endl;
  my_sim.run();
  std::cout << "Ran successfully!" << std::endl;
  qristal::Table2d<qristal::Table2d<double>> out_prob_gradients = my_sim.get_out_prob_jacobians();

  // Print the raw shot results, probabilities, and jacobians
  size_t num_outputs = qristal::ipow(2, circ1.num_qubits());
  std::cout << "Results 1:" << std::endl << my_sim.results()[0][0] << std::endl;
  std::cout << "Circ 1 probabilities: \n";
  for (size_t idx = 0; auto elem: my_sim.get_out_probs()[0][0]) {
    std::cout << "Probability for index " << idx << ": " << elem << "\n";
    idx += 1;
  }
  std::cout << "]\n";
  std::cout << "Results 2:" << std::endl << my_sim.results()[1][0] << std::endl;
  std::cout << "Circ 2 probabilities: \n";
  size_t idx = 0;
  for (size_t idx = 0; auto elem: my_sim.get_out_probs()[1][0]) {
    std::cout << "Probability for index " << idx << ": " << elem << "\n";
    idx += 1;
  }

  std::cout << "Circ 1 jacobian: \n[";
  for (size_t i = 0; i < circ1.num_free_params(); i++) {
    std::cout << "[";
    for (size_t j = 0; j < num_outputs; j++) {
      std::cout << out_prob_gradients[0][0][i][j] << ", ";
    }
    std::cout << "],\n";
  }
  size_t num_outputs2 = qristal::ipow(2, circ2.num_qubits());
  std::cout << "Circ 2 jacobian: \n[";
  for (size_t i = 0; i < circ2.num_free_params(); i++) {
    std::cout << "[";
    for (size_t j = 0; j < num_outputs2; j++) {
      std::cout << out_prob_gradients[1][0][i][j] << ", ";
    }
    std::cout << "],\n";
  }
}
