# qbos_core - Circuit simulation with the timing, noise and topology parameters of QB hardware

`qbos_core` is a command-line executable that carries over the previous functionality of `qbqe`, **with the exception of VQE and QAOA** functionality.  VQE functionality is now provided by a new `qbos_op_vqe` standalone executable.  Similarly, QAOA functionality is provided by a new `qbos_op_qaoa` standalone executable.

```bash
  qbos_core [input-circuit-file] {OPTIONS}

    qbos_core - Circuit simulation with the timing, noise and topology parameters of
    QB hardware. This tool is a component of qbOS. The configuration of this
    tool is set in a configuration file named "qbos_cfg.json". Note: command-line options
    specified here will override that of the configuration file.

  OPTIONS:

      input-circuit-file                Name of file containing a circuit
      -h, --help                        Display this help menu
      -v, --verbose                     Display additional placement and circuit
                                        optimisation info
      * General options
        -q[#qubits]                       -q10 accepts up to 10 qubits, default:
                                          12 (qbos can currently support up to
                                          maximum 48 qubits. All qubits on a QB
                                          chip are operationally connected.
                                          However, the clustered arrangement of
                                          the qubits means that no more than six
                                          may be physically fully connected,
                                          while clusters have nearest-neighbour
                                          connections.)
        -s[#shots]                        -s128 gives 128 shots, default: 1024
      * Switches
        -n, --noise                       Enable QB noise model, a simulation of
                                          noise sources within the QB hardware
                                          and their effect on results. The noise
                                          has three main sources, internal
                                          thermal and magnetic fluctuations, and
                                          also fluctuations in the control
                                          mechanism. The inputs for the
                                          noise-model are already hard-coded
                                          with realistic parameters. Currently,
                                          the noise-model can only work
                                          alongside "--acc=aer" option
        --noplacement                     Disable placement mapping
        --optimise                        Enable circuit optimiser
        --nosim                           Skip simulation
      * Random circuit options
        --random=[#random_circuit_depth]  --random=20 will sample and analyse
                                          quantum random circuits of [#qubits]
                                          and depth 20 at each repetition
      * Test threshold options
        --threshold=[cutoff]              --threshold=0.15 sets 0.15 as critical
                                          value for Jensen-Shannon divergence,
                                          default: 0.05
      * Developer/test use only
        --test                            Run unit tests
        --svd-cutoff=[SVD cutoff]         --svd-cutoff=1.0e-12 sets the cutoff
                                          for exatn-mps to 1.0e-12, default:
                                          1.0e-8
        --max-bond-dimension=[max bond
        dimension]                        --max-bond-dimension=2000 sets the
                                          maximum bond dimenson for exatn-mps to
                                          2000, default: 256
        --acc=[acc]                       --acc='aer' or --acc='qpp' to select
                                          back-end simulators, default: qpp
        -x, --xasm                        Interpret input in XASM format,
                                          default input is OpenQASM
        --quil1                           Interpret input in QUIL 1.0 format
      "--" can be used to terminate flag options and force all following
      arguments to be treated as positional options

    Version: --- Build time 2021-08-24T01:03:34
    
```