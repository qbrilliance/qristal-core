// Copyright (c) 2022 Quantum Brilliance Pty Ltd
/**
 * This example shows how to turn on noise in a simulation,
 * and how to modify the default noise model used.
 */ 


#include "qb/core/session.hpp"

int main(int argc, char * argv[]) 
{
    qb::session my_sim;

    // 2 qubits
    const int n = 2;

    // Set up meaningful defaults
    my_sim.init();

    // Set the number of qubits
    my_sim.set_qn(n);               

    // Aer simulator selected
    my_sim.set_acc("aer");          

    // Set this to true to include noise
    my_sim.set_noise(true);  
    
    // Define the kernel
    my_sim.set_instring(R"(
       OPENQASM 2.0;
       include "qelib1.inc";
       creg c[2];
       h q[0];
       cx q[0],q[1];
       measure q[1] -> c[1];
       measure q[0] -> c[0];
       )");

    // If the option "--qdk" is passed, attempt to use the noise model
    // "qb-qdk1" from the Qristal Emulator (must be installed). If you
    // just want to use default noise, the following is not needed. 
    std::vector<std::string> arguments(argv + 1, argv + argc);
    bool qdk = false;
    for (auto x : arguments) { if (x == "--qdk") qdk = true; }

    // If the option "--noisier" is passed, inflate readout error. If you 
    // just want to use default noise, the following is not needed. 
    bool noisier = false;
    for (auto x : arguments) { if (x == "--noisier") noisier = true; }

    if (qdk or noisier)    
    {
      // Create a noise model with 2 qubits.
      qb::NoiseModel nm(qdk ? "qb-qdk1" : "default", n);
  
      // If requested, overwrite the readout errors on the first bit of
      // the model with some very large values (for the sake of example).
      if (noisier)
      {
        qb::ReadoutError ro_error;          
        ro_error.p_01 = 0.20;           
        ro_error.p_10 = 0.30;           
        nm.set_qubit_readout_error(0, ro_error);
      }
  
      // Hand over the noise model to the session.  Note that if this call
      // is not made, the default model will automatically get created 
      // with the correct number of qubits and used.
      my_sim.set_noise_model(nm);  
    }

    // Hit it.
    my_sim.run();

    // Lookee.
    std::string result = ((my_sim.get_out_raws_json()).at(0)).at(0);
    std::cout << result << std::endl;

    // Bye.
    return 0;
}
