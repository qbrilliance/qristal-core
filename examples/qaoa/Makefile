# Copyright (c) 2022 Quantum Brilliance Pty Ltd
SHELL := /bin/bash

QB_DIR = /opt/qb/core
SRC_DIR = ${QB_DIR}/src/optimization/qaoa
XACC_DIR= $(wildcard $(QB_DIR)/deps/xacc/*)#1.0.0-da24f49e
JSON_DIR = $(wildcard $(QB_DIR)/deps/nlohmann_json/*)#c0ddea98dd99624f51929177740dbc9c2c7898de
ARGS_DIR = $(wildcard $(QB_DIR)/deps/args/*)#ab85b487acb949e2b658105646501d1da3925112
EIGEN_DIR = $(wildcard $(QB_DIR)/deps/eigen3/*)#9a867200b380d028964c94a0f5e1bc69ea203172

INCLUDES = \
-I${QB_DIR}/include \
-I${XACC_DIR}/include \
-I${XACC_DIR}/include/xacc \
-I${XACC_DIR}/include/cppmicroservices4 \
-I${XACC_DIR}/include/quantum/gate \
-I${XACC_DIR}/include/gtest \
-I${JSON_DIR}/include \
-I${ARGS_DIR} \
-I${EIGEN_DIR}

XACC_LIB = ${XACC_DIR}/lib

LIBINCLUDES = ${XACC_LIB}/libxacc.so \
${XACC_LIB}/libCppMicroServices.so \
${XACC_LIB}/libxacc-quantum-gate.so \
${XACC_LIB}/libxacc-pauli.so \
${QB_DIR}/build/lib/libqbcore.so \
-lpthread

#execName = test_VQE #VQE_tester
#all : ${execName}


#LDDFLAGS += -g
CPPFLAGS += -O3#-g
cxx=g++
#cxx=mpicxx
#CPPFLAGS += -DENABLE_MPI
# in cmake: target_compile_definitions( myTarget PUBLIC ENABLE_MPI=TRUE)

all : qaoa_example

#%.bin : %.cpp
#	@echo build $@
#	@mpicxx -g $< -o $@ 

#test:
#	@echo $(JSON_DIR)/include
#	@echo ${JSON_DIR}/include	

# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
qaoa_validators.o : ${SRC_DIR}/qaoa_validators.cpp
	@echo compile $@ ...
	@${cxx} $(CPPFLAGS) -c ${SRC_DIR}/qaoa_validators.cpp ${INCLUDES}

qaoa_base.o : ${SRC_DIR}/qaoa_base.cpp
	@echo compile $@ ...
	@${cxx} $(CPPFLAGS) -c ${SRC_DIR}/qaoa_base.cpp ${INCLUDES}

qaoa_simple.o : ${SRC_DIR}/qaoa_simple.cpp
	@echo compile $@ ...
	@${cxx} $(CPPFLAGS) -c ${SRC_DIR}/qaoa_simple.cpp ${INCLUDES}

qaoa_example.o : qaoa_example.cpp 
	@echo compile $@ ...
	@${cxx} $(CPPFLAGS) -c qaoa_example.cpp ${INCLUDES}

qaoa_example : qaoa_example.o qaoa_base.o qaoa_simple.o qaoa_validators.o
	@echo link $@ ...
	@${cxx} $(LDDFLAGS) -o qaoa_example qaoa_example.o  ${LIBINCLUDES} qaoa_simple.o qaoa_base.o qaoa_validators.o ${LIBINCLUDES}  


clean :
	rm *.o qaoa_example
