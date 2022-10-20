#!/bin/bash
# Copyright (c) 2021 Quantum Brilliance Pty Ltd
#
# For internal QB use only
#
# Modules for qbOS maintain test driver scripts within their respective
# test/ subdirectory.  in_container_tests.sh is intended to be a collection
# point to unify the integration testing workflow of these individual modules.
#
# Prerequisites:
# 1. The host has cloned the repository 
#    into their home directory (ie ~):
#        cd ~ 
#        git clone https://gitlab.com/qbau/qbos.git
#        cd qbos
#        git submodule init
#        git submodule update --init --recursive
#
# Adding new tests:
#    Provide functions named:
#        function_<your-test-name-and-id>()
#        function_job_<your-test-name-and-id>()
# 
#    When done, your new test can be launched with the command:
#        source trigger.sh <your-test-name-and-id>
#     

function_perform_checks(){
	if [[ ! -z "${DO_CHECK}" ]]; then
	  if [[ -z "${FN_PREREQS}" ]]; then
		source "${ORIGINDIR}"/ci/include/function_prereqs.sh
	  else       
		echo "- Prerequisites already installed...skipping function_prereqs" 
	  fi  
	  if [[ -z "${FN_SETPATHS}" ]]; then
		source "${ORIGINDIR}"/ci/include/function_setpaths.sh
	  else       
		echo "- Paths already set...skipping function_setpaths"
	  fi  
	fi  
}

function_build_core(){
    cd "${ORIGINDIR}"/src/qbos_core/src
    rm -rf build && mkdir build && cd build
    cmake .. -DCMAKE_INSTALL_PREFIX="${QB_INSTALL_PATH}"
    make -j4
    make install
}

function_CI-210826-1() {
    echo " ***"
    echo " *** CI-210826-1: qbos_core tests (via Pytest)"
    echo " ***"

	function_perform_checks
    
	function_build_core

    cd "${ORIGINDIR}"/src/qbos_core/test
    python3 qbqe_if_model.py &
    pytest test_cases.py --junitxml=report.xml || exit 210826

    echo " ***"
    echo " *** Passed *** - CI-210826-1: qbos_core tests (via Pytest)"
    echo " ***"
}

function_job_CI-210826-1() {
    function_CI-210826-1
    exit 0
}

function_CI-210826-2() {
	echo " ***"
    echo " *** CI-210826-2: qbos_core tests (via gtest)"
    echo " ***"

    function_perform_checks
    
    function_build_core

    cd "${ORIGINDIR}"/src/qbos_core/test
    qbos_core --test --gtest_output='xml:report.xml' || exit 210826

    echo " ***"
    echo " *** Passed *** - CI-210826-2: qbos_core tests (via gtest)"
    echo " ***"
}

function_job_CI-210826-2() {
    function_CI-210826-2
    exit 0
}


function_CI-220308() {
	echo " ***"
    echo " *** CI-220308: qbos_core tests (via gtest)"
    echo " ***"

    function_perform_checks
    
    function_build_core

    cd "${ORIGINDIR}"/src/qbos_core/test/advice/
    qbos_core --test --gtest_output='xml:report.xml' || exit 220308
    echo " ***"
    echo " *** Passed *** - CI-220308: qbos_core tests (via gtest)"
    echo " ***"
}

function_job_CI-220308() {
    function_CI-220308
    exit 0
}


function_CI-210930-1() {
	echo " ***"
    echo " *** CI-210930-1: qbos_op - VQE and QAOA tests (via gtest)"
    echo " ***"

    function_perform_checks
    
    function_build_core

    cd "${ORIGINDIR}"/plugins/optimisation_modules/VQE/src
    rm -rf build && mkdir build && cd build
    cmake .. -DCMAKE_INSTALL_PREFIX="${QB_INSTALL_PATH}"
    make -j
    make install
    cd "${ORIGINDIR}"/plugins/optimisation_modules/VQE/test
    qbos_op_vqe --test --gtest_output='xml:report.xml' || exit 210930

    cd "${ORIGINDIR}"/plugins/optimisation_modules/QAOA/src
    rm -rf build && mkdir build && cd build
    cmake .. -DCMAKE_INSTALL_PREFIX="${QB_INSTALL_PATH}"
    make -j
    make install

    cd "${ORIGINDIR}"/plugins/optimisation_modules/QAOA/test
    qbos_op_qaoa --test --gtest_output='xml:report.xml' || exit 220215

    echo " ***"
    echo " *** Passed *** - CI-210930-1: qbos_op - VQE and QAOA tests (via gtest)"
    echo " ***"
}

function_job_CI-210930-1() {
    function_CI-210930-1
    exit 0
}


