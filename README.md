# Quantum Brilliance SDK Qristal -- Core #


Qristal is the QB Software Development Kit for quantum computing.  This is its core module.

### Installation

**Prerequisites**

Linux (e.g. Ubuntu) is required. One may choose to use Ubuntu with WSL 2 ([Windows Subsystem for Linux](https://learn.microsoft.com/en-us/windows/wsl/)), as the installation sequence is the same. The best performance is achieved with regular Linux, due to additional overhead coming from the Windows filesystem. Please ensure if using WSL that you turn on file system case sensitivity for the location where you intend to install Qristal.

Before building Qristal, you must have the following packages already installed and working:

- Python 3.8 or later

- gcc, g++, and gfortran 11.4.0 or later. LLVM-Clang 16.0.6 or later is supported, but gcc/g++ is still required for building exatn and tnqvm.

- cmake 3.20 or later

- Boost 1.71 or later

- OpenBLAS

- OpenSSL

- Curl


For example, on Ubuntu 22.04, you can use `apt` to install all of the above:

```
sudo apt install build-essential cmake gfortran libboost-all-dev libcurl4-openssl-dev libssl-dev libopenblas-dev libpython3-dev python3 python3-pip
```

Qristal will be built with support for CUDA Quantum if and only if cmake detects that your system has a compatible CUDA Quantum installation.

**Compilation**

<a name="compilation"></a>

After cloning the Qristal SDK repository, compile and install it with

```
mkdir build && cd build
cmake .. -DINSTALL_MISSING=ON
make -j$(nproc) install
```

If you wish to only install missing C++ or Python dependencies, instead of passing `-DINSTALL_MISSING=ON` you can pass `-DINSTALL_MISSING=CXX` or `-DINSTALL_MISSING=PYTHON`.

If you wish to build Qristal's C++ noise-aware circuit placement routines, you must also enable the use of the additional dependency [TKET](https://github.com/CQCL/tket). This is done by passing `-DWITH_TKET=ON` to `cmake`. TKET will be installed automatically by `cmake` if both `-DWITH_TKET=ON` and `-DINSTALL_MISSING=ON` (or `-DINSTALL_MISSING=CXX`) are passed to `cmake`. Alternatively, if you have an existing TKET installation, you can pass `-DWITH_TKET=ON -DTKET_DIR=<YOUR TKET INSTALLATION DIR>` to `cmake` to tell it to use your installation rather than building TKET from source.

If you also wish to build the html documentation, you can pass `-DBUILD_DOCS=ON` to `cmake`.

MPI acceleration is supported via adding `-DWITH_MPI=ON` to the `cmake` configuration step. Note that `-DMPI_HOME=...` can also be set instead (or in addition) to enable MPI and specify a custom install location. See [Installing from source](https://qristal.readthedocs.io/en/latest/rst/getting_started.html#installing-from-source) for more information on MPI support in Qristal.

## Documentation
You can find the docs for Qristal on the web at [qristal.readthedocs.io](https://qristal.readthedocs.io).  If you have built and installed the documentation (see [compilation](#compilation)), you can also find it at `<installation_directory>/docs/html/index.html`.

## License ##
[Apache 2.0](LICENSE)
