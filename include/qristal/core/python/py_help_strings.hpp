// Copyright (c) Quantum Brilliance Pty Ltd

// Help strings for Python bindings
namespace qristal {
  namespace help {

    const char* infiles_ = R"(
        infile:

        Name of a file containing (default: OpenQASM format) a quantum circuit.

        infiles:

        Valid settings: [infile, ...]

        A 1d-array (list) version of session.infile.
    )";


    const char* irtarget_ms_ = R"(
        ir_target:

        Input quantum circuit (core.Circuit).

        ir_targets:

        A 1d-array (list) version of ir_target.
    )";

    const char* instrings_ = R"(
        instring:

        A string that defines a circuit in OpenQASM format.  Simple example:

        .. code-block::

                __qpu__ void qristal_circuit(qreg q) {
                        OPENQASM 2.0;
                        include "qelib1.inc;"
                        creg c0[1];
                        creg c1[1];
                        h q[0];
                        cx q[0],q[1];
                        measure q[0] -> c0[0];
                        measure q[1] -> c1[0];
                }


        instrings:

        Valid settings: [instring, ...]

        A 1d-array (list) version of session.instring.
    )";

    const char* include_qbs_ = R"(
        include_qb:

        A file that contains OpenQASM format gate definitions for custom Quantum Brilliance gates.

        include_qbs:

        Valid settings: [include_qb, ...]

        A 1d-array (list) version of session.include_qb.
    )";

    const char* remote_backend_database_path_ = R"(
        A YAML file that contains configuration data for remote backends (including hardware).
    )";

    const char* accs_ = R"(
        acc:

        Valid settings: aer | tnqvm

        Select a back-end simulator. The single setting applies globally to all infiles, all instrings, and random circuits.

        accs:

        Valid settings: [[aer|tnqvm, ...], [aer|tnqvm, ...]]

        The lead dimension's element 0 matches the vector of infiles, element 1 matches the vector of instrings, element 2 matches the vector of random depths.
    )";

    const char* aer_sim_types_ = R"(
        aer_sim_type:

        Valid settings: statevector | density_matrix | matrix_product_state

        Selects a simulation method for the AER simulator.

        aer_sim_types:

        A 1d-array (list) version of aer_sim_type.
    )";

    const char* xasms_ = R"(
        xasm:

        Valid settings: True | False

        Setting this to True causes circuits to be interpreted in XASM format.  The single setting applies globally to all infiles and all instrings.

        xasms:

        Valid settings: [[True|False, ...], [True|False, ...]]

        The lead dimension's element 0 matches the vector of infiles, element 1 matches the vector of instrings, element 2 matches the vector of randoms.
    )";

    const char* quil1s_ = R"(
        quil1:

        Valid settings: True | False

        Setting this to True causes circuits to be interpreted in Quil 1.0 format. The single setting applies globally to all infiles and all instrings.

        quil1s:

        Valid settings: [[True|False, ...], [True|False, ...]]

        The lead dimension's element 0 matches the vector of infiles, element 1 matches the vector of instrings, element 2 matches the vector of randoms.
    )";

    const char* calc_jacobians_ = R"(
        calc_jacobian:

        Valid settings: True | False

        Setting this to True will calculate gradients for the circuit when session.run() is evoked. The single setting applies globally.
        The gradients calculated will be a jacobian of the output probabilities with respect to the input parameters, i.e. d_probs/d_params.
        Note that calc_jacobian = True implies calc_out_counts = True.

        calc_jacobians:

        Valid settings: [[True|False, ...], [True|False, ...]]

        A 1d-array (list) version of calc_jacobian.
    )";

    const char* calc_out_counts_ = R"(
        calc_out_counts:

        Valid settings: True | False

        Setting this to True will calculate a non-compact vector of count results when session.run() is evoked. The single setting applies globally.
        Note that calc_out_counts = True is implied by calc_jacobian = True.

        calc_out_countss:

        Valid settings: [[True|False, ...], [True|False, ...]]

        A 1d-array (list) version of calc_out_counts.
    )";

    const char* noplacements_ = R"(
        noplacement:

        Valid settings: True | False

        Setting this to True to disable circuit placement. The single setting applies globally.

        noplacements:

        Valid settings: [[True|False, ...], [True|False, ...]]

        A 1d-array (list) version of noplacement.
    )";

    const char *placements_ = R"(
        placement:

        Valid settings: "swap-shortest-path" | "noise-aware"

        Setting the method to map from logical qubits to the physical qubits of the device that will be used to carry them. The single setting applies globally to all infiles and all instrings.
        Default: "swap-shortest-path"

        placements:

        Valid settings: [["swap-shortest-path" | "noise-aware", ...], ["swap-shortest-path" | "noise-aware", ...]]

        The lead dimension's element 0 matches the vector of infiles, element 1 matches the vector of instrings, element 2 matches the vector of randoms.
    )";

    const char *circuit_opts_ = R"(
        circuit_optimization:

        List of circuit optimization passes.

        circuit_optimizations:

        A 1d-array (list) version of circuit_optimization.
    )";

    const char* nooptimises_ = R"(
        nooptimise:

        Valid settings: True | False

        Setting this to True to disable circuit optimization. The single setting applies globally.

        nooptimises:

        Valid settings: [[True|False, ...], [True|False, ...]]

        A 1d-array (list) version of nooptimise.
    )";

    const char* execute_circuits_ = R"(
        execute_circuit:

        Valid settings: True | False

        Set this to False to disable circuit simulation. The single setting applies globally.

        execute_circuits:

        Valid settings: [[True|False, ...], [True|False, ...]]

        A 1d-array (list) version of execute_circuit.
    )";

    const char* noises_ = R"(
        noise:

        Valid settings: True | False

        Setting this to True to enable noisy simulation (if supported by the `acc` backend). The single setting applies globally.

        noises:

        Valid settings: [[True|False, ...], [True|False, ...]]

        A 1d-array (list) version of noise.
    )";
    const char* state_vec_ = R"(
        in_get_state_vec:

        Using this will extract the state vector (supported by the `qpp` backend only).
    )";
    const char* notimings_ = R"(
        .. warning::
                This property is currently unused.

        notiming:

        Valid settings: True | False

        Setting this to True to disable timing estimation. The single setting applies globally.

        notimings:

        Valid settings: [[True|False, ...], [True|False, ...]]

        A 1d-array (list) version of notiming.
    )";
    const char* output_oqm_enableds_ = R"(
        output_oqm_enabled:

        Valid settings: True | False

        Setting this to True to enable circuit timing and resource estimation. The single setting applies globally.

        output_oqm_enableds:

        Valid settings: [[True|False, ...], [True|False, ...]]

        A 1d-array (list) version of output_oqm_enabled.
    )";

    const char* qns_ = R"(
        qn:

        Number of qubits. The single setting applies globally.

        qns:

        A 1d-array (list) version of qn.
    )";

    const char* sns_ = R"(
        sn:

        Number of measurement shots. The single setting applies globally.

        sns:

        A 1d-array (list) version of sn.
    )";

    const char* randoms_ = R"(
        random:

        Circuit depth of the random circuit to use as input. The single setting applies globally.

        randoms:

        A 1d-array (list) version of random.
    )";

    const char* parameter_vectors_ = R"(
        parameter_vector:

        Runtime parameters for the input parametrized circuit. The single setting applies globally.

        parameter_vectors:

        A 1d-array (list) version of parameter_vector.
    )";

    const char* initial_bond_dimensions_ = R"(
        initial_bond_dimension:

        Set the initial bond dimension (tensor network simulators). The single setting applies globally.

        initial_bond_dimensions:

        A 1d-array (list) version of initial_bond_dimension.

        .. note::
                This is only needed if using the tensor network backend accelerators.
    )";

    const char* initial_kraus_dimensions_ = R"(
        initial_kraus_dimension:

        Set the initial kraus dimension (emulator purification simulator). The single setting applies globally.

        initial_kraus_dimensions:

        A 1d-array (list) version of initial_kraus_dimension.

        .. note::
                This is only needed if using the emulator purification backend accelerator.
    )";

    const char* max_bond_dimensions_ = R"(
        max_bond_dimension:

        Set the maximum bond dimension (tensor network simulators). The single setting applies globally.

        max_bond_dimensions:

        A 1d-array (list) version of max_bond_dimension.

        .. note::
                This is only needed if using the tensor network backend accelerators.
    )";

    const char* max_kraus_dimensions_ = R"(
        max_kraus_dimension:

        Set the maximum kraus dimension (emulator purification simulator). The single setting applies globally.

        max_kraus_dimensions:

        A 1d-array (list) version of max_kraus_dimension.

        .. note::
                This is only needed if using the emulator purification backend accelerator.
    )";

    const char* svd_cutoffs_ = R"(
        svd_cutoff:

        Set the SVD cutoff threshold value (tensor network simulators). The single setting applies globally.

        svd_cutoffs:

        A 1d-array (list) version of svd_cutoff.

        .. note::
                This is only needed if using the tensor network backend accelerators.
    )";

    const char* rel_svd_cutoffs_ = R"(
        rel_svd_cutoff:

        Set the relative SVD cutoff threshold value (tensor network simulators). The single setting applies globally.

        rel_svd_cutoffs:

        A 1d-array (list) version of rel_svd_cutoff.

        .. note::
                This is only needed if using the tensor network backend accelerators.
    )";

    const char* measure_sample_methods_ = R"(
        measure_sample_method:

        Set the measurement sampling method (QB emulator tensor network simulator). The single setting applies globally.

        measure_sample_methods:

        A 1d-array (list) version of measure_sample_method.

        .. note::
                This is only needed if using the emulator tensor network accelerators.
    )";

    const char* noise_models_ = R"(
        noise_model:

        Set the noise model to be used in subsequent simulations.

        .. note::
                Requires setting: noise = True (to have effect)

        The default (built in) model is a simple depolarizing noise model on all qubits.

        Users may make their own instances of the NoiseModel class (or make an instance of the
        default and modify it), and then assign that model to this property.  See examples/python/noise_model*.py.

        If the Qristal Emulator is installed, the following additional models are available and
        can be accessed by specifying the relevant model name as a string passed to the constructor
        of the NoiseModel class:

        "qb-nm1" : 4x4 NV centres in x-y grid, 3 qubits per NV centre

        "qb-nm2" : 8x8 NV centres in x-y grid, 1 qubit per NV centre, nearest x and nearest y connectivity

        "qb-nm3" : 48 NV centres in a 1-dimensional grid with linear qubit index, 1 qubits per NV centre

        "qb-qdk1" : 1 NV centre with 2 qubits, fidelities tuned to match deployed device

        "qb-dqc2" : 1 NV centre with 2 qubits, fidelities tuned to match lab-based device

        noise_models:

        A 1d-array (list) version of noise_model.

    )";

    const char* expected_amplitudes_ = R"(
        expected_amplitudes:

        Set the expected amplitudes for Jensen-Shannon divergence calculation. The single setting applies globally.

        amp:

        A 1d-array (list) version of expected_amplitudes.
    )";

    const char* results_ = R"(
        results:

        After calling session.run(), the counts from running sn shots are stored in a dictionary of shot counts. The dictionary keys are bit lists of length qn, with the same bit indexation as the processor registers.
    )";

    const char* results_native_ = R"(
        results_native:

        An additional results container holding the native measurement results if automatic SPAM correction was enabled in session.
    )";

    const char* out_counts_ = R"(
        out_counts:

        After calling session.run(), the counts from running sn shots are stored in a list of shot counts.
        The indices of this list correspond to the different possible base-2 bitstring solutions, with the mapping from bitstring to list index provided by the function bitstring_index.
    )";

    const char* out_probs_ = R"(
        out_probs:

        After calling session.run() with `calc_jacobian(s)` set to true, the probability distribution from running sn shots stored in a list of solution output probabilities.
        The indices of this list correspond to the different possible base-2 bitstring solutions, with the mapping from bitstring to list index provided by the function bitstring_index.
    )";

    const char* out_jacobians_ = R"(
        out_prob_jacobians:

        After calling session.run() with `calc_jacobian(s)` set to true, the probability jacobians from running sn shots are stored in session.out_prob_jacobian, using a list of 2D list-of-lists format.
        The jacobians calculate the gradients of the probability with respect to the runtime parameters, in the following format (where y is the probability list and x is the parameter list):

        .. math::
            \begin{bmatrix}
            \frac{dy_0}{dx_0} & \frac{dy_0}{dx_1} & ... & \frac{dy_0}{dx_n} \\
            \frac{dy_1}{dx_0} & \frac{dy_1}{dx_1} & ... & \frac{dy_1}{dx_n} \\
            ... \\
            \frac{dy_m}{dx_0} & \frac{dy_m}{dx_1} & ... & \frac{dy_m}{dx_n}
            \end{bmatrix}

        Since the jacobian is returned as a list-of-lists, it can be accessed in row major format, and indexing the above matrix can be done accordingly, i.e. get_out_jacobians()[0][1] corresponds to the dy_0/dx_1 value.
        x_i correspond to the parameters set in the parameter list (i.e. the parameters ordered by their first appearance in the circuit.)
        y_i are the output probabilities of different bitstrings, indexed in the same manner as the out_count list. Explicitly, the index i corresponding to a specific bitstring can be obtained by calling bitstring_index(bitstring), with bitstring given as a list of bit values.
    )";

    const char* out_divergences_ = R"(
        out_divergence:

        After calling session.divergence(), the Jensen-Shannon divergence between session.results and session.expected_amplitudes is calculated and stored in session.out_divergence.

        out_divergences:

        A 1d-array (list) version of session.out_divergence.
    )";

    const char* out_transpiled_circuits_ = R"(
        out_transpiled_circuit:

        After calling session.run(), the transpiled version of session.instring is stored as session.out_transpiled_circuit.

        out_transpiled_circuits:

        A 1d-array (list) version of session.out_transpiled_circuit.
    )";

    const char* out_qobjs_ = R"(
        out_qobj:

        When acc='aer', the .qobj JSON input used by a standalone Aer installation is stored in out_qobj.  Note: session.run() must be called first.

        out_qobjs:

        A 1d-array (list) version of session.out_qobj.
    )";

    const char* out_qbjsons_ = R"(
        out_qbjson:

        Shows JSON data sent to QB hardware.  Note: session.run() must be called first.

        out_qbjsons:

        A 1d-array (list) version of session.out_qbjson.
    )";

    const char* out_single_qubit_gate_qtys_ = R"(
        out_single_qubit_gate_qty:

        After calling session.profile(), the circuit in session.out_transpiled_circuit is processed and the count of the number of single qubit gates is stored as session.out_single_qubit_gate_qty, using a dictionary where the keys are integers corresponding to qubit indexes.

        out_single_qubit_gate_qtys:

        A 1d-array (list) version of session.out_single_qubit_gate_qty.
    )";

    const char* out_double_qubit_gate_qtys_ = R"(
        out_double_qubit_gate_qty:

        After calling session.profile(), the circuit in session.out_transpiled_circuit is processed and the count of the number of two-qubit gates is stored as session.out_double_qubit_gate_qty, using a dictionary where the keys are integers corresponding to qubit indexes.

        out_double_qubit_gate_qtys:

        A 1d-array (list) version of session.out_double_qubit_gate_qty.
    )";

    const char* out_total_init_maxgate_readout_times_ = R"(
        out_total_init_maxgate_readout_time:

        After calling session.profile(), the circuit in in session.out_transpiled_circuit is processed and timing estimates taken to perform the required number of shots [sn] are stored as session.out_total_init_maxgate_readout_time.

        It uses a dictionary with the following keys [integer]:

                0: total, in ms;

                1: initialisation time component, in ms;

                2: max depth gate time component, in ms;

                3: readout time component, in ms;

                4: total (classically simulated time), in ms;

                5: PC transfer to controller time, in ms.


        out_total_init_maxgate_readout_times:

        A 1d-array (list) version of session.out_total_init_maxgate_readout_time.
    )";

    const char* out_z_op_expects_ = R"(
        out_z_op_expect:

        After calling run(), the Z-operator expectation value determined from counts in respective states is stored in out_z_op_expects, using a dictionary where the keys are integers, and currently only key:0 is used.

        out_z_op_expects:

        A 1d-array (list) version of out_z_op_expect.
    )";

    const char* debug_ = R"(
        debug:

        Valid settings: True | False

        When set to True, extra debugging information will be printed.

    )";

    const char* noise_mitigations_ = R"(
        noise_mitigation:

        Select a noise mitigation module.

        noise_mitigations:

        A 1d-array (list) version of noise_mitigation.
    )";

    const char* SPAM_confusion_ = R"(
        SPAM_confusion:

        Set a state preparation and measurement (SPAM) confusion matrix, which will be used to correct the measured results.
        After setting, and calling `run()`, the results variable will automatically be populated with SPAM-corrected counts.
        The native counts will be stored in `results_native` instead. Calling `SPAM_confusion` after setting, will return
        the used SPAM correction matrix, i.e., the inverse of the passed confusion matrix.
    )";

    const char* seeds_ = R"(
        seed:

        Set the random seed value.

        seeds:

        A 1d-array (list) version of seed.
    )";

    const char* bitstring_index_ = R"(
        Get the (base-10) integer index for the counts/probabilities list, corresponding to a specific output state specified by a list of bit values.
    )";

  }

}