function_CI-210930-2() {
	echo " ***"
    echo " *** CI-210930-2: qbos_op.vqe() - VQE and QAOA tests (via pytest)"
    echo " ***"

    function_perform_checks
    
    function_build_core

    cd "${ORIGINDIR}"/plugins/optimisation_modules
    rm -rf build && mkdir build && cd build
    cmake .. -DCMAKE_INSTALL_PREFIX="${QB_INSTALL_PATH}"
    make -j
    make install
    
    # VQE pytest requires pyscf plugin to load atomic geometry
    python3 -m pip install pyscf

    cd "${ORIGINDIR}"/plugins/optimisation_modules/VQE/test
    pytest test_cases.py --junitxml=report.xml || exit 210930

    cd "${ORIGINDIR}"/plugins/optimisation_modules/QAOA/test
    pytest test_cases.py --junitxml=report.xml || exit 220215

    echo " ***"
    echo " *** Passed *** - CI-210930-2: qbos_op.vqe() - VQE and QAOA tests (via pytest)"
    echo " ***"
}

function_job_CI-210930-2() {
    function_CI-210930-2
    exit 0
}

function_DEPS-211101-1() {
	if [[ ! -z $DO_CHECK ]]; then
	  echo "********"
	  echo "Workaround: installing pyscf from qbOS precompiled wheel during CI test..."
	  echo "********"

	  apt-get update   
	  add-apt-repository -y ppa:ubuntu-toolchain-r/test
	  apt-get install -y openssh-client \
						 libcurl4-openssl-dev \
						 libopenmpi-dev \
						 libopenblas-base libopenblas-dev libblas-dev liblapack-dev libboost-dev libarmadillo-dev \
						 bc \
						 gcc-8 g++-8 gfortran-8 \
						 python3 libpython3-dev python3-pip \
						 libunwind-dev 
	  python3 -m pip install --upgrade pip
	  python3 -m pip install --upgrade cmake
	  python3 -m pip install --upgrade pytest
	  python3 -m pip install ase
	  python3 -m pip install "${ORIGINDIR}"/tpls/pyscf-wheel/pyscf-2.0.0-cp36-cp36m-linux_x86_64.whl
	  python3 -m pip install openfermion
	  python3 -m pip install openfermionpyscf
	  # more recent cirq versions are known to have problems with openfermion 
	  python3 -m pip install cirq==0.10.0

	  if [[ -z "${FN_SETPATHS}" ]];
	  then
		source "${ORIGINDIR}"/ci/include/function_setpaths.sh
	  else       
		echo "- Paths already set...skipping function_setpaths"
	  fi  

	fi
}

function_CI-211101-1() {
    echo " ***"
    echo " *** CI-211101-1: q_chemistry tests (via Pytest)"
    echo " ***"

	function_DEPS-211101-1

    function_build_core

    cd "${ORIGINDIR}"/plugins/optimisation_modules
    rm -rf build && mkdir build && cd build
    cmake .. -DCMAKE_INSTALL_PREFIX="${QB_INSTALL_PATH}"
    make -j
    make install

    cd "${ORIGINDIR}"/apps/q_chemistry/test
    cp -p "${ORIGINDIR}"/apps/q_chemistry/external_potential.py "${QB_INSTALL_PATH}/lib"/.
    cp -p "${ORIGINDIR}"/apps/q_chemistry/vqe_calculator.py "${QB_INSTALL_PATH}/lib"/.
    pytest test_cases.py --junitxml=report.xml || exit 211101

    echo " ***"
    echo " *** Passed *** - CI-211101-1: q_chemistry tests (via Pytest)"
    echo " ***"
}

function_job_CI-211101-1() {
    function_CI-211101-1
    exit 0
}

function_DEPS-211202-1() {
	if [[ ! -z $DO_CHECK ]]; then
	  python3 -m pip install pyQuirk
	  python3 -m pip install opencv-python
	  python3 -m pip install mock
	fi
}

function_CI-211202-1() {
	echo " ***"
    echo " *** CI-211202-1: qbos_vis - Quantum Utility plot tests (via pytest)"
    echo " ***"

    function_perform_checks
	
	function_DEPS-211202-1
    
    cd "${ORIGINDIR}"/plugins/qbos_visual/test
    export PYTHONPATH="${ORIGINDIR}"/plugins/qbos_visual:"${PYTHONPATH}"
    pytest test_qbos_vis_quantum_utility.py --junitxml=report.xml || exit 211202
    pytest test_qbos_vis_tomography.py --junitxml=report.xml || exit 211202
    
    echo " ***"
    echo " *** Passed *** - CI-211202-1: qbos_vis Quantum Utility plot tests (via pytest)"
    echo " ***"
}

function_job_CI-211202-1() {
    function_CI-211202-1
    exit 0
}

function_CI-220307-1() {
	echo " ***"
    echo " *** CI-220307-1: qbOS Exponential Search with Comparator tests (via Pytest)"
    echo " ***"

    function_perform_checks
    
    function_build_core

    cd "${ORIGINDIR}"/src/qbos_core/test
    pytest exponential_search_tests.py --junitxml=report.xml || exit 220307
    echo " ***"
    echo " *** Passed *** - CI-220307-1: Exponential Search with Comparator (via Pytest)"
    echo " ***"
}

function_job_CI-220307-1() {
    function_CI-220307-1
    exit 0
}

