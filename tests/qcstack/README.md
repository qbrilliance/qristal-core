# Tests for Qristal client using api/v1 of the Quantum Brilliance QDK

This section contains a test harness for unit testing.

To build the test, see: `core/cmake/tests.cmake`
There, uncomment the line:
```
  tests/qcstack/QuantumBrillianceRemoteAcceleratorTester.cpp
```
To execute the test, use:
```
cd core/build
make install
./CITests
```