===============
Getting started
===============

Quickstart
----------

.. include:: ../md/getting_started_bell_state.md
   :parser: myst_parser.sphinx_


Installation
------------

Qristal can either be run `directly from the provided Docker image <https://github.com/qbrilliance/qristal#docker>`_, or `installed from source <https://github.com/qbrilliance/qristal>`_.  This section provides instructions for working with the source. If using Windows Subsystem for Linux 2, ensure you have Docker Desktop installed **on Windows** and not under your Linux distribution, as it allows Docker to use `WSL2 as a backend <https://docs.docker.com/desktop/windows/wsl/>`_.

Dependencies
^^^^^^^^^^^^

.. _required_deps:

Installing Qristal from source **requires** the following libraries/packages to be installed:

* `GNU C++ Compiler <https://gcc.gnu.org/>`_ >= 11.4.0.  Usage of clang 16.0.6 or later is supported, but gcc/g++ is still required for building exatn and tnqvm.
* `GNU Fortran Compiler <https://gcc.gnu.org/>`_ >= 11.4.0
* `CMake <https://cmake.org/>`_ >= 3.20
* `Python <http://python.org/>`_ >= 3.8
* `OpenBLAS (Basic Linear Algebra Subprograms) <https://www.openblas.net/>`_
* `OpenSSL <https://www.openssl.org>`_
* `Boost <https://www.boost.org/>`_ >= 1.71
* `libcurl <https://curl.se/>`_

Also, note that Qristal will be built with support for CUDA Quantum if and only if cmake detects that your system has a compatible CUDA Quantum installation. A shortcut is to run Qristal directly from `the Docker image that we provide with a compatible version of CUDA Quantum already installed <https://github.com/qbrilliance/qristal#docker>`_.

.. _auto_install_deps:

Additional dependencies that can be installed automatically at build time:

* `XACC <https://github.com/eclipse/xacc>`_
* `ExaTN <https://github.com/ORNL-QCI/exatn>`_
* `TNQVM <https://github.com/ORNL-QCI/tnqvm>`_
* `pybind11 <https://github.com/pybind/pybind11>`_ >= 2.10
* `Eigen <https://eigen.tuxfamily.org/>`_ >= 3.3.7
* `cpr <https://docs.libcpr.org/>`_ >= 1.3.0
* `args <https://github.com/Taywee/args>`_ >= 6.4.1
* `cppitertools <https://github.com/ryanhaining/cppitertools>`_ >= 2.1
* `GoogleTest <https://github.com/google/googletest>`_ >= 1.12.1
* `Nlohmann JSON <https://github.com/nlohmann/json>`_ >= 3.1.1
* Various Python modules.

.. note::
      **Automatic dependency installation**

      It is highly recommended that users let the Qristal build system install the :ref:`additional dependencies mentioned above <auto_install_deps>`, by setting the ``-DINSTALL_MISSING=ON`` CMake option.

Installing from source
^^^^^^^^^^^^^^^^^^^^^^

Automatic Dependency Installation
"""""""""""""""""""""""""""""""""

1. For development purposes, it is **recommended** to install Qristal using *automatic dependency installation* mode.

.. code-block:: bash

  git clone https://github.com/qbrilliance/qristal.git qristal
  cmake -B qristal/build -S qristal -DINSTALL_MISSING=ON
  cmake --build qristal/build --parallel $(nproc)
  cmake --install qristal/build # Default installs to the build directory which in this case is "qristal/build"

The ``-DINSTALL_MISSING=ON`` flag ensures that all missing dependencies (if any) will be downloaded and installed automatically.  To automatically download and install only C++ dependencies, instead set ``-DINSTALL_MISSING=CXX``.  To download and install only Python module dependencies, use ``-DINSTALL_MISSING=PYTHON``.

The :ref:`required dependencies <required_deps>` **must** be installed on your system. ``-DINSTALL_MISSING=ON`` will not handle those mandatory dependencies.


2. Other useful ``cmake`` options

The directory into which Qristal is to be installed can be specified by setting ``-DCMAKE_INSTALL_PREFIX=<YOUR QRISTAL INSTALLATION DIR>``.

If you wish to build Qristal's C++ noise-aware circuit placement routines, you must also enable the use of the additional dependency `TKET <https://github.com/CQCL/tket>`_. This is done by passing ``-DWITH_TKET=ON`` to ``cmake``. TKET will be installed automatically by ``cmake`` if both ``-DWITH_TKET=ON`` and ``-DINSTALL_MISSING=ON`` (or ``-DINSTALL_MISSING=CXX``) are passed to ``cmake``.

If you also wish to build this html documentation, pass ``-DBUILD_DOCS=ON``.


Manual Dependency Installation
""""""""""""""""""""""""""""""

1. Manual Installation of :ref:`additional dependencies <auto_install_deps>` (**Advanced alternative to step 1**)

- Follow the installation instructions of `XACC <https://github.com/eclipse/xacc>`_, `ExaTN <https://github.com/ORNL-QCI/exatn>`_ and `TNQVM <https://github.com/ORNL-QCI/tnqvm>`_.

- Perform system-level installation of the :ref:`remaining libraries<auto_install_deps>`.

- Configure CMake build with

.. code-block:: bash

  cmake .. -DXACC_DIR=<YOUR XACC INSTALLATION DIR> -DEXATN_DIR=<YOUR EXATN INSTALLATION DIR> -DTNQVM_DIR=<YOUR TNQVM INSTALLATION DIR>

In this manual mode, the build system will check for a **specific** version of XACC, EXATN and TNQVM as provided.
If not satisfied, it will terminate the build and ask for a reinstallation of the dependency.
Please follow the error message to install the correct version (specified as a git commit hash key).

Similarly, if building with noise-aware placement routines enabled using ``-DWITH_TKET=ON``, you can pass ``-DWITH_TKET=ON -DTKET_DIR=<YOUR TKET INSTALLATION DIR>`` to ``cmake`` to tell it to use your own installation of TKET rather than building TKET from source.


MPI support
"""""""""""

When using Qristal, enabling MPI allows the user to take advantage of multiple computing nodes to accelerate calculations. Workloads are partitioned across multiple processes, which are typically configured to run across different nodes via something like Slurm, and executed in parallel. The resultant data from each process is then synchronised. The implementation of MPI in the application or dependency determines how the work is partitioned, run and synchronised.

Qristal Native
''''''''''''''

To enable native MPI acceleration in Qristal, either add ``-DWITH_MPI=ON`` or ``-DMPI_HOME=<MPI install location>`` to the ``cmake`` configuration step. The default behaviour is to use whatever MPI installation has been installed to the system's root directory. A custom MPI installation can be specified via ``-DMPI_HOME=<MPI install location>``. For more fine-grained and customised control of MPI compiler, header and library locations to configure the build with, see the relevant section in the `cmake documentation <https://cmake.org/cmake/help/v3.20/module/FindMPI.html#variables-for-locating-mpi>`_.

Qristal implements MPI acceleration by partitioning shots across processes. Once all MPI processes finish running their allocated number of shots, the supervisor MPI process synchronises and, where necessary, rescales the results of all processes before combining them with its own.

Qristal Dependencies
''''''''''''''''''''

To enable MPI acceleration in Qristal dependencies, add ``-DENABLE_MPI_IN_DEPS=ON`` to the ``cmake`` configuration step. Enabling MPI acceleration for Qristal will *not* implicitly enable MPI acceleration within dependencies. When ``-DENABLE_MPI_IN_DEPS=ON`` is used, finding MPI is left up to the individual dependencies that support MPI.

**Dependency support for MPI:**

1. Parallelisation when using specific ``xacc`` backends

   a. ``tnqvm`` backend when built with ``exatn`` backend (which Qristal does)
   b. ``"hpc-virtualization"`` backend

      - See the ``HPCVirtDecorator`` ("xacc/quantum/plugins/decorators/hpc-virtualization/hpc_virt_decorator.*") class declaration (hpp) and definition (cpp) for more information