# function_CI-220512-1() {
#     if [[ -z "${FN_PREREQS}" ]];
#     then
#       source "${ORIGINDIR}"/ci/include/function_prereqs.sh
#     else
#       echo "- Prerequisites already installed...skipping function_prereqs"
#     fi
#     if [[ -z "${FN_SETPATHS}" ]];
#     then
#       source "${ORIGINDIR}"/ci/include/function_setpaths.sh
#     else
#       echo "- Paths already set...skipping function_setpaths"
#     fi
#     echo " ***"
#     echo " *** CI-220512-1: qbos quantum decoder tests (via gtest)"
#     echo " ***"

#     cd "${ORIGINDIR}"/src/qbos_core/src
#     rm -rf build && mkdir build && cd build
#     cmake .. -DCMAKE_INSTALL_PREFIX="${QE_INSTALL_PATH}"
#     make -j
#     make install
#     cd "${ORIGINDIR}"/src/qbos_core/src/build/plugins/qbos_algorithms/quantum_decoder/tests
#     ./QuantumDecoderAlgorithmTester --test --gtest_output='xml:report.xml' || exit 220512
#     echo " ***"
#     echo " *** Passed *** - CI-220512-1: qbos quantum decoder tests (via gtest)"
#     echo " ***"
# }

# function_job_CI-220512-1() {	
#     function_CI-220512-1	
#     exit 0	
# }

function_CI-220407-1() {
	echo " ***"
    echo " *** CI-220407-1: qbOS Lambda Workstation Accelerator Mock tests (via gtest)"
    echo " ***"

    function_perform_checks

	function_build_core

    cd "${ORIGINDIR}"/src/qbos_core/src/build/plugins/qb_lambda/tests
    #TODO python3 -m pip install flask
    python3 fake_aer_server.py &
    # Wait some time for the Python server (mocking Lambda machine) to start
    sleep 30 && ./QBLambdaTester --gtest_output='xml:report.xml' || exit 220307

    echo " ***"
    echo " *** Passed *** - CI-220407-1: Lambda Workstation Accelerator Mock tests (via gtest)"
    echo " ***"
}

function_job_CI-220407-1() {
    function_CI-220407-1
    exit 0
}

function_CI-220608-1() {
	echo " ***"
    echo " *** CI-220608-1: qbOS sparse simulator tests (via gtest)"
    echo " ***"

    function_perform_checks
    
    function_build_core

    cd "${ORIGINDIR}"/src/qbos_core/src/build/plugins/qb_sparse_simulator/tests
    ./QBSparseSimTester --gtest_output='xml:report.xml' || exit 220608

    echo " ***"
    echo " *** Passed *** - CI-220608-1: qbOS sparse simulator tests (via gtest)"
    echo " ***"
}

function_job_CI-220608-1() {
    function_CI-220608-1
    exit 0
}

function_CI-220727-1() {
    if [[ -z "${FN_PREREQS}" ]];
    then
      source "${ORIGINDIR}"/ci/include/function_prereqs.sh
    else       
      echo "- Prerequisites already installed...skipping function_prereqs"
    fi  
    if [[ -z "${FN_SETPATHS}" ]];
    then
      source "${ORIGINDIR}"/ci/include/function_setpaths.sh
    else       
      echo "- Paths already set...skipping function_setpaths"
    fi  
    echo " ***"
    echo " *** CI-220727-1: qbOS Exponent (base 2) tests (via Pytest)"
    echo " ***"
    
    cd "${ORIGINDIR}"/src/qbos_core/src
    rm -rf build && mkdir build && cd build
    cmake .. -DCMAKE_INSTALL_PREFIX="${QE_INSTALL_PATH}"
    make -j
    make install
    cd "${ORIGINDIR}"/src/qbos_core/test
    pytest exponent_tests.py --junitxml=report.xml || exit 220727
    echo " ***"
    echo " *** Passed *** - CI-220727-1: Exponent base 2 (via Pytest)"
    echo " ***"
}

function_job_CI-220727-1() {
    function_CI-220727-1
    exit 0
}


function_CI-220727-1-alpha() {
    if [[ -z "${FN_PREREQS}" ]];
    then
      source "${ORIGINDIR}"/ci/include/function_prereqs.sh
    else       
      echo "- Prerequisites already installed...skipping function_prereqs"
    fi  
    if [[ -z "${FN_SETPATHS}" ]];
    then
      source "${ORIGINDIR}"/ci/include/alpha/function_setpaths.sh
    else       
      echo "- Paths already set...skipping function_setpaths"
    fi  
    echo " ***"
    echo " *** CI-220727-1-alpha: qbOS Exponent (base 2) tests (via Pytest)"
    echo " ***"
    
    cd "${ORIGINDIR}"/src/qbos_core/src
    rm -rf build && mkdir build && cd build
    cmake .. -DCMAKE_INSTALL_PREFIX="${QE_INSTALL_PATH}"
    make -j
    make install
    cd "${ORIGINDIR}"/src/qbos_core/test
    pytest exponent_tests.py --junitxml=report.xml || exit 220727
    echo " ***"
    echo " *** Passed *** - CI-220727-1-alpha: Exponent base 2 (via Pytest)"
    echo " ***"
}

function_job_CI-220727-1-alpha() {
    function_CI-220727-1-alpha
    exit 0
}


function_CI-220715-1() {
    echo " ***"
    echo " *** CI-220715-1: qbOS VQE-gen tests (via gtest)"
    echo " ***"
    
    function_perform_checks
    
    function_build_core

    cd "${ORIGINDIR}"/src/qbos_core/src/build/plugins/uccsd/tests
    ./UCCSDTester --gtest_output='xml:report.xml' || exit 220715
    cd "${ORIGINDIR}"/src/qbos_core/src/build/plugins/qb_vqe/tests
    ./VqeGenTester --gtest_output='xml:report.xml' || exit 220715

    echo " ***"
    echo " *** Passed *** - CI-220715-1: qbOS VQE-gen tests (via gtest)"
    echo " ***"
}

function_job_CI-220715-1() {
    function_CI-220715-1
    exit 0
}

function_CI-220916-1() {
    echo " ***"
    echo " *** CI-220916-1: qbOS noise model nm2 (via Pytest)"
    echo " ***"

    function_perform_checks
    
    function_build_core
    
    cd "${ORIGINDIR}"/src/qbos_core/test
    pytest test_noise_models.py --junitxml=report.xml || exit 220916
    echo " ***"
    echo " *** Passed *** - CI-220916-1: qbOS noise model nm2 (via Pytest)"
    echo " ***"
}

function_job_CI-220916-1() {
    function_CI-220916-1
    exit 0
}

# Package release deploy jobs
function_release_package() {
	echo " ***"
    echo " *** Release package : prepare a tarball with binaries + required libraries"
    echo " ***"

    function_perform_checks

    # legacy qbqe v1 and v2
    cd "${ORIGINDIR}"/legacy/quantum-emulator/quantum_brilliance/100_tests/002_integration/200909_qbqe_with_noise
    rm -rf build && mkdir build && cd build
    cmake .. -DCMAKE_INSTALL_PREFIX="${QB_INSTALL_PATH}"
    make
    make install
    ldconfig >/dev/null 2>&1
    cd "${ORIGINDIR}"/legacy/quantum-emulator/quantum_brilliance/100_tests/001_unit/03_libqbqe
    rm -rf build && mkdir build && cd build
    # For this legacy libqbqe, we always use gcc8
    CC=gcc-8 CXX=g++-8 cmake .. -DCMAKE_INSTALL_PREFIX="${QB_INSTALL_PATH}"
    make
    make install
    ldconfig >/dev/null 2>&1 

    # qbos_core
    function_build_core

    ldconfig >/dev/null 2>&1 

    # qbos_op (VQE and QAOA)
    cd "${ORIGINDIR}"/plugins/optimisation_modules
    rm -rf build && mkdir build && cd build
    cmake .. -DCMAKE_INSTALL_PREFIX="${QB_INSTALL_PATH}"
    make
    make install
    ldconfig >/dev/null 2>&1 

    # qbOS plugins installed to $QB_INSTALL_PATH/lib/plugins
    mkdir -p /mnt/qb/lib/plugins
    cp -pr "${ORIGINDIR}"/plugins/qbos_base_algorithms /mnt/qb/lib/plugins/.
    cp -pr "${ORIGINDIR}"/plugins/qbos_decoder /mnt/qb/lib/plugins/.
    # cp -pr "${ORIGINDIR}"/plugins/QuantumBitStringComparator /mnt/qb/lib/plugins/QuantumBitStringComparator
    cp -pr "${ORIGINDIR}"/plugins/QuantumGates /mnt/qb/lib/plugins/.
    # qbOS visualisation
    cp -pr "${ORIGINDIR}"/plugins/qbos_visual/qbos_vis /mnt/qb/lib/plugins/.
    #
    cp -p "${ORIGINDIR}"/plugins/__init__.py /mnt/qb/lib/plugins/__init__.py

    # qbOS apps - q_chemistry
    cp -p "${ORIGINDIR}"/apps/q_chemistry/external_potential.py "${QB_INSTALL_PATH}/lib"/.
    cp -p "${ORIGINDIR}"/apps/q_chemistry/vqe_calculator.py "${QB_INSTALL_PATH}/lib"/.
    
     # qbOS documentation
    cd /mnt 
    mkdir -p /mnt/qb/share

    cp "${ORIGINDIR}"/assets/user_guide/build/jupyterlab/index.md qb/Documentation.md
    cp -pr "${ORIGINDIR}"/assets/user_guide/build/jupyterlab/qbOS\ Doc* qb/.
    rm -f qb/qbOS\ Documentation*.md

    # qbOS entrypoint for Docker container
    cp "${ORIGINDIR}"/install-r.sh qb/.
    
    # QB 48-qubit noise model
    cp "${ORIGINDIR}"/src/aer_noise_model_qb.json qb/share/aer_noise_model_qb.json
    
    # QB custom gates for OpenQASM
    cp "${ORIGINDIR}"/src/qblib.inc qb/share/qblib.inc

    # QB qpu_config.json
    cp "${ORIGINDIR}"/src/qpu_config.json qb/share/qpu_config.json
    
    # Add /mnt/qb/share/qpu_config.json
    cp "${ORIGINDIR}"/src/qpu_config.json qb/share/qpu_config.json

    # Generate the tarball
    tar chfj qbos-1_0_1-2021.tar.bz2 qb/bin qb/lib qb/share qb/xacc-local qb/exatn-local qb/install-r.sh qb/Documentation.md qb/qbOS\ Doc*
    mv qbos-1_0_1-2021.tar.bz2 "${ORIGINDIR}"/.
}

