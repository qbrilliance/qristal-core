// Copyright (c) Quantum Brilliance Pty Ltd

// Help strings for Python bindings
namespace qristal {
  namespace help {

    const char* infile = R"(
        infile:

        Name of a file containing (default: OpenQASM format) a quantum circuit.
    )";


    const char* irtarget = R"(
        irtarget:

        Input quantum circuit (core.Circuit).
    )";

    const char* instring = R"(
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
    )";

    const char* include_qb = R"(
        include_qb:

        A file that contains OpenQASM format gate definitions for custom Quantum Brilliance gates.
    )";

    const char* remote_backend_database_path = R"(
        remote_backend_database_path:

        A YAML file that contains configuration data for remote backends (including hardware).
    )";

    const char* acc = R"(
        acc:

        The selected circuit executor backend (simulator or hardware).
    )";

    const char* aer_sim_type = R"(
        aer_sim_type:

        Valid settings: statevector | density_matrix | matrix_product_state

        Selects a simulation method for the AER simulator.
    )";

    const char* aer_omp_threads = R"(
        aer_omp_threads:

        Valid settings: any positive integer

        Sets the number of threads allowed to be used by the AER simulator.
        Defaults to the value of the environment variable OMP_NUM_THREADS; if that is empty, thread number is chosen by XACC & AER.
    )";

    const char* calc_gradients = R"(
        calc_gradients:

        Valid settings: True | False

        Setting this True will calculate gradients for the circuit when session.run() is invoked.  The gradients calculated will be a jacobian of the
        output probabilities with respect to the input parameters, i.e. d_probs/d_params.  Note that calc_gradients = True implies calc_all_bitstring_counts = True.
    )";

    const char* calc_all_bitstring_counts = R"(
        calc_all_bitstring_counts:

        Valid settings: True | False

        Setting this True will calculate a non-compact vector of count results when session.run() is invoked.
        Note that calc_all_bitstring_counts = True is implied by calc_gradients = True.
    )";

    const char* noplacement = R"(
        noplacement:

        Valid settings: True | False

        Setting this True disables circuit placement.
    )";

    const char* placement = R"(
        placement:

        Valid settings: "swap-shortest-path" | "noise-aware"

        Setting the method to map from logical qubits to the physical qubits of the device that will be used to carry them.
        Default: "swap-shortest-path"
    )";

    const char* circuit_opts = R"(
        circuit_opts:

        List of circuit optimization passes.
    )";

    const char* nooptimise = R"(
        nooptimise:

        Valid settings: True | False

        Setting this True disables circuit optimization.
    )";

    const char* execute_circuit = R"(
        execute_circuit:

        Valid settings: True | False

        Set this False to disable circuit simulation.
    )";

    const char* noise = R"(
        noise:

        Valid settings: True | False

        Setting this True enables noisy simulation (if supported by the backend selected via `acc`).
    )";

    const char* calc_state_vec = R"(
        calc_state_vec:

        Valid settings: True | False

        Set True to extract the state vector (supported by the `qpp` and `aer` statevector backends only).
    )";

    const char* state_vec = R"(
        state_vec:

        Get the full complex state vector -- works with qpp and aer statevector backends only!
    )";

    const char* notiming = R"(
        .. warning::
                This property is currently unused.

        notiming:

        Valid settings: True | False

        Setting this True disables timing estimation.
    )";

    const char* output_oqm_enabled = R"(
        output_oqm_enabled:

        Valid settings: True | False

        Setting this True enables circuit timing and resource estimation.
    )";

    const char* qn = R"(
        qn:

        Number of qubits.
    )";

    const char* sn = R"(
        sn:

        Number of measurement shots.
    )";

    const char* random_circuit_depth = R"(
        random_circuit_depth:

        Circuit depth of the random circuit to generate.
    )";

    const char* input_language = R"(
        input_language:

        The frontend language in which the input circuit is written.
    )";

    const char* circuit_parameters = R"(
        circuit_parameters:

        Runtime parameters for the input parametrized circuit.
    )";

    const char* initial_bond_dimension = R"(
        initial_bond_dimension:

        Set the initial bond dimension (tensor network simulators).

        .. note::
                This is only needed if using the tensor network backend accelerators.
    )";

    const char* initial_kraus_dimension = R"(
        initial_kraus_dimension:

        Set the initial kraus dimension (emulator purification simulator).

        .. note::
                This is only needed if using the emulator purification backend accelerator.
    )";

    const char* max_bond_dimension = R"(
        max_bond_dimension:

        Set the maximum bond dimension (tensor network simulators).

        .. note::
                This is only needed if using the tensor network backend accelerators.
    )";

    const char* max_kraus_dimension = R"(
        max_kraus_dimension:

        Set the maximum kraus dimension (emulator purification simulator).

        .. note::
                This is only needed if using the emulator purification backend accelerator.
    )";

    const char* svd_cutoff = R"(
        svd_cutoff:

        Set the SVD cutoff threshold value (tensor network simulators).

        .. note::
                This is only needed if using the tensor network backend accelerators.
    )";

    const char* rel_svd_cutoff = R"(
        rel_svd_cutoff:

        Set the relative SVD cutoff threshold value (tensor network simulators).

        .. note::
                This is only needed if using the tensor network backend accelerators.
    )";

    const char* measure_sample_method = R"(
        measure_sample_method:

        Set the measurement sampling method (QB emulator tensor network simulator).
    )";

    const char* gpu_device_ids = R"(
        gpu_device_ids:

        Set list of GPU device IDs to use with GPU-enabled backends.
    )";

    const char* svd_type = R"(
        svd_type:

        Set the singular value decomposition method (QB emulator tensor network simulator).
    )";

    const char* svdj_tol = R"(
        svdj_tol:

        Set the Jacobian singular value decomposition accuracy tolerance (QB emulator tensor network simulator).
    )";

    const char* svdj_max_sweeps = R"(
        svdj_max_sweeps:

        Set the Jacobian singular value decomposition maximum iteration sweeps (QB emulator tensor network simulator).
    )";

    const char* noise_model = R"(
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
    )";

    const char* results = R"(
        results:

        After calling session.run(), the counts from running sn shots are stored in a dictionary of shot counts. The dictionary keys are bit lists of length qn, with the same bit indexation as the processor registers.
    )";

    const char* results_native = R"(
        results_native:

        An additional results container holding the native measurement results if automatic SPAM correction was enabled in session.
    )";

    const char* all_bitstring_counts = R"(
        all_bitstring_counts:

        After calling session.run(), the counts from running sn shots are stored in a list of shot counts.
        The indices of this list correspond to the different possible base-2 bitstring solutions, with the mapping from bitstring to list index provided by the function bitstring_index.
    )";

    const char* all_bitstring_probabilities = R"(
        all_bitstring_probabilities:

        The probability distribution from running sn shots stored in a list of solution output probabilities, provided after calling session.run() with `calc_gradients` set True.
        The indices of this list correspond to the different possible base-2 bitstring solutions, with the mapping from bitstring to list index provided by the function bitstring_index.
    )";

    const char* all_bitstring_probability_gradients = R"(
        all_bitstring_probability_gradients:

        The probability Jacobians from running sn shots, after calling session.run() with `calc_gradients` set True.
        The Jacobian is an array of gradients of the probability of each bitstring combination with respect to the runtime parameters,
        in the following format (where y is the list of probabilities and x is the parameter list):

        .. math::
            \begin{bmatrix}
            \frac{dy_0}{dx_0} & \frac{dy_0}{dx_1} & ... & \frac{dy_0}{dx_n} \\
            \frac{dy_1}{dx_0} & \frac{dy_1}{dx_1} & ... & \frac{dy_1}{dx_n} \\
            ... \\
            \frac{dy_m}{dx_0} & \frac{dy_m}{dx_1} & ... & \frac{dy_m}{dx_n}
            \end{bmatrix}

        As the Jacobian is returned as a list-of-lists, it can be accessed in row major format, and indexing the above matrix can be done accordingly, i.e. all_bitstring_probability_gradients()[0][1] corresponds to the dy_0/dx_1 value.
        The entry x_i corresponds to the ith parameter in the parameter list (i.e. the parameters ordered by their first appearance in the circuit.)
        The entry y_i is the output probability of ith bitstring, indexed in the same manner as the all_bitstring_counts. Explicitly, the index i corresponding to a specific bitstring can be obtained by calling bitstring_index(bitstring), with bitstring given as a list of bit values.
    )";

    const char* transpiled_circuit = R"(
        transpiled_circuit:

        Retrieve the transpiled version of the executed circuit after calling session.run().
    )";

    const char* qobj = R"(
        qobj:

        Retrieve the input passed to the aer simulator backend in .qobj JSON format, after calling session.run().
    )";

    const char* qbjson = R"(
        qbjson:

        Get the output QB JSON string sent to QB hardware, after calling session.run().
    )";

    const char* one_qubit_gate_depths = R"(
        one_qubit_gate_depths:

        After calling session.run(), get the number of single qubit gates applied to each qubit, using a dictionary where the keys are integers corresponding to qubit indices.
    )";

    const char* two_qubit_gate_depths = R"(
        two_qubit_gate_depths:

        After calling session.run(), get the number of two qubit gates applied to each qubit, using a dictionary where the keys are integers corresponding to qubit indices.
    )";

    const char* timing_estimates = R"(
        timing_estimates:

        After calling session.run(), get estimated circuit execution times on hardware, in ms.

        Keys (integers):
           0: Total time
           1: Initialisation component
           2: Gate (max. depth) component
           3: Readout component
           4: Total time (from classical simulation)
           5: PC transfer to controller time
    )";

    const char* z_op_expectation = R"(
        z_op_expectation:

        After calling session.run(), get the output expected value in the Z basis, from the shot counts observed.
    )";

    const char* debug = R"(
        debug:

        Valid settings: True | False

        When set True, extra debugging information will be printed.
    )";

    const char* noise_mitigation = R"(
        noise_mitigation:

        Select a noise mitigation module.
    )";

    const char* SPAM_confusion = R"(
        SPAM_confusion:

        Set a state preparation and measurement (SPAM) confusion matrix, which will be used to correct the measured results.
        After setting, and calling `run()`, the results variable will automatically be populated with SPAM-corrected counts.
        The native counts will be stored in `results_native` instead. Calling `SPAM_confusion` after setting, will return
        the used SPAM correction matrix, i.e., the inverse of the passed confusion matrix.
    )";

    const char* seed = R"(
        seed:

        The random seed value.
    )";

    const char* bitstring_index = R"(
        Get the (base-10) integer index for the counts/probabilities list, corresponding to a specific output state specified by a list of bit values.
    )";

  }

}

