// Copyright (c) 2022 Quantum Brilliance Pty Ltd
/**
 * This example shows how to make your own noise model.
 */ 


#include "qb/core/session.hpp"
#include "qb/core/noise_model/noise_model.hpp"

// Build and return a noise model for an n-qubit ring
qb::NoiseModel ring_noise_model(size_t nb_qubits)
{
    // Make an empty noise model
    qb::NoiseModel noise_model;
  
    // Name the model whatever you like
    noise_model.name = "ring_noise_model";

    // Define the gate fidelities (errors are 1 - fidelity)
    constexpr double u1_error = 1e-4;
    constexpr double u2_error = 1e-3;
    constexpr double u3_error = 1e-3;
    constexpr double cx_error = 1e-2;

    // Define the readout errors
    qb::ReadoutError ro_error;
    ro_error.p_01 = 1e-2;
    ro_error.p_10 = 5e-3;

    // Loop over the qubits
    for (size_t qId = 0; qId < nb_qubits; ++qId)
    {
        // Set the readout errors
        noise_model.set_qubit_readout_error(qId, ro_error);

        // Set the single-qubit gate fidelities
        noise_model.add_gate_error(qb::DepolarizingChannel::Create(qId, u1_error), "u1", {qId});
        noise_model.add_gate_error(qb::DepolarizingChannel::Create(qId, u2_error), "u2", {qId});
        noise_model.add_gate_error(qb::DepolarizingChannel::Create(qId, u3_error), "u3", {qId});

        // Set the qubit connections to form a ring
        const size_t qId2 = (qId != nb_qubits - 1 ? qId + 1 : 0);
        noise_model.add_qubit_connectivity(qId, qId2);

        // Set the corresponding two-qubit gate fidelities
        noise_model.add_gate_error(qb::DepolarizingChannel::Create(qId, qId2, cx_error), "cx", {qId, qId2});
        noise_model.add_gate_error(qb::DepolarizingChannel::Create(qId, qId2, cx_error), "cx", {qId2, qId});
    }
    
    return noise_model;
}


int main(int argc, char * argv[]) 
{
    qb::session my_sim;

    // 2 qubits
    const int n = 2;

    // Set up meaningful defaults
    my_sim.qb12();                  

    // Set the number of qubits
    my_sim.set_qn(n);               

    // Aer simulator selected
    my_sim.set_acc("aer");          

    // Set this to true to include noise
    my_sim.set_noise(true);  
    
    // Hand over the noise model to the session.
    my_sim.set_noise_model(ring_noise_model(n));  

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

    // Hit it.
    my_sim.run();

    // Lookee.
    std::string result = ((my_sim.get_out_raws()).at(0)).at(0);
    std::cout << result << std::endl;

    // Bye.
    return 0;
}
