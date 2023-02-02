# Quantum Brilliance SDK -- Core #

The QB SDK is the QB software development kit for quantum computing.  This is its core module.

### Installation

**Prerequisites**

In this private beta release, only Linux (e.g., Ubuntu) is supported.

At a minimum, the following packages are required:

- Python3.8+

- gcc, g++, and gfortran (version 7+).  Usage of clang is supported, but gcc/g++ is still required for building exatn and tnqvm.

- cmake (version 3.20+)

- Eigen 3.4+

- Boost 1.71+

- OpenBLAS

- Curl


For example, on Debian-based distributions (e.g., Ubuntu), we can use `apt` to install all above prerequisites:

```
sudo apt install build-essential cmake gfortran libboost-all-dev libcurl4-openssl-dev  libeigen3-dev libopenblas-dev libpython3-dev python3 python3-pip
```

**Compilation**

<a name="compilation"></a>

After cloning the repository, compile and install it with

```
export GITLAB_PRIVATE_TOKEN=<YOUR GITLAB API KEY>
mkdir build && cd build
cmake .. -DINSTALL_MISSING=ON
make -j$(nproc) install
```

If you also wish to build the html documentation, you can pass `-DQB_BUILD_DOCS=ON` to `cmake`.

## Documentation
If you have built and installed the documentation (see [compilation](#compilation)), you can find it at `<installation_directory>/docs/html/index.html`.

## License ##
[Apache 2.0](LICENSE)
