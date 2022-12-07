// Copyright (c) 2021 Quantum Brilliance Pty Ltd

#include "qb/core/session.hpp"

namespace qb {
/// Help strings for Python bindings
const char* session::help_infiles_ = R"(
        infile:

        Name of a file containing (default: OpenQASM format) a quantum circuit.

        infiles:

        Valid settings: [infile, ...]

        A 1d-array (list) version of session.infile.
)";


const char* session::help_irtarget_ms_ = R"(
        (C++ only) - directly specify a XACC IR
)";

const char* session::help_instrings_ = R"(
        instring:

        A string that defines a circuit in OpenQASM format.  Simple example:
        
        .. code-block::

                __qpu__ void QBCIRCUIT(qreg q) {
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

const char* session::help_include_qbs_ = R"(
        include_qb:

        A file that contains OpenQASM format gate definitions for custom Quantum Brilliance gates.

        include_qbs:

        Valid settings: [include_qb, ...]

        A 1d-array (list) version of session.include_qb.
)";

const char* session::help_qpu_configs_ = R"(
        qpu_config:

        A JSON file that contains configuration data for Quantum Brilliance hardware.

        qpu_configs:

        Valid settings: [qpu_config, ...]

        A 1d-array (list) version of session.qpu_config.
)";

const char* session::help_accs_ = R"(
        acc:

        Valid settings: aer | tnqvm

        Select a back-end simulator. The single setting applies globally to all infiles, all instrings, and random circuits.

        accs:

        Valid settings: [[aer|tnqvm, ...], [aer|tnqvm, ...]]

        The lead dimension's element 0 matches the vector of infiles, element 1 matches the vector of instrings, element 2 matches the vector of random depths.
)";

const char* session::help_aws_device_names_ = R"(
        aws_device:

        Valid settings: SV1 | TN1 | DM1 | Rigetti | LocalSimulator

        Selects an AWS Braket back-end simulator or QPU.

        aws_devices:

        A 1d-array (list) version of aws_device.
)";

const char* session::help_aws_formats_ = R"(
        aws_format:

        Valid settings: braket | openqasm3

        Selects an AWS Braket back-end simulator or QPU language.

        aws_formats:

        A 1d-array (list) version of aws_device.
)";

const char* session::help_aws_s3s_ = R"(
        aws_s3:

        Valid settings: amazon-braket-*

        Specifies an AWS S3 bucket in which to store outputs from AWS Braket.
        Must begin with the prefix: "amazon-braket-"

        aws_s3s:

        A 1d-array (list) version of aws_s3.
)";

const char* session::help_aws_s3_paths_ = R"(
        aws_s3_path:

        Valid settings: see AWS documentation for S3 path name conventions.

        Specifies a path (inside the S3 bucket specified by aws_s3) to store outputs from AWS Braket.

        aws_s3_paths:

        A 1d-array (list) version of aws_s3_path.
)";

const char* session::help_aws_verbatims_ = R"(
        aws_verbatim:

        Valid settings: true | false

        Selects whether verbatim mode is used on AWS Braket hardware QPU (currently Rigetti).

        aws_verbatims:

        A 1d-array (list) version of aws_verbatim flag.
)";

const char* session::help_aer_sim_types_ = R"(
        aer_sim_type:

        Valid settings: statevector | density_matrix | matrix_product_state

        Selects a simulation method for the AER simulator.

        aer_sim_types:

        A 1d-array (list) version of aer_sim_type.
)";

const char* session::help_xasms_ = R"(
        xasm:

        Valid settings: True | False

        Setting this to True causes circuits to be interpreted in XASM format.  The single setting applies globally to all infiles and all instrings.

        xasms:

        Valid settings: [[True|False, ...], [True|False, ...]]

        The lead dimension's element 0 matches the vector of infiles, element 1 matches the vector of instrings, element 2 matches the vector of randoms.
)";

const char* session::help_quil1s_ = R"(
        quil1:

        Valid settings: True | False

        Setting this to True causes circuits to be interpreted in Quil 1.0 format. The single setting applies globally to all infiles and all instrings.

        quil1s:

        Valid settings: [[True|False, ...], [True|False, ...]]

        The lead dimension's element 0 matches the vector of infiles, element 1 matches the vector of instrings, element 2 matches the vector of randoms.
)";

const char* session::help_noplacements_ = R"(
        noplacement:

        noplacements:

)";

const char *session::help_placements_ = R"(
        placement:

        Valid settings: "swap-shortest-path" | "tket"

        Setting the method to map from logical qubits to the physical qubits of the device that will be used to carry them. The single setting applies globally to all infiles and all instrings.
        Default: "swap-shortest-path"

        placements:

        Valid settings: [["swap-shortest-path" | "tket", ...], ["swap-shortest-path" | "tket", ...]]

        The lead dimension's element 0 matches the vector of infiles, element 1 matches the vector of instrings, element 2 matches the vector of randoms.
)";

const char* session::help_nooptimises_ = R"(
        nooptimise:

        nooptimises:

)";

const char* session::help_nosims_ = R"(
        nosim:

        nosims:

)";