function_job_release_package() {
    function_release_package
    exit 0
}

function_release_alpha_package() {
	echo " ***"
    echo " *** Release alpha package : prepare a tarball with binaries + required libraries"
    echo " ***"

    function_perform_checks

    # qbos_core
    function_build_core

    ldconfig >/dev/null 2>&1 

    # qbos_op (VQE and QAOA)
    cd "${ORIGINDIR}"/plugins/optimisation_modules
    rm -rf build && mkdir build && cd build
    cmake .. -DCMAKE_INSTALL_PREFIX="${QB_INSTALL_PATH}"
    make
    make install
    ldconfig >/dev/null 2>&1 

    # qbOS plugins installed to $QB_INSTALL_PATH/lib/plugins
    mkdir -p /mnt/qb/lib/plugins
    cp -pr "${ORIGINDIR}"/plugins/qbos_base_algorithms /mnt/qb/lib/plugins/.
    cp -pr "${ORIGINDIR}"/plugins/qbos_decoder /mnt/qb/lib/plugins/.
    # cp -pr "${ORIGINDIR}"/plugins/QuantumBitStringComparator /mnt/qb/lib/plugins/QuantumBitStringComparator
    cp -pr "${ORIGINDIR}"/plugins/QuantumGates /mnt/qb/lib/plugins/.
    # qbOS visualisation
    cp -pr "${ORIGINDIR}"/plugins/qbos_visual/qbos_vis /mnt/qb/lib/plugins/.
    #
    cp -p "${ORIGINDIR}"/plugins/__init__.py /mnt/qb/lib/plugins/__init__.py

    # qbOS apps - q_chemistry
    cp -p "${ORIGINDIR}"/apps/q_chemistry/external_potential.py "${QB_INSTALL_PATH}/lib"/.
    cp -p "${ORIGINDIR}"/apps/q_chemistry/vqe_calculator.py "${QB_INSTALL_PATH}/lib"/.
    
    # qbOS tket plugin
    cd "${ORIGINDIR}"/plugins/qbos_tket
    rm -rf build && mkdir build && cd build
    cmake .. 
    make -j$(nproc) install
    ldconfig >/dev/null 2>&1 

    # qbOS documentation
    cd /mnt 
    mkdir -p /mnt/qb/share

    cp "${ORIGINDIR}"/assets/user_guide/build/jupyterlab/index.md qb/Documentation.md
    cp -pr "${ORIGINDIR}"/assets/user_guide/build/jupyterlab/qbOS\ Doc* qb/.
    rm -f qb/qbOS\ Documentation*.md

    # HTML documentation
    cp -pr "${ORIGINDIR}"/assets/user_guide/build/html qb/.

    # qbOS entrypoint for Docker container
    cp "${ORIGINDIR}"/alpha/install-r.sh qb/.
    
    # QB 48-qubit noise model
    cp "${ORIGINDIR}"/src/aer_noise_model_qb.json qb/share/aer_noise_model_qb.json
    
    # QB custom gates for OpenQASM
    cp "${ORIGINDIR}"/src/qblib.inc qb/share/qblib.inc

    # Add /mnt/qb/share/qpu_config.json
    cp "${ORIGINDIR}"/src/qpu_config.json qb/share/qpu_config.json

    # Generate the tarball
    tar chfj qbos-2_0_1-2022.tar.bz2 qb/bin qb/lib qb/share qb/xacc-local qb/exatn-local qb/install-r.sh qb/Documentation.md qb/html qb/qbOS\ Doc*
    mv qbos-2_0_1-2022.tar.bz2 "${ORIGINDIR}"/.
}

function_job_release_alpha_package() {
    function_release_alpha_package
    exit 0
}

# Alpha CI tests
function_CI-220329-alpha() {
	echo " ***"
    echo " *** CI-220329-alpha: qbos_core tests (via Pytest)"
    echo " ***"

    function_perform_checks
    
    function_build_core

    cd "${ORIGINDIR}"/src/qbos_core/test
    python3 qbqe_if_model.py &
    pytest test_cases.py --junitxml=report.xml || exit 220329

    echo " ***"
    echo " *** Passed *** - CI-220329-alpha: qbos_core tests (via Pytest)"
    echo " ***"
}

function_job_CI-220329-alpha() {
    function_CI-220329-alpha
    exit 0
}

