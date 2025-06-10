// Copyright (c) Quantum Brilliance Pty Ltd
/**
 * This example shows how to turn on noise in a simulation,
 * and how to modify the default noise model used.
 */


#include <qristal/core/session.hpp>

int main(int argc, char * argv[])
{
    // Collect commandline arguments
    std::vector<std::string> arguments(argv + 1, argv + argc);

    // Init a Qristal quantum computing session
    qristal::session my_sim;

    // Set the number of qubits
    my_sim.qn = 2;

    // Set the number of shots
    my_sim.sn = 100;

    // Aer simulator selected
    my_sim.acc = "aer";

    // Choose how many OpenMP threads to use to run the AER simulator
    my_sim.aer_omp_threads = 8;

    // Include noise
    my_sim.noise = true;

    // Define the kernel
    my_sim.instring = R"(
       OPENQASM 2.0;
       include "qelib1.inc";
       creg c[2];
       h q[0];
       cx q[0],q[1];
       measure q[1] -> c[1];
       measure q[0] -> c[0];
       )";

    // If the option "--qdk" is passed, attempt to use the noise model
    // "qb-qdk1" from the Qristal Emulator (must be installed). If you
    // just want to use default noise, the following is not needed.
    // Note that the default value of session::noise_model points to an
    // instance of the default model with number qubits equal to my_sim.qn.
    for (auto x : arguments)
    {
      if (x == "--qdk") my_sim.noise_model = std::make_shared<qristal::NoiseModel>("qb-qdk1", my_sim.qn);
    }

    // If the option "--noisier" is passed, inflate readout error. If you
    // just want to use default noise, the following is not needed.
    for (auto x : arguments)
    {
      if (x == "--noisier")
      {
        qristal::ReadoutError ro_error;
        ro_error.p_01 = 0.20;
        ro_error.p_10 = 0.30;
        my_sim.noise_model->set_qubit_readout_error(0, ro_error);
      }
    }

    // Hit it.
    my_sim.run();

    // Lookee.
    std::cout << my_sim.results() << std::endl;

    // Bye.
    return 0;
}