const char* session::help_noises_ = R"(
        noise:

        noises:

)";
const char* session::help_notimings_ = R"(
        notiming:

        notimings:

)";
const char* session::help_output_oqm_enableds_ = R"(
        output_oqm_enabled:

        output_oqm_enableds:

)";
const char* session::help_log_enableds_ = R"(
        log_enabled:

        log_enableds:

)";

const char* session::help_qns_ = R"(
        qn:

        qns:

)";

const char* session::help_rns_ = R"(
        rn:

        rns:

)";

const char* session::help_sns_ = R"(
        sn:

        sns:

)";

const char* session::help_randoms_ = R"(
        random:

        randoms:

)";

const char* session::help_betas_ = R"(
        beta:

        betas:

)";

const char* session::help_thetas_ = R"(
        theta:

        thetas:

)";

const char* session::help_max_bond_dimensions_ = R"(
        max_bond_dimension:

        max_bond_dimensions:

)";

const char* session::help_svd_cutoffs_ = R"(
        svd_cutoff:

        svd_cutoffs:

)";

const char* session::help_noise_models_ = R"(
        noise_model:

        Valid settings: "default" | "qb-nm1" | "qb-nm2"

        Requires setting: noise = True (to have effect)
        Default: "default"
        "default": Simple depolarizing noise model on all qubits (builtin)
        These two options require the QB emulator module (addons):
        "qb-nm1" : 4x4 NV centres in x-y grid, 3 qubits per NV centre
        "qb-nm2" : 8x8 NV centres in x-y grid, 1 qubit per NV centre, nearest x and nearest y connectivity

        noise_models:

        A 1d-array (list) version of noise_model.

)";
const char* session::help_output_amplitudes_ = R"(
        output_amplitude:

        output_amplitudes:

)";

const char* session::help_out_raws_ = R"(
        out_raw:

        After calling session.run(), the counts from running sn shots are stored in session.out_raw, using a JSON format.

        out_raws:

        A 1d-array (list) version of session.out_raw.
)";

const char* session::help_out_counts_ = R"(
        out_count:

        After calling session.run(), the counts from running sn shots are stored in session.out_count, using a dictionary where the keys are state label bits interpreted as BCD (MSB format).

        out_counts:

        A 1d-array (list) version of session.out_count.
)";

const char* session::help_out_divergences_ = R"(
        out_divergence:

        After calling session.divergence(), the Jensen-Shannon divergence between session.out_count and session.output_amplitude is calculated and stored session.out_divergence.

        out_divergences:

        A 1d-array (list) version of session.out_divergence.
)";

const char* session::help_out_transpiled_circuits_ = R"(
        out_transpiled_circuit:

        After calling session.run(), the transpiled version of session.instring is stored as session.out_transpiled_circuit.

        out_transpiled_circuits:

        A 1d-array (list) version of session.out_transpiled_circuit.
)";

const char* session::help_out_qobjs_ = R"(
        out_qobj:

        When acc='aer', the .qobj JSON input used by a standalone Aer installation is stored in out_qobj.  Note: session.run() must be called first.

        out_qobjs:

        A 1d-array (list) version of session.out_qobj.
)";

const char* session::help_out_qbjsons_ = R"(
        out_qbjson:

        Shows JSON data sent to QB hardware.  Note: session.run() must be called first.

        out_qbjsons:

        A 1d-array (list) version of session.out_qbjson.
)";

const char* session::help_out_single_qubit_gate_qtys_ = R"(
        out_single_qubit_gate_qty:

        After calling session.profile(), the circuit in session.out_transpiled_circuit is processed and the count of the number of single qubit gates is stored as session.out_single_qubit_gate_qty, using a dictionary where the keys are integers corresponding to qubit indexes.

        out_single_qubit_gate_qtys:

        A 1d-array (list) version of session.out_single_qubit_gate_qty.
)";

const char* session::help_out_double_qubit_gate_qtys_ = R"(
        out_double_qubit_gate_qty:

        After calling session.profile(), the circuit in session.out_transpiled_circuit is processed and the count of the number of two-qubit gates is stored as session.out_double_qubit_gate_qty, using a dictionary where the keys are integers corresponding to qubit indexes.

        out_double_qubit_gate_qtys:

        A 1d-array (list) version of session.out_double_qubit_gate_qty.
)";

const char* session::help_out_total_init_maxgate_readout_times_ = R"(
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

const char* session::help_out_z_op_expects_ = R"(
        out_z_op_expect:

        After calling run(), the Z-operator expectation value determined from counts in respective states is stored in out_z_op_expects, using a dictionary where the keys are integers, and currently only key:0 is used.

        out_z_op_expects:

        A 1d-array (list) version of out_z_op_expect.
)";

const char* session::help_debug_ = R"(
        debug:

        Valid settings: True | False

        When set to True, extra debugging information will be printed.

)";

const char* session::help_noise_mitigations_ = R"(
        noise_mitigation:

        Select a noise mitigation module.

        noise_mitigations:

        A 1d-array (list) version of noise_mitigation.
)";

const char* session::help_seeds_ = R"(
        seed:

        Set the random seed value.

        seeds:

        A 1d-array (list) version of seed.
)";
} // namespace qb