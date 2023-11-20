===============
Getting started
===============

Using Qristal
-------------

.. include:: ../md/getting_started_bell_state.md
   :parser: myst_parser.sphinx_

Development guide
-----------------

Qristal can either be run `directly from the provided Docker image <https://github.com/qbrilliance/qristal#docker>`_, or `installed from source <https://github.com/qbrilliance/qristal>`_.  This section provides instructions for working with the source. If using Windows Subsystem for Linux 2, ensure you have Docker Desktop installed **on Windows** and not under your Linux distribution, as it allows Docker to use `WSL2 as a backend <https://docs.docker.com/desktop/windows/wsl/>`_. 

Dependencies
^^^^^^^^^^^^

.. _required_deps:

Installing Qristal from source **requires** the following libraries/packages to be installed:

* `GNU C++ Compiler <https://gcc.gnu.org/>`_ >= 11.4.0.  Usage of clang 16.0.0 or later is supported, but gcc/g++ is still required for building exatn and tnqvm.
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

.. note:: **Automatic dependency installation**

      It is highly recommended that users let the Qristal build system install the :ref:`additional dependencies mentioned above <auto_install_deps>`, by setting the ``-DINSTALL_MISSING=ON`` CMake option.

Installation
^^^^^^^^^^^^

1. For development purposes, it is **recommended** to install Qristal using *automatic dependency installation* mode.

.. code-block:: bash

    git clone https://github.com/qbrilliance/qristal.git qristal
    cd qristal
    mkdir build && cd build
    cmake .. -DINSTALL_MISSING=ON
    make install

The ``-DINSTALL_MISSING=ON`` flag ensures that all missing dependencies (if any) will be downloaded and installed automatically.  To automatically download and install only C++ dependencies, instead set ``-DINSTALL_MISSING=CXX``.  To download and install only Python module dependencies, use ``-DINSTALL_MISSING=PYTHON``.

.. note:: The :ref:`required dependencies <required_deps>` **must** be installed on your system. ``-DINSTALL_MISSING=ON`` will not handle those mandatory dependencies.
.. note:: MPI is enabled by adding the option ``-DENABLE_MPI=ON`` to the ``cmake`` command.

2. Manual Installation of :ref:`additional dependencies <auto_install_deps>` (**Advanced**)

- Follow the installation instructions of `XACC <https://github.com/eclipse/xacc>`_, `ExaTN <https://github.com/ORNL-QCI/exatn>`_ and `TNQVM <https://github.com/ORNL-QCI/tnqvm>`_.

- Perform system-level installation of the :ref:`remaining libraries<auto_install_deps>`.

- Configure CMake build with

.. code-block:: bash

    cmake .. -DXACC_DIR=<YOUR XACC INSTALLATION DIR> -DEXATN_DIR=<YOUR EXATN INSTALLATION DIR> -DTNQVM_DIR=<YOUR TNQVM INSTALLATION DIR>

.. note::

      In this manual mode, the build system will check for a **specific** version of XACC, EXATN and TNQVM as provided.
      If not satisfied, it will terminate the build and ask for a reinstallation of the dependency.
      Please follow the error message to install the correct version (specified as a git commit hash key).


Contributing
^^^^^^^^^^^^

There are many ways in which you can contribute to Qristal, whether by contributing some code or by engaging in discussions;
we value contributions in all shapes and sizes!

Here are some ideas for how you can get involved.

1. **Asking Questions**

Have a question? Some concepts are hard-to-understand?

Please feel free to file an issue to ask your questions `here <https://github.com/qbrilliance/qristal/issues/new/choose>`_.

Your question will serve as resource to others searching for help.

2. **Reporting and/or Commenting on Issues**

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

3. **Contributing Code**

Before submitting a `new pull request <https://github.com/qbrilliance/qristal/pulls>`_, please make sure the following is done:

* **New features should include a unit test.** If you've fixed a bug or added
  code that should be tested, add a test to the ``tests`` directory.

* **Ensure that the test suite passes**, e.g., by running ``ctest``. This will also be checked by our CI when the MR is submitted.

When ready, submit your fork as a `pull request <https://docs.github.com/articles/using-pull-requests>`_
to the QB GitHub repository, filling out the pull request form.

* When describing the pull request, please include as much detail as possible
  regarding the changes made/new features added/performance improvements. If including any
  bug fixes, mention the issue numbers associated with the bugs.

* Once you have submitted the pull request, the **CI pipeline** will automatically run to ensure that all tests continue to pass.

We may ask for changes to a pull request if it requires more documentation or unit tests to better make use of it.

Last but not least, **thank you** for taking the time to contribute.

