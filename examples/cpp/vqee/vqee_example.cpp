// Copyright (c) 2022 Quantum Brilliance Pty Ltd

#include "qb/core/optimization/vqee/vqee.hpp"

using qb::vqee::operator<<; // cout overload for vector<>

int main (int argc, char *argv[]) {
    // even if only 1 thread is started in qpp the backend(Eigen is openMP parallized): use " export $OMP_NUM_THREADS=1 " to suppress the Eigen threading
    // ececute with: mpiexec -n NPROCS test_VQE NTHREADS

    xacc::Initialize(); //xacc::Initialize(argc, argv);
    xacc::external::load_external_language_plugins();
    xacc::set_verbose(false);
    xacc::ScopeTimer timer_for_cpu("Walltime in ms", false);

    const bool isRoot = GetRank() == 0;
    if (isRoot) {
        if (isMPIEnabled()) {
            printf("MPI_enabled\n");
        }
        else {
            printf("not MPI_enabled\n");
        }

        printf("Program Name Is: %s\n",argv[0]);
        if(argc==1)
            printf("No Extra Command Line Argument Passed Other Than Program Name!\n");
        if(argc>=2)
        {
            printf("Number Of Arguments Passed: %d\n",argc);
            printf("----Following Are The Command Line Arguments Passed----\n");
            for(auto counter=0; counter<argc; counter++)
                printf("argv[%d]: %s\n",counter,argv[counter]);
        }
    }

    int nWorker{GetSize()}; // get number of MPI processes
    int nThreadsPerWorker{1}; // set default number of threads to 1
    if(argc>=2) {
        nThreadsPerWorker = atoi( argv[1] ); // get number of threads per process from main call
    }

    if (isRoot){
        printf("Executing VQE test with %i workers.\n\n",nWorker);
        // Not printing the number of threads per worker, as that isn't properly passed down to the simulator (yet).
    }

    qb::vqee::Params params{qb::vqee::makeJob(qb::vqee::JobID::H2_UCCSD)}; // has all inputs for VQE

    // options may be modified
    // default is deterministic and 1 shot
    /*
    params.tolerance = 1E-2;
    params.nShots = 1000000;
    params.maxIters = 200;
    params.isDeterministic = false;
    */

    params.nWorker = GetSize();
    params.nThreadsPerWorker = nThreadsPerWorker;
    //params.partitioned = true; // enable for cases with many Pauli-terms

    qb::vqee::VQEE vqe{params};
    vqe.optimize();

    const auto      nIters = params.energies.size();
    const double  cpu_ms = timer_for_cpu.getDurationMs();
    if (isRoot){ std::cout << "theta: " << params.theta << ", energy: " << params.optimalValue << ", iterations: " << nIters << ", CPU wall-time: " << cpu_ms << " ms" << std::endl; }

    xacc::Finalize();

    return 0;
}
