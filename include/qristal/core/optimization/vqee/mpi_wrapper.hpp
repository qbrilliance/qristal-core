#pragma once

#ifdef ENABLE_MPI
    #include <mpi.h>

    namespace {
    #define QRISTAL_MPI_CHECK_RESULT(MPIFunc, Args)                                   \
      {                                                                            \
        int check_result = MPIFunc Args;                                           \
        if (check_result != MPI_SUCCESS)                                           \
          throw std::runtime_error(#MPIFunc);                                      \
      }

    int GetSize() {
      int size;
      QRISTAL_MPI_CHECK_RESULT(MPI_Comm_size, (MPI_COMM_WORLD, &size))
      return size;
    }

    int GetRank() {
      int rank;
      QRISTAL_MPI_CHECK_RESULT(MPI_Comm_rank, (MPI_COMM_WORLD, &rank))
      return rank;
    }

    bool isMPIEnabled() {
      return true;
    }

    template <typename T>
    void MPI_Bcast_Vector(std::vector<T> &v, int root = 0, MPI_Comm comm = MPI_COMM_WORLD) {
      if (!v.empty())  QRISTAL_MPI_CHECK_RESULT(MPI_Bcast, (&v[0], v.size() * sizeof(T), MPI_BYTE, root, MPI_COMM_WORLD));
    }
    }
#else
    namespace {
    int GetSize() {
      return 1;
    }

    int GetRank() {
      return 0;
    }

    bool isMPIEnabled() {
      return false;
    }
    }

#endif
