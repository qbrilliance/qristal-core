#include "qristal/core/benchmark/workflows/PyGSTiBenchmark.hpp"
#include "qristal/core/benchmark/metrics/PyGSTiResults.hpp"

#include "qristal/core/session.hpp"
#include "qristal/core/circuit_builder.hpp"
#include "qristal/core/noise_model/noise_model.hpp"

#include <iostream>

using namespace qristal::benchmark;

int main() {
    const size_t n_qubits = 2;
    const size_t n_shots = 1000;

    //(1) define session  
    qristal::session sim(false); 
    sim.init();
    sim.set_acc("aer");
    sim.set_noise(true);
    qristal::NoiseModel nm("default", n_qubits);
    sim.set_noise_model(nm);
    sim.set_sn(n_shots);
    sim.set_qn(n_qubits);

    //(2) define workflow (read in pyGSTi circuits from std::cin)
    PyGSTiBenchmark workflow(std::move(std::cin), sim);

    //(3) pass to metric
    PyGSTiResults<PyGSTiBenchmark> metric(workflow);

    //(4) force new evaluation of circuits and print the results in pyGSTi compatible format
    auto results = metric.evaluate(true, false); //force new execution, set verbose to false
    for (const auto& [_, result] : results) {
        for (size_t i = 0; i < result.size(); ++i) {
            std::cout << result[i] << std::endl;
        }
    }
}