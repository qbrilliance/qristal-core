# Validating Qristal Jacobian Calculations

Test data used to unit test Jacobian calculations in Qristal was generated manually. A simple circuit was run 16,000 times using the "aer" backend. 16 processes were launched concurrently and looped 1000 times overnight using something like:

```
for i in {1..16}; do
  (
    for j in {1..1000}; do
      build/save_jacobian_calc proc$i 2>/dev/null && echo "Process $i: iteration $j completed"
    done
  ) &
done
wait
```

The simple jacobian calculation program is below:

```
#include <qristal/core/circuit_builder.hpp>
#include <qristal/core/session.hpp>

#include <fstream>

int main(int argc, char** argv)
{
  qristal::session session;
  session.acc = "aer";
  constexpr int32_t number_of_qubits = 2;
  session.qn = number_of_qubits;
  constexpr int32_t number_of_shots = 1000000;
  session.sn = number_of_shots;
  session.calc_gradients = true;

  auto circuit = qristal::CircuitBuilder();
  circuit.RX(0, "alpha");
  circuit.RX(1, "beta");
  circuit.MeasureAll(-1);
  constexpr double alpha = M_PI / 3;
  constexpr double beta = 2 * M_PI / 7;
  std::vector<double> circuit_param_vec = {alpha, beta};
  const int32_t number_of_parameters = circuit_param_vec.size();
  session.irtarget = circuit.get();
  session.circuit_parameters = circuit_param_vec;

  session.run();

  const qristal::session::OutProbabilityGradientsType &out_prob_jacobians =
    session.all_bitstring_probability_gradients();

  std::fstream file(fmt::format("jacobians_{}.txt", argv[1]), std::ios::app);
  file << fmt::format("{}\n", fmt::join(out_prob_jacobians, " "));
  file.close();

  return 0;
}

```

Results were then consolidated (`cat jacobian* > jacobians.txt`) into a single file. Each line in [jacobians.txt](./jacobians.txt) has the following format for the test circuit:

$$
\begin{bmatrix}
\frac{\partial P_{00}}{\partial \alpha} & \frac{\partial P_{10}}{\partial \alpha} & \frac{\partial P_{01}}{\partial \alpha} & \frac{\partial P_{11}}{\partial \alpha}
\end{bmatrix}
\begin{bmatrix}
\frac{\partial P_{00}}{\partial \beta} & \frac{\partial P_{10}}{\partial \beta} & \frac{\partial P_{01}}{\partial \beta} & \frac{\partial P_{11}}{\partial \beta}
\end{bmatrix}
$$

The file was then parsed using [jacobians_std_dev.py](./jacobians_std_dev.py) to calculate the standard deviation for each calculated jacobian element on the 16,000 sample set. This standard deviation was then used as the basis for the tolerance of subsequent session runs jacobian calculations in the gtest.
