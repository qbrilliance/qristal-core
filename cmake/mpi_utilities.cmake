function(add_mpi)

    find_package(MPI COMPONENTS CXX)

    message(STATUS
        "MPI configuration:\n"
        "      MPI_HOME: ${MPI_HOME}\n"
        "      MPI_VERSION: ${MPI_VERSION}\n"
        "      MPI_CXX_VERSION: ${MPI_CXX_VERSION}\n"
        "      MPI_CXX_INCLUDE_DIRS: ${MPI_CXX_INCLUDE_DIRS}\n"
        "      MPI_CXX_LIBRARIES: ${MPI_CXX_LIBRARIES}\n"
        "      MPI_CXX_COMPILER: ${MPI_CXX_COMPILER}\n"
        "      MPI_CXX_COMPILE_DEFINITIONS: ${MPI_CXX_COMPILE_DEFINITIONS}\n"
        "      MPI_CXX_COMPILE_OPTIONS: ${MPI_CXX_COMPILE_OPTIONS}\n"
        "      MPI_CXX_LINK_FLAGS: ${MPI_CXX_LINK_FLAGS}\n"
        "      MPIEXEC_EXECUTABLE: ${MPIEXEC_EXECUTABLE}"
    )

    if(MPI_CXX_FOUND)
        check_mpi_configuration()
    else()
        message(FATAL_ERROR
            "Build has been configured for MPI support but MPI has not been found.\n"
            "Either install an MPI implementation or ensure the cmake configuration is configured correctly.\n"
            "Non-system install locations can be specified using -DMPI_HOME=<custom install location>.\n"
            "Otherwise, either install an MPI implementation or try manually setting:\n"
            "    -DMPI_CXX_HEADER_DIR to the location of mpi.h\n"
            "    -DMPI_CXX_COMPILER to the location of the mpicxx compiler\n"
            "    -DMPI_CXX_LIBRARIES to the locations of libmpi.so and libmpi_cxx.so\n"
            "See https://cmake.org/cmake/help/v3.20/module/FindMPI.html for more information."
        )
    endif()

endfunction()


function(check_mpi_configuration)

    # Check the compiler is correctly configured
    execute_process(
        COMMAND ${MPI_CXX_COMPILER} --version
        OUTPUT_VARIABLE MPI_COMPILER_VERSION
    )
    execute_process(
        COMMAND ${CMAKE_CXX_COMPILER} --version
        OUTPUT_VARIABLE CMAKE_COMPILER_VERSION
    )

    if("${MPI_COMPILER_VERSION}" STREQUAL "${CMAKE_COMPILER_VERSION}")
        message(STATUS
            "MPI C++ compiler wraps the same compiler that's configured for this cmake project (${CMAKE_CXX_COMPILER})"
        )
    else()
        message(WARNING
            "MPI C++ compiler wraps a different compiler to the one configured for this cmake project "
            "(${CMAKE_CXX_COMPILER}). This may lead to incompatibilities.\n\"${MPI_CXX_COMPILER} --version\" "
            "has the following output:\n${MPI_COMPILER_VERSION}\n$"
        )
    endif()

    # Check MPI items and directories found are under MPI_HOME. If they are not, this means
    # the user has set MPI_HOME to some location but find_package(MPI) has found MPI under
    # another location. As a result, the MPI implementation being used is not the one the
    # user expects and is likely to cause issues.
    set(ALL_FOUND_MPI_ITEMS ${MPI_CXX_INCLUDE_DIRS} ${MPI_CXX_LIBRARIES} ${MPIEXEC_EXECUTABLE} ${MPI_CXX_COMPILER})
    if(MPI_HOME)
        file(REAL_PATH ${MPI_HOME} MPI_HOME_RESOLVED)
    endif()
    foreach(found_mpi_item ${ALL_FOUND_MPI_ITEMS})
        set(found_in_mpi_home (${found_mpi_item} MATCHES ^${MPI_HOME}))
        set(found_in_mpi_home_resolved (${found_mpi_item} MATCHES ^${MPI_HOME_RESOLVED}))
        if(NOT found_in_mpi_home AND NOT found_in_mpi_home_resolved)
            message(WARNING "${found_mpi_item} is not a child of ${MPI_HOME}.")
            set(wrong_mpi_implementation TRUE)
        endif()
    endforeach()

    if(NOT IGNORE_MPI_HOME_CHECK AND wrong_mpi_implementation)
        message(FATAL_ERROR
            "Configuration will not continue as the found MPI implementation is not under the configured MPI_HOME "
            "(${MPI_HOME})."
        )
    endif()

endfunction()


# Set underlying C++ compiler for various implementations
# Open MPI - https://www.open-mpi.org/
set(ENV{OMPI_CXX} "${CMAKE_CXX_COMPILER}") # Tested with v5.0.7
# MPICH - https://www.mpich.org/
set(ENV{MPICH_CXX} "${CMAKE_CXX_COMPILER}") # Tested with v4.3.0
# Intel MPI - https://www.intel.com/content/www/us/en/developer/tools/oneapi/mpi-library.html
set(ENV{I_MPI_CXX} "${CMAKE_CXX_COMPILER}") # Tested with v2021.14.2.9
# MVAPICH2 - https://mvapich.cse.ohio-state.edu/
set(ENV{CXX} "${CMAKE_CXX_COMPILER}") # not tested - need InfiniBand/Myrinet network device to compile
# IBM Platform MPI
set(ENV{MPISERIALCOMPILER} "${CMAKE_CXX_COMPILER}") # not tested - need Power PC
# Microsoft MPI
set(ENV{MSMPI_CXX} "${CMAKE_CXX_COMPILER}") # not tested - need Windows Qristal setup
