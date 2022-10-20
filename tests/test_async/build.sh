#!/bin/bash
#// Copyright (c) 2022 Quantum Brilliance Pty Ltd


#set -x

ROOT=/home/jojo/QB/QBOS/qbos
cd $ROOT
source install.sh --set-paths

SOURCE=$ROOT/src/qbos_core/src

cd tests
#-g
#g++ -g -c test_async.cpp $SOURCE/qbos_methods.cpp $SOURCE/qbos_async_executor.cpp  $(find ~/QB/QBOS/qbos/src/qbos_core -type f -iname '*.hpp' -printf "-I%h\n" | sort -u) \
g++ -g -c test_async.cpp $SOURCE/qbos_methods.cpp $SOURCE/thread_pool.cpp $SOURCE/qbos_async_executor.cpp $ROOT/src/QuantumBrillianceNoiseModel.cpp $ROOT/src/QuantumBrillianceRemoteAccelerator.cpp $(find ~/QB/QBOS/qbos/src/qbos_core -type f -iname '*.hpp' -printf "-I%h\n" | sort -u) \
-I../src \
-I${XACC_DIR}/include \
-I${XACC_DIR}/include/xacc \
-I${XACC_DIR}/include/cppmicroservices4 \
-I${XACC_DIR}/include/quantum/gate \
-I${XACC_DIR}/include/gtest

XACC_LIB=${XACC_DIR}/lib
QBOS_LIB=$ROOT/src/qbos_core/src/build

#g++ -g -o test_async test_async.o qbos_methods.o qbos_async_executor.o \
g++ -g -o test_async test_async.o qbos_methods.o thread_pool.o qbos_async_executor.o QuantumBrillianceNoiseModel.o QuantumBrillianceRemoteAccelerator.o \
${XACC_LIB}/libxacc.so \
${XACC_LIB}/libCppMicroServices.so \
${XACC_LIB}/libxacc-quantum-gate.so \
-lpthread #\
#${QBOS_LIB}/libqbos-base.so \
#${QBOS_LIB}/libqbnoise.so \
#-fsanitize=address
