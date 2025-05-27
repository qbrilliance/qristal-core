#include <qristal/core/mpi/mpi_manager.hpp>

namespace qristal::mpi {

MpiManager::MpiManager() : MpiManager(nullptr, nullptr) {}

MpiManager::MpiManager(int *argc, char ***argv) {
  int32_t initialised;
  MPI_Initialized(&initialised);

  if (!initialised) {
    MPI_Init(argc, argv);
    auto finalise_mpi = []() { MPI_Finalize(); };
    std::atexit(finalise_mpi);
  }

  MPI_Comm_rank(MPI_COMM_WORLD, &mpi_process_id_);
  MPI_Comm_size(MPI_COMM_WORLD, &total_processes_);
}
}
