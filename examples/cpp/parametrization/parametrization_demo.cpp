#include <qristal/core/circuit_builder.hpp>
#include <qristal/core/session.hpp>

#include <iomanip>

int main() {
  // And we're off!
  std::cout << "Executing parametrized circuit C++ demo..." << std::endl;

  // Make a Qristal session
  qristal::session my_sim;

  // Choose a simulator backend
  my_sim.acc = "sparse-sim";

  // Choose how many qubits to simulate
  my_sim.qn = 2;

  // Choose how many 'shots' to run through the circuit
  my_sim.sn = 10000;

  // Choose to enable gradient calculations
  my_sim.calc_gradients = true;

  // Define the quantum program to run (aka 'quantum kernel' aka 'quantum circuit')
  qristal::CircuitBuilder circ1;
  circ1.RX(0, "alpha");
  circ1.RX(0, "beta");
  circ1.Measure(0);
  my_sim.irtarget = circ1.get();

  // Can use a map to define parameter mapping
  std::map<std::string, double> circ1_param_map;
  circ1_param_map["alpha"] = M_PI_2;
  circ1_param_map["beta"] = 2*M_PI/3;
  // Then, convert the parameters to a vector for runtime evaluation
  my_sim.circuit_parameters = circ1.param_map_to_vec(circ1_param_map);

  // Run the circuit 10000 times, count up the results and print them.
  my_sim.run();
  std::cout << "Results 1:" << std::endl << my_sim.results() << std::endl;
  std::cout << "Circ 1 probabilities: \n";
  for (size_t idx = 0; auto elem: my_sim.all_bitstring_probabilities()) {
    std::cout << "Probability for index " << idx << ": " << elem << "\n";
    idx += 1;
  }
  std::cout << "\n";
  std::cout << "Circ 1 jacobian: \n[";
  for (size_t i = 0; i < circ1.num_free_params(); i++) {
    std::cout << "[";
    for (size_t j = 0; j < qristal::ipow(2, my_sim.qn); j++) {
      std::cout << my_sim.all_bitstring_probability_gradients().at(i).at(j) << ", ";
    }
    std::cout << "],\n";
  }

  // Define another quantum program, with different parameters and a different number of qubits
  qristal::CircuitBuilder circ2;
  circ2.RX(0, "alpha2");
  circ2.RX(1, "beta2");
  circ2.MeasureAll(-1);
  my_sim.irtarget = circ2.get();

  // Can also directly set the parameters as a vector. The parameters will be
  // assigned in order of definition in the circuit (i.e. index 0 of vector
  // mapped to "alpha2", index 1 mapped to "beta2", etc.). If a parameter is
  // used on multiple gates, the index still corresponds to the first definition
  // relative to the other unique parameters.
  std::vector<double> circ2_param_vec = {M_PI/3, 2*M_PI/7};
  my_sim.circuit_parameters = circ2_param_vec;

  // Run the circuit 10000 times, count up the results and print them.
  my_sim.run();
  std::cout << "Results 1:" << std::endl << my_sim.results() << std::endl;
  std::cout << "Circ 2 probabilities: \n";
  for (size_t idx = 0; auto elem: my_sim.all_bitstring_probabilities()) {
    std::cout << "Probability for index " << idx << ": " << elem << "\n";
    idx += 1;
  }
  std::cout << "\n";
  std::cout << "Circ 2 jacobian: \n[";
  for (size_t i = 0; i < circ2.num_free_params(); i++) {
    std::cout << "[";
    for (size_t j = 0; j < qristal::ipow(2, my_sim.qn); j++) {
      std::cout << my_sim.all_bitstring_probability_gradients().at(i).at(j) << ", ";
    }
    std::cout << "],\n";
  }

}
