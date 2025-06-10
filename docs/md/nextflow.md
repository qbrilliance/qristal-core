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
        my_sim.qn = 2     # Number of qubits
        my_sim.sn = 1024  # Number of shots
        my_sim.acc = "tnqvm"
        my_sim.noplacement = True
        my_sim.nooptimise = True
        my_sim.notiming = True
        my_sim.output_oqm_enabled = False
        my_sim.svd_cutoff = ${svdcutoff}
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
        print("SVD cutoff: ", my_sim.svd_cutoff)
        print(my_sim.results)
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
