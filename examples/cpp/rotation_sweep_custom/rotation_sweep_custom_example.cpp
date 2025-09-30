#include <qristal/core/benchmark/workflows/RotationSweep.hpp>
#include <qristal/core/benchmark/workflows/PreOrAppendWorkflow.hpp>
#include <qristal/core/benchmark/metrics/BitstringCounts.hpp>
#include <qristal/core/primitives.hpp>

#include <time.h>
#include <range/v3/view/zip.hpp>

using namespace qristal::benchmark;

int main() {

    const size_t n_qubits = 1;

    //(1) set up session
    qristal::session sim;
    sim.acc = "qpp";
    sim.sn = 1000;
    sim.qn = n_qubits;

    //(2) define workflow 
    RotationSweep base_workflow({'Z'}, -180, +180, 5, sim);
    //Prepend rotation to |X+> to workflow circuits 
    std::vector<qristal::BlochSphereUnitState> prepend {qristal::BlochSphereUnitState::Symbol::Xp};
    PreOrAppendWorkflow<RotationSweep> prepended_workflow(
        base_workflow, 
        prepend, 
        Placement::Prepend
    );
    //Append measurements (X, Z) to workflow circuits
    std::vector<std::vector<qristal::Pauli>> append {
        {qristal::Pauli::Symbol::X}, 
        {qristal::Pauli::Symbol::Z}
    };
    PreOrAppendWorkflow<PreOrAppendWorkflow<RotationSweep>> final_workflow(
        prepended_workflow, 
        append, 
        Placement::Append
    );

    //(3) pass to metric
    BitstringCounts<PreOrAppendWorkflow<PreOrAppendWorkflow<RotationSweep>>> metric(final_workflow);
    std::vector<qristal::CircuitBuilder> circuits = final_workflow.get_circuits();

    //(4) evaluate and print
    auto results = metric.evaluate();
    for (const auto& [timestamp, counts_vec] : results) {
        std::cout << "Evaluated metric from " << "UTC: " << std::put_time(std::gmtime(&timestamp), "%c %Z")
                  << "(local: " << std::put_time(std::localtime(&timestamp), "%c %Z") << "): [" << std::endl;
        for (const auto& [counts, circuit] : ::ranges::views::zip(counts_vec, circuits)) {
            std::cout << "Circuit:" << std::endl; 
            circuit.print(); 
            std::cout << "Measured bitstring counts:" << std::endl; 
            std::cout << counts << std::endl;
            std::cout << "---" << std::endl;
        }
    }

}
