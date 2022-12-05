# Copyright (c) 2022 Quantum Brilliance Pty Ltd
SHELL := /bin/bash

QB_DIR = /opt/qb/core
XACC_DIR= ${QB_DIR}/deps/xacc/1.0.0-da24f49e

INCLUDES = \
-I${QB_DIR}/include \
-I${XACC_DIR}/include \
-I${XACC_DIR}/include/xacc \
-I${XACC_DIR}/include/cppmicroservices4 \
-I${XACC_DIR}/include/quantum/gate \
-I${XACC_DIR}/include/gtest

XACC_LIB = ${XACC_DIR}/lib

LIBINCLUDES = ${XACC_LIB}/libxacc.so \
${XACC_LIB}/libCppMicroServices.so \
${XACC_LIB}/libxacc-quantum-gate.so \
${XACC_LIB}/libxacc-pauli.so \
-lpthread

#CPPFLAGS += -g
#cxx=g++
cxx=mpicxx
CPPFLAGS += -DENABLE_MPI
# in cmake: target_compile_definitions( myTarget PUBLIC ENABLE_MPI=TRUE)

all : vqee_example

#%.bin : %.cpp
#	@echo build $@
#	@mpicxx -g $< -o $@ 

# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
vqee_example : vqee_example.o
	@echo link $@ ...
	@${cxx} $(CPPFLAGS) -o vqee_example vqee_example.o ${LIBINCLUDES}  

vqee_example.o : vqee_example.cpp #jobConstants.hpp MPI_Wrapper.hpp CaseGenerator.hpp VQEE.hpp
	@echo compile $@ ...
	@${cxx} $(CPPFLAGS) -c vqee_example.cpp ${INCLUDES}

clean :
	rm *.o vqee_example