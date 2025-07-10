# MPI Demo

This Qristal demo shows how to use multiple vQPUs across multiple MPI processes.

To run the demo:
1. Build and install Qristal
2. Change directory to `$CMAKE_INSTALL_PREFIX/examples/cpp/mpi_multi_qpu_demo`
3. Run `MPI_COMPATIBLE_VQPU_IMAGE=<image> docker compose up`
4. Build and run `mpi_multi_qpu_demo`
    - The demo is setup such that the number of MPI processes (up to 24 processes) determines how many vQPUs will be used
        - e.g. `mpirun -n 20 mpi_multi_qpu_demo` will run 20 vQPUs in parallel

Where `<image>` is an MPI-compatible vQPU image.