function_CI-220329-2-alpha() {
	echo " ***"
    echo " *** CI-220329-2-alpha: qbos_core + advice tests (via gtest)"
    echo " ***"

    function_perform_checks
    
    function_build_core

    # cd "${ORIGINDIR}"/src/qbos_core/test
    qbos_core --test --gtest_output='xml:report.xml' || exit 2203292
    plugins/qbos_circuits/tests/UPrimeTester --gtest_output='xml:report-uprime.xml' || exit 2203293
    plugins/qbos_circuits/tests/QPrimeTester --gtest_output='xml:report-qprime.xml' || exit 2203293
    plugins/qbos_circuits/tests/WPrimeTester --gtest_output='xml:report-wprime.xml' || exit 2203293

    echo " ***"
    echo " *** Passed *** - CI-220329-2-alpha: qbos_core + advice tests (via gtest)"
    echo " ***"
}

function_job_CI-220329-2-alpha() {
    function_CI-220329-2-alpha
    exit 0
}

function_CI-220406-alpha() {
	echo " ***"
    echo " *** CI-220406-alpha: qbos_op.vqe() - VQE and QAOA tests (via pytest)"
    echo " ***"

    function_perform_checks
    
    function_build_core

    cd "${ORIGINDIR}"/plugins/optimisation_modules
    rm -rf build && mkdir build && cd build
    cmake .. -DCMAKE_INSTALL_PREFIX="${QB_INSTALL_PATH}"
    make -j
    make install
    
    # VQE pytest requires pyscf plugin to load atomic geometry
    python3 -m pip install pyscf

    cd "${ORIGINDIR}"/plugins/optimisation_modules/VQE/test
    pytest test_cases.py --junitxml=report.xml || exit 220406

    cd "${ORIGINDIR}"/plugins/optimisation_modules/QAOA/test
    pytest test_cases.py --junitxml=report.xml || exit 220406

    echo " ***"
    echo " *** Passed *** - CI-220406-alpha: qbos_op.vqe() - VQE and QAOA tests (via pytest)"
    echo " ***"
}

function_job_CI-220406-alpha() {
    function_CI-220406-alpha
    exit 0
}

function_CI-220406-2-alpha() {
	echo " ***"
    echo " *** CI-220406-2-alpha: qbos_op - VQE and QAOA tests (via gtest)"
    echo " ***"

    function_perform_checks
    
    function_build_core

    cd "${ORIGINDIR}"/plugins/optimisation_modules/VQE/src
    rm -rf build && mkdir build && cd build
    cmake .. -DCMAKE_INSTALL_PREFIX="${QB_INSTALL_PATH}"
    make -j
    make install
    cd "${ORIGINDIR}"/plugins/optimisation_modules/VQE/test
    qbos_op_vqe --test --gtest_output='xml:report-vqe.xml' || exit 220406

    cd "${ORIGINDIR}"/plugins/optimisation_modules/QAOA/src
    rm -rf build && mkdir build && cd build
    cmake .. -DCMAKE_INSTALL_PREFIX="${QB_INSTALL_PATH}"
    make -j
    make install

    cd "${ORIGINDIR}"/plugins/optimisation_modules/optimisation_circuits
    rm -rf build && mkdir build && cd build
    cmake .. -DCMAKE_INSTALL_PREFIX="${QE_INSTALL_PATH}"
    make -j
    make install

    cd "${ORIGINDIR}"/plugins/optimisation_modules/QAOA/test
    qbos_op_qaoa --test --gtest_output='xml:report-qaoa.xml' || exit 220406

    echo " ***"
    echo " *** Passed *** - CI-220406-2-alpha: qbos_op - VQE and QAOA tests (via gtest)"
    echo " ***"
}

function_job_CI-220406-2-alpha() {
    function_CI-220406-2-alpha
    exit 0
}

function_CI-220406-3-alpha() {
	echo " ***"
    echo " *** CI-220406-3-alpha: qbOS Exponential Search with Comparator tests (via Pytest)"
    echo " ***"

    function_perform_checks
    
    function_build_core

    cd "${ORIGINDIR}"/src/qbos_core/test
    pytest exponential_search_tests.py --junitxml=report.xml || exit 220406

    echo " ***"
    echo " *** Passed *** - CI-220406-3-alpha: Exponential Search with Comparator (via Pytest)"
    echo " ***"
}

function_job_CI-220406-3-alpha() {
    function_CI-220406-3-alpha
    exit 0
}

# function_CI-220512-1-alpha() {
#     if [[ -z "${FN_PREREQS}" ]];
#     then
#       source "${ORIGINDIR}"/ci/include/function_prereqs.sh
#     else
#       echo "- Prerequisites already installed...skipping function_prereqs"
#     fi
#     if [[ -z "${FN_SETPATHS}" ]];
#     then
#       source "${ORIGINDIR}"/ci/include/alpha/function_setpaths.sh
#     else
#       echo "- Paths already set...skipping function_setpaths"
#     fi
#     echo " ***"
#     echo " *** CI-220512-1-alpha: qbos quantum decoder tests (via gtest)"
#     echo " ***"