2. Circuit-level parallelisation for expectation value computation using the ``VQEE`` optimisation algorithm (uses the  `"hpc-virtualization"`` backend)

Example Usage
'''''''''''''

.. code-block:: none
  :class: no-lexing

  cmake -B build -S . -DWITH_MPI=ON -DMPI_HOME=/opt/mpich -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DINSTALL_MISSING=ON
  ...
  -- Found MPI_CXX: /opt/mpich/lib/libmpicxx.so (found version "4.1") 
  -- Found MPI: TRUE (found version "4.1") found components: CXX 
  -- MPI configuration:
      MPI_HOME: /opt/mpich
      MPI_VERSION: 
      MPI_CXX_VERSION: 4.1
      MPI_CXX_INCLUDE_DIRS: /opt/mpich/include
      MPI_CXX_LIBRARIES: /opt/mpich/lib/libmpicxx.so;/opt/mpich/lib/libmpi.so
      MPI_CXX_COMPILER: /opt/mpich/bin/mpicxx
      MPI_CXX_COMPILE_DEFINITIONS: 
      MPI_CXX_COMPILE_OPTIONS: 
      MPI_CXX_LINK_FLAGS: -Wl,-rpath -Wl,/opt/mpich/lib -Wl,--enable-new-dtags
      MPIEXEC_EXECUTABLE: /opt/mpich/bin/mpiexec
  -- MPI C++ compiler wraps the same compiler that's configured for this cmake project (/usr/bin/c++)
  ...

Note that MPI integration with Qristal assumes all "found" MPI directories and binaries are under the ``MPI_HOME`` directory and uses this to ensure ``cmake`` has not found an MPI install that is different to where the user has specified it to be. If this is not the case, the below error will occur:

.. code-block:: none
  :class: no-lexing

  -- MPI configuration:
      MPI_HOME: /opt/mpich
      MPI_VERSION: 
      MPI_CXX_VERSION: 3.1
      MPI_CXX_INCLUDE_DIRS: /usr/lib/aarch64-linux-gnu/openmpi/include;/usr/lib/aarch64-linux-gnu/openmpi/include/openmpi
      MPI_CXX_LIBRARIES: /usr/lib/aarch64-linux-gnu/openmpi/lib/libmpi_cxx.so;/usr/lib/aarch64-linux-gnu/openmpi/lib/libmpi.so
      MPI_CXX_COMPILER: /usr/bin/mpicxx
      MPI_CXX_COMPILE_DEFINITIONS: 
      MPI_CXX_COMPILE_OPTIONS: 
      MPI_CXX_LINK_FLAGS: 
      MPIEXEC_EXECUTABLE: /usr/bin/mpiexec
  -- MPI C++ compiler wraps the same compiler that's configured for this cmake project (/usr/bin/c++)
  CMake Warning at cmake/mpi_utilities.cmake:69 (message):
      /usr/lib/aarch64-linux-gnu/openmpi/include is not a child of /opt/mpich.
  Call Stack (most recent call first):
      cmake/mpi_utilities.cmake:20 (check_mpi_configuration)
      cmake/dependencies.cmake:198 (add_mpi)
      CMakeLists.txt:31 (include)


  CMake Warning at cmake/mpi_utilities.cmake:69 (message):
      /usr/lib/aarch64-linux-gnu/openmpi/include/openmpi is not a child of
      /opt/mpich.
  Call Stack (most recent call first):
      cmake/mpi_utilities.cmake:20 (check_mpi_configuration)
      cmake/dependencies.cmake:198 (add_mpi)
      CMakeLists.txt:31 (include)


  CMake Warning at cmake/mpi_utilities.cmake:69 (message):
      /usr/lib/aarch64-linux-gnu/openmpi/lib/libmpi_cxx.so is not a child of
      /opt/mpich.
  Call Stack (most recent call first):
      cmake/mpi_utilities.cmake:20 (check_mpi_configuration)
      cmake/dependencies.cmake:198 (add_mpi)
      CMakeLists.txt:31 (include)


  CMake Warning at cmake/mpi_utilities.cmake:69 (message):
      /usr/lib/aarch64-linux-gnu/openmpi/lib/libmpi.so is not a child of
      /opt/mpich.
  Call Stack (most recent call first):
      cmake/mpi_utilities.cmake:20 (check_mpi_configuration)
      cmake/dependencies.cmake:198 (add_mpi)
      CMakeLists.txt:31 (include)


  CMake Warning at cmake/mpi_utilities.cmake:69 (message):
      /usr/bin/mpiexec is not a child of /opt/mpich.
  Call Stack (most recent call first):
      cmake/mpi_utilities.cmake:20 (check_mpi_configuration)
      cmake/dependencies.cmake:198 (add_mpi)
      CMakeLists.txt:31 (include)


  CMake Warning at cmake/mpi_utilities.cmake:69 (message):
      /usr/bin/mpicxx is not a child of /opt/mpich.
  Call Stack (most recent call first):
      cmake/mpi_utilities.cmake:20 (check_mpi_configuration)
      cmake/dependencies.cmake:198 (add_mpi)
      CMakeLists.txt:31 (include)


  CMake Error at cmake/mpi_utilities.cmake:75 (message):
      Configuration will not continue as the found MPI implementation is not
      under the configured MPI_HOME (/opt/mpich).
  Call Stack (most recent call first):
      cmake/mpi_utilities.cmake:20 (check_mpi_configuration)
      cmake/dependencies.cmake:198 (add_mpi)
      CMakeLists.txt:31 (include)

  -- Configuring incomplete, errors occurred!


Contributing
------------

There are many ways in which you can contribute to Qristal, whether by contributing some code or by engaging in discussions;
we value contributions in all shapes and sizes!

Here are some ideas for how you can get involved.

Asking Questions
^^^^^^^^^^^^^^^^

Have a question? Some concepts are hard-to-understand?

Please feel free to file an issue to ask your questions `here <https://github.com/qbrilliance/qristal/issues/new/choose>`_.

Your question will serve as resource to others searching for help.


Reporting and Discussing Issues
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

If you have feedback about Qristal, please let us know by filing a `new issue <https://github.com/qbrilliance/qristal/issues/new/choose>`_!

When filing a bug report, please follow the below template:

.. code-block::

   # [Title]
   A concise but specific description of the bug.

   ### Steps to reproduce
   Describe what needs to be done to reproduce the faulty behavior.

   ### Resulting and expected behavior
   A brief description of what you expected and what you actually got.

   ### Self help
   If you found a workaround or tried to fix the bug yourself, please provide your attempt here.

   ### Technical output
   If available, provide a traceback, logs or similar. Console output is helpful here.

   ### Screenshot
   If applicable and helpful, provide a screenshot.

   ### System information and environment
   - Software version:
   - Operating system:


We also encourage you to look at the list of currently `open issues <https://github.com/qbrilliance/qristal/issues>`_ to share your ideas and expertise.

Contributing Code
^^^^^^^^^^^^^^^^^

Before submitting a `new pull request <https://github.com/qbrilliance/qristal/pulls>`_, please make sure to do the following:

* **Include a unit test for any new routines or features.** If you've fixed a bug or added
  code that should be tested, add a test to the ``tests`` directory.

* **Ensure that the test suite passes**, e.g., by running the ``CITests`` executable. This will also be checked by our CI when the MR is submitted.

* **Fix (not suppress) any warnings generated by your changes.** You can turn warnings on in the code by passing ``-DWARNINGS=ON`` to ``cmake``.

When ready, submit your fork as a `pull request <https://docs.github.com/articles/using-pull-requests>`_
to the QB GitHub repository, filling out the pull request form.

* When describing the pull request, please include as much detail as possible
  regarding the changes made/new features added/performance improvements. If including any
  bug fixes, mention the issue numbers associated with the bugs.

* Once you have submitted the pull request, the **CI pipeline** will automatically run to ensure that all tests continue to pass.

We may ask for changes to a pull request if it requires more documentation or unit tests to better make use of it.

Last but not least, **thank you** for taking the time to contribute.
