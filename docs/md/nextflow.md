# Nextflow

[Nextflow](https://www.nextflow.io/index.html) is the preferred workflow framework in Qristal.  The typical use case is a large, multi-stage HPC application that has sequential stages (best executed within an interactive session); followed by stages that are parallel (best executed as independent job submissions to the HPC scheduler).

Qristal's class structure is well suited to producing command line executable programs that utilize MPI for parallelization.  We then use Nextflow to stitch together multiple MPI-enabled executables into a main application workflow.  For hybrid quantum-classical applications, each process of the MPI executable has functions that run either on a simulated quantum backend, or an actual QPU.

## Installing Nextflow

### Prerequisites

**Java Runtime Environment**
```
sudo apt install openjdk-17-jre
```

### Downloading Nextflow
```
curl -s https://get.nextflow.io | bash
```

## Structure of a Nextflow project
The root directory of a project will contain these files:

### nextflow.config
An example of this file is shown below:

    profiles {
        standard {
            process.executor = 'local'
            process.cpus = 4
            process.memory = '2 GB'
        }
        cluster {
            process.executor = 'slurm'
            process.queue = 'work'
            process.cpus = 16
            process.memory = '16 GB'
        }
        qdk {
            process.executor = 'local'
            process.cpus = 1
            process.memory = '2 GB'
        }
        aws {
            process.cpus = 1
            process.memory = '2 GB'
        }
    }

### main.nf
This is the entrypoint script that drives the Nextflow execution.  Below is an example of a `main.nf` that sweeps over 5 settings for SVD cutoff (used by the MPS method):

    #!/usr/bin/env nextflow
    nextflow.enable.dsl=2

    process sweepSvdCutoff {
        debug false
        input:
            val svdcutoff

        output:
            stdout

        """
        #!/usr/bin/python3
        import qristal.core
        import numpy as np

        my_sim = qristal.core.session()
        my_sim.init()
        my_sim.qn = 2     # Number of qubits
        my_sim.sn = 1024  # Number of shots
        my_sim.acc = "tnqvm"
        my_sim.noplacement = True
        my_sim.nooptimise = True
        my_sim.notiming = True
        my_sim.output_oqm_enabled = False
        my_sim.svd_cutoff[0][0][0] = ${svdcutoff}
        my_sim.instring = '''

        __qpu__ void qristal_circuit(qreg q) {
        OPENQASM 2.0;
        include "qelib1.inc";
        creg c[2];
        h q[0];
        cx q[0],q[1];
        measure q[1] -> c[1];
        measure q[0] -> c[0];
        }

        '''
        my_sim.run()
        print("SVD cutoff: ", my_sim.svd_cutoff[0][0][0])
        print(my_sim.results[0][0])
        """
    }

    workflow {
      channel.of(0.1, 0.01, 0.001, 0.0001, 0.00001) | sweepSvdCutoff | view
    }

### bin/
This directory should contain all scripts (eg Bash, Perl, Python) that are executable.  This directory is automatically added to the search path by Nextflow.

### work/
This is where output from Nextflow is stored.  There will be one subdirectory per pipeline execution here.

## Executing a Nextflow pipeline and generating a memory usage and execution time report
Execute with the `standard` profile:
```
nextflow run main.nf -with-report
```
Execute with the `cluster` profile:
```
nextflow run main.nf -profile cluster -with-report
```
## Example demonstrating asynchronous execution over multiple workers (shot partitioning)
The pipeline parallelises circuit evaluation across multiple processes, and across multiple threads per process.  This example detects all circuits in the current working directory (extension `.oqm`).  These are assumed to be in OpenQASM 2.0 format.

### main-partitioned.nf

    #!/usr/bin/env nextflow
    nextflow.enable.dsl=2
    import groovy.json.JsonSlurper

    def N_SHOTS = 512
    def N_PROCESSES = 4
    def N_ASYNC_THREADS = 2

    def N_PHYSICAL_QUBITS = 2
    def QRISTAL_ACC = "aer"

    def jsonSlurper = new JsonSlurper()

    process partitionCircuitQubitBackend {
        debug false
        input:
            path circuit
            each shots_N

        output:
           stdout

        """
        #!/usr/bin/python3
        import ast
        import json
        import numpy as np
        import time
        import qristal.core
        my_sim = qristal.core.session()
        my_sim.init()

        my_sim.qn = $N_PHYSICAL_QUBITS  # Number of qubits
        my_sim.infile = "$circuit"

        my_sim.noplacement = True
        my_sim.nooptimise = True
        my_sim.notiming = True
        my_sim.output_oqm_enabled = False

        NW = $N_ASYNC_THREADS  # number of async workers
        SLEEP_SECONDS = 0.1 # seconds to sleep between progress
        ALG_UNDER_TEST = 0

        # Set up workers
        # Set up the pool of backends for parallel task distribution
        qpu_config = {"accs": NW*[{"acc": "$QRISTAL_ACC"}]}
        my_sim.set_parallel_run_config(json.dumps(qpu_config))

        # Set the number of threads to use in the thread pool
        my_sim.num_threads = NW

        # Set up jobs that partition the requested number of shots
        my_sim.sn[ALG_UNDER_TEST].clear()
        for jj in range(NW):
            my_sim.sn[ALG_UNDER_TEST].append($shots_N // NW)

        handles = []
        for i in range(NW):
            handles.append(my_sim.run_async(ALG_UNDER_TEST, i))
            time.sleep(0.001)

        # Gather the results
        allres = {}
        componentres = [ast.literal_eval(handles[i].get()) for i in range(NW)]
        for ii in range(NW):
            allres = {k: allres.get(k,0) + componentres[ii].get(k,0) for k in set(allres) | set(componentres[ii])}

        # View the results
        print(json.dumps(allres))

        # Store the settings
        save_js = {}
        save_js['shots'] = $shots_N
        save_js['backend'] = "$QRISTAL_ACC"
        save_js['workers'] = $N_ASYNC_THREADS
        save_js['circuit'] = "$circuit"
        save_js['qubits'] = $N_PHYSICAL_QUBITS
        with open('settings.json', 'w') as f:
            json.dump(save_js, f)
        """
    }
    def gatherall = [:]
    workflow {
      circuit_ch = Channel.fromPath("*.oqm")
      shots_N_ch = (0..<N_PROCESSES).collect { N_SHOTS/N_PROCESSES }
      shotoutcomes_ch = partitionCircuitQubitBackend(circuit_ch, shots_N_ch).map { jsonSlurper.parseText( it ) }
      (shotoutcomes_ch.map { gatherall = (gatherall.keySet() + it.keySet()).collectEntries { k -> [k, (gatherall[k] ?: 0) + (it[k] ?: 0)] } }).last().view()
    }

### Example OpenQASM file: `ex-nf1.oqm`
```
OPENQASM 2.0;
include "qelib1.inc";
qreg q[2];
creg c[2];
h q[0];
cx q[0],q[1];
measure q[0] -> c[0];
measure q[1] -> c[1];
```
Execute the pipeline using:
```
nextflow run main-partitioned.nf
```
The results are stored in subdirectories under `work/`.  View the `.command.out` and `settings.json` files there.
