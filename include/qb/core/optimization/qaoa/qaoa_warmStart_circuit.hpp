/// Modifications Copyright (c) Quantum Brilliance.
///
/// This file is adapted from one that is part of XACC: 
/// https://github.com/eclipse/xacc/blob/master/quantum/plugins/algorithms/qaoa/qaoa_circuit.hpp

/*******************************************************************************
 * Copyright (c) 2019 UT-Battelle, LLC.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompanies this
 * distribution. The Eclipse Public License is available at
 * http://www.eclipse.org/legal/epl-v10.html and the Eclipse Distribution
 *License is available at https://eclipse.org/org/documents/edl-v10.php
 *
 * Contributors:
 *   Thien Nguyen - initial API and implementation
 *******************************************************************************/
#pragma once

#include "Circuit.hpp"
#include "IRProvider.hpp"
#include "Observable.hpp"
#include "xacc_service.hpp"

namespace xacc {
namespace circuits {
// Generate WS_QAOA ansatz for VQE
class WS_QAOA : public xacc::quantum::Circuit {
public:
    WS_QAOA() : Circuit("ws_qaoa") { 
        // Default runtime arguments
        arguments.push_back(std::make_shared<xacc::CompositeArgument>("qReg", "qreg"));
    }

    bool expand(const xacc::HeterogeneousMap &runtimeOptions) override;
    const std::vector<std::string> requiredKeys() override;
    void applyRuntimeArguments() override;
    DEFINE_CLONE(WS_QAOA);

private:
    std::shared_ptr<CompositeInstruction> constructParameterizedKernel(bool extendedMode = true);
    void parseObservables(Observable* costHam);
private:
    size_t m_nbQubits;
    size_t m_nbSteps;
    std::vector<std::string> m_costHam;
    std::string good_cut;
    // Should we shuffle the terms when generating the circuit?
    // This technically doesn't matter but may be relevant for placement and scheduling
    // i.e. gates on different set of qubits can be aligned -> reduce depth.
    // Users may loop over the algorithm to observe the final circuit depth.
    bool m_shuffleTerms = false;
};
} // namespace circuits
} // namespace xacc
