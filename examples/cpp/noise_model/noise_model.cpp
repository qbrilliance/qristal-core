// Copyright (c) 2022 Quantum Brilliance Pty Ltd

#include "qb/core/session.hpp"

int main(int argc, char * argv[]) 
{
    auto my_sim = qb::session();
    my_sim.qb12();          // Set up meaningful defaults
    my_sim.set_qn(2);       // 2 qubits
    my_sim.set_acc("aer");  // Aer simulator selected
    my_sim.set_noise(true); // Set this to true for noise models to be active
    my_sim.set_noise_model("default"); // Also available from the Qristal Emulator: "qb-nm1" , "qb-nm2" , "qb-qdk1"
    my_sim.set_instring(R"(
       OPENQASM 2.0;
       include "qelib1.inc";
       creg c[2];
       h q[0];
       cx q[0],q[1];
       measure q[1] -> c[1];
       measure q[0] -> c[0];
       )");
    my_sim.run();
    std::string result = ((my_sim.get_out_raws()).at(0)).at(0);
    std::cout << result << std::endl;
    return 0;
}
