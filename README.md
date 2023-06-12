# Quantum Brilliance SDK Qristal -- Core #

Qristal is the QB Software Development Kit for quantum computing.  This is its core module.

### Installation

**Prerequisites**

As of this beta release, only Linux (e.g., Ubuntu) is supported.

At a minimum, the following packages are required:

- Python3.8+

- gcc, g++, and gfortran (version 7+).  Usage of clang is supported, but gcc/g++ is still required for building exatn and tnqvm.

- cmake (version 3.20+)

- Boost 1.71+

- OpenBLAS

- OpenSSL

- Curl


For example, on Debian-based distributions (e.g., Ubuntu), we can use `apt` to install all above prerequisites:

```
sudo apt install build-essential cmake gfortran libboost-all-dev libcurl4-openssl-dev libssl-dev libopenblas-dev libpython3-dev python3 python3-pip
```

Qristal will be built with support for CUDA Quantum if and only if cmake detects that your system has a compatible CUDA Quantum installation.

**Compilation**

<a name="compilation"></a>

After cloning the QB Qristal SDK repository, compile and install it with

```
mkdir build && cd build
cmake .. -DINSTALL_MISSING=ON
make -j$(nproc) install
```

If you wish to only install missing C++ or Python dependencies, instead of passing `-DINSTALL_MISSING=ON` you can pass `-DINSTALL_MISSING=CXX` or `-DINSTALL_MISSING=PYTHON`.

If you also wish to build the C++ noise-aware circuit placement based on the [TKET](https://github.com/CQCL/tket) library, you can pass `-DWITH_TKET=ON` to `cmake`.

Along with the `-DINSTALL_MISSING=ON` option as shown above, `cmake` will automatically pull in and build TKET for you.
Alternatively, if you have an existing TKET installation, you can pass `-DWITH_TKET=ON -DTKET_DIR=<YOUR TKET INSTALLATION DIR>` to `cmake` to let it use your installation rather than building TKET from source.  

If you also wish to build the html documentation, you can pass `-DBUILD_DOCS=ON` to `cmake`.

## Documentation
You can find the docs for Qristal on the web at [qristal.readthedocs.io](https://qristal.readthedocs.io).  If you have built and installed the documentation (see [compilation](#compilation)), you can also find it at `<installation_directory>/docs/html/index.html`.  

## License ##
[Apache 2.0](LICENSE)