#     cd "${ORIGINDIR}"/src/qbos_core/src
#     rm -rf build && mkdir build && cd build
#     cmake .. -DCMAKE_INSTALL_PREFIX="${QE_INSTALL_PATH}"
#     make -j
#     make install
#     cd "${ORIGINDIR}"/src/qbos_core/src/build/plugins/qbos_algorithms/quantum_decoder/tests
#     ./QuantumDecoderAlgorithmTester --test --gtest_output='xml:report.xml' || exit 220512
#     echo " ***"
#     echo " *** Passed *** - CI-220512-1-alpha: qbos quantum decoder tests (via gtest)"
#     echo " ***"
# }

function_job_CI-220512-1-alpha() {	
    function_CI-220512-1-alpha	
    exit 0	
}

function_DEPS-220406-4-alpha() {
	if [[ ! -z $DO_CHECK ]]; then
	  echo "********"
	  echo "Workaround: installing pyscf from qbOS precompiled wheel during CI test..."
	  echo "********"

	  apt-get update   
	  add-apt-repository -y ppa:ubuntu-toolchain-r/test
	  apt-get install -y openssh-client \
						 libcurl4-openssl-dev \
						 libopenmpi-dev \
						 libopenblas-base libopenblas-dev libblas-dev liblapack-dev libboost-dev libarmadillo-dev \
						 bc \
						 gcc-8 g++-8 gfortran-8 \
						 python3 libpython3-dev python3-pip \
						 libunwind-dev 
	  python3 -m pip install --upgrade pip
	  python3 -m pip install --upgrade cmake
	  python3 -m pip install --upgrade pytest
	  python3 -m pip install ase
	  python3 -m pip install pyscf
	  # python3 -m pip install "${ORIGINDIR}"/tpls/pyscf-wheel/pyscf-2.0.0-cp36-cp36m-linux_x86_64.whl
	  python3 -m pip install openfermion
	  python3 -m pip install openfermionpyscf
	  # more recent cirq versions are known to have problems with openfermion 
	  python3 -m pip install cirq==0.10.0

	  if [[ -z "${FN_SETPATHS}" ]];   then
		source "${ORIGINDIR}"/ci/include/function_setpaths.sh
	  else       
		echo "- Paths already set...skipping function_setpaths"
	  fi  
	fi
}

function_CI-220406-4-alpha() {
    echo " ***"
    echo " *** CI-220406-4-alpha: q_chemistry tests (via Pytest)"
    echo " ***"

	function_DEPS-220406-4-alpha

    function_build_core

    cd "${ORIGINDIR}"/plugins/optimisation_modules
    rm -rf build && mkdir build && cd build
    cmake .. -DCMAKE_INSTALL_PREFIX="${QB_INSTALL_PATH}"
    make -j
    make install

    cd "${ORIGINDIR}"/apps/q_chemistry/test
    cp -p "${ORIGINDIR}"/apps/q_chemistry/external_potential.py "${QB_INSTALL_PATH}/lib"/.
    cp -p "${ORIGINDIR}"/apps/q_chemistry/vqe_calculator.py "${QB_INSTALL_PATH}/lib"/.
    pytest test_cases.py --junitxml=report.xml || exit 220406

    echo " ***"
    echo " *** Passed *** - CI-220406-4-alpha: q_chemistry tests (via Pytest)"
    echo " ***"
}

function_job_CI-220406-4-alpha() {
    function_CI-220406-4-alpha
    exit 0
}

function_DEPS-aws-braket() {
	if [[ ! -z $DO_CHECK ]]; then
	  python3 -m pip install awscli
	  python3 -m pip install boto3    
	  python3 -m pip install amazon-braket-sdk
	fi
}

function_CI-220531-aws-braket-local-sv-dm-alpha() {
	echo " ***"
    echo " *** CI-220531-aws-braket-local-sv-dm-alpha: AWS Braket - Local SV and DM simulator tests (via Pytest)"
    echo " ***"  

    function_perform_checks

    function_build_core

	function_DEPS-aws-braket

    # git clone https://github.com/aws/amazon-braket-sdk-python.git 
    # cd amazon-braket-sdk-python && python3 -m pip install .
    cd "${ORIGINDIR}"/src/qbos_core/test
    pytest test_aws_braket_local.py --junitxml=report.xml || exit 220531
    echo " ***"
    echo " *** Passed *** - CI-220531-aws-braket-local-sv-dm-alpha: AWS Braket - Local SV and DM simulator tests (via Pytest)"
    echo " ***"
}

function_job_CI-220531-aws-braket-local-sv-dm-alpha() {
    function_CI-220531-aws-braket-local-sv-dm-alpha
    exit 0
}

function_CI-220601-aws-braket-sv1-dm1-tn1-alpha() {
	echo " ***"
    echo " *** CI-220601-aws-braket-local-sv1-dm1-tn1-alpha: AWS Braket - hosted SV1, DM1 and TN1 simulator tests (via Pytest)"
    echo " ***"  

    function_perform_checks

    function_build_core

	function_DEPS-aws-braket

    # git clone https://github.com/aws/amazon-braket-sdk-python.git 
    # cd amazon-braket-sdk-python && python3 -m pip install .
    cd "${ORIGINDIR}"/src/qbos_core/test
    pytest test_aws_braket_hosted.py --junitxml=report.xml || exit 220601
    echo " ***"
    echo " *** Passed *** - CI-220601-aws-braket-local-sv1-dm1-tn1-alpha: AWS Braket - hosted SV1, DM1 and TN1 simulator tests (via Pytest)"
    echo " ***"
}

