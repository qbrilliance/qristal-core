# Copyright (c) 2022 Quantum Brilliance Pty Ltd
SHELL := /bin/bash

QB_DIR= /home/jojo/QB/QBOS
SOURCE = ${QB_DIR}/qbos/src/qbos_core/src
QB_SRC = ${QB_DIR}/qbos/src/
hpps = ${shell find ${QB_SRC}/qbos_core -type f -iname '*.hpp' -printf "-I%h\n" | sort -u}

XACC_DIR= ${QB_DIR}/xacc-local

INCLUDES = $(hpps) \
-I../src \
-I$(XACC_DIR)/include \
-I${XACC_DIR}/include/xacc \
-I${XACC_DIR}/include/cppmicroservices4 \
-I${XACC_DIR}/include/quantum/gate \
-I${XACC_DIR}/include/gtest

XACC_LIB = ${XACC_DIR}/lib

LIBINCLUDES = ${XACC_LIB}/libxacc.so \
${XACC_LIB}/libCppMicroServices.so \
${XACC_LIB}/libxacc-quantum-gate.so \
-lpthread

all : test_async 

# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
test_async : test_async.o qbos_methods.o thread_pool.o qbos_async_executor.o QuantumBrillianceNoiseModel.o QuantumBrillianceRemoteAccelerator.o
	@echo link $@ ...
	@g++ -g -o test_async test_async.o qbos_methods.o thread_pool.o qbos_async_executor.o QuantumBrillianceNoiseModel.o QuantumBrillianceRemoteAccelerator.o \
	${LIBINCLUDES}

test_async.o : test_async.cpp ${SOURCE}/qbos_methods.cpp ${SOURCE}/thread_pool.cpp ${SOURCE}/qbos_async_executor.cpp \
	${QB_SRC}/QuantumBrillianceNoiseModel.cpp ${QB_SRC}/QuantumBrillianceRemoteAccelerator.cpp
	@echo compile $@ ...
	@g++ -g -c test_async.cpp ${INCLUDES}

qbos_methods.o : ${SOURCE}/qbos_methods.cpp ${SOURCE}/thread_pool.cpp ${SOURCE}/qbos_async_executor.cpp ${QB_SRC}/QuantumBrillianceNoiseModel.cpp ${QB_SRC}/QuantumBrillianceRemoteAccelerator.cpp
	@echo compile $@ ...
	@g++ -g -c ${SOURCE}/qbos_methods.cpp ${INCLUDES}

qbos_async_executor.o : ${SOURCE}/thread_pool.cpp ${SOURCE}/qbos_async_executor.cpp ${QB_SRC}/QuantumBrillianceRemoteAccelerator.cpp
	@echo compile $@ ...	
	@g++ -g -c ${SOURCE}/qbos_async_executor.cpp ${INCLUDES}

thread_pool.o : ${SOURCE}/thread_pool.cpp
	@echo compile $@ ...
	@g++ -g -c ${SOURCE}/thread_pool.cpp ${INCLUDES}

QuantumBrillianceNoiseModel.o : ${QB_SRC}/QuantumBrillianceNoiseModel.cpp ${QB_SRC}/QuantumBrillianceRemoteAccelerator.cpp 
	@echo compile $@ ...
	@g++ -g -c ${QB_SRC}/QuantumBrillianceNoiseModel.cpp ${INCLUDES}

QuantumBrillianceRemoteAccelerator.o : ${QB_SRC}/QuantumBrillianceRemoteAccelerator.cpp
	@echo compile $@ ...
	@g++ -g -c ${QB_SRC}/QuantumBrillianceRemoteAccelerator.cpp ${INCLUDES}

clean :
	rm *.o test_async