function_job_CI-220601-aws-braket-sv1-dm1-tn1-alpha() {
    function_CI-220601-aws-braket-sv1-dm1-tn1-alpha
    exit 0
}

function_DEPS-220602-1-alpha() {
	if [[ ! -z $DO_CHECK ]]; then
	  # Use the later version of cirq
	  python3 -m pip uninstall -y cirq
	  python3 -m pip install cirq==0.14.1
	  python3 -m pip install qsimcirq==0.12.1
	fi
}

function_CI-220602-1-alpha() {
	echo " ***"
    echo " *** CI-220602-1-alpha: qbos QSIM noise model test"
    echo " ***"

    function_perform_checks

    function_build_core

	function_DEPS-220602-1-alpha

    cd "${ORIGINDIR}"/src/qbos_core/src/build/plugins/qb_qsimcirq/tests
    ./QBCirqNoiseModelTester --gtest_output='xml:report.xml' || exit 220602
    cd "${ORIGINDIR}"/src/qbos_core/src/plugins/qb_qsimcirq/tests
    pytest test_cases.py --junitxml=report.xml || exit 220602
    echo " ***"
    echo " *** Passed *** - CI-220602-1-alpha: qbos QSIM noise model test"
    echo " ***"
}

function_job_CI-220602-1-alpha() {	
    function_CI-220602-1-alpha	
    exit 0	
}

function_CI-220608-1-alpha() {
	echo " ***"
    echo " *** CI-220608-1-alpha: qbOS sparse simulator tests (via gtest)"
    echo " ***"

    function_perform_checks
    
    function_build_core

    cd "${ORIGINDIR}"/src/qbos_core/src/build/plugins/qb_sparse_simulator/tests
    ./QBSparseSimTester --gtest_output='xml:report.xml' || exit 220608
    echo " ***"
    echo " *** Passed *** - CI-220608-1-alpha: qbOS sparse simulator tests (via gtest)"
    echo " ***"
}

function_job_CI-220608-1-alpha() {
    function_CI-220608-1-alpha
    exit 0
}

function_CI-220630-1-alpha() {
	echo " ***"
    echo " *** CI-220630-1-alpha: tket noise-aware placement (via gtest)"
    echo " ***"

    function_perform_checks
    
    function_build_core

    # qbOS tket placement plugin
    cd "${ORIGINDIR}"/plugins/qbos_tket
    rm -rf build && mkdir build && cd build
    cmake .. 
    make -j$(nproc) install

    cd "${ORIGINDIR}"/plugins/qbos_tket/build/tests
    ./QBTketPlacementTester --gtest_output='xml:report.xml' || exit 220630
    echo " ***"
    echo " *** Passed *** - CI-220630-1-alpha: tket noise-aware placement (via gtest)"
    echo " ***"
}

function_job_CI-220630-1-alpha() {
    function_CI-220630-1-alpha
    exit 0
}

## Add additional alpha release tests here

function_CI-220407-build-xacc-aermps-alpha() {
	echo " ***"
    echo " *** CI-220407-build-xacc-aermps-alpha: Full build of XACC, TNQVM, ExaTN, AER MPS, qbOS core"
    echo " ***"  

    function_perform_checks

    cd "${ORIGINDIR}"  
    export QE_DIR="${ORIGINDIR}/.."
    source alpha/install.sh
    qbos_core --test --gtest_output='xml:report-fullbuild.xml' || exit 220407
}

function_job_CI-220407-build-xacc-aermps-alpha() {
    function_CI-220407-build-xacc-aermps-alpha
    exit 0
}

function_job_stage_aws() {
    function_release_package
    cd "${ORIGINDIR}"/ci/include
    chmod 600 staging-aws.key
    scp -i staging-aws.key -o StrictHostKeyChecking=no "${ORIGINDIR}"/qbos-1_0_1-2021.tar.bz2 ubuntu@ec2-3-25-57-92.ap-southeast-2.compute.amazonaws.com:~/.
    ssh -i staging-aws.key -t -o StrictHostKeyChecking=no ubuntu@ec2-3-25-57-92.ap-southeast-2.compute.amazonaws.com -- sudo tar xjf qbos-1_0_1-2021.tar.bz2
    exit 0
}

function_job_build-test-all-alpha() {
    function_CI-220407-build-xacc-aermps-alpha

}

export ORIGINDIR=$(pwd)

######
args=("$@")
if [[ -n ${args[0]} ]];
then
  function_job_"${args[0]}"
fi

# Install packages 
source "${ORIGINDIR}"/ci/include/function_prereqs.sh

# Set paths
source "${ORIGINDIR}"/ci/include/function_setpaths.sh
