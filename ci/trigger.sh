#!/bin/bash
# Copyright (c) 2021 Quantum Brilliance Pty Ltd
#
# For internal QB use only
# Usage:
#      git clone https://gitlab.com/qbau/qbos.git
#      cd qbos && source ci/trigger.sh
#
# This script runs a sequence of tests (unit, integration, regression)
# for qbOS inside the xacc/xacc:latest Docker container
# released from Oak Ridge National Lab
#
# Check the script has been sourced
if [[ "${BASH_SOURCE[0]}" == "${0}" ]]; then
   echo "Please source this script:"
   echo
   echo "  git clone https://gitlab.com/qbau/qbos.git"
   echo "  cd qbos && source ci/trigger.sh"
   echo
   exit 1
fi
origindir=$(pwd)
args=("$@")
#
sudo docker run --security-opt seccomp=unconfined --init --rm  --name qbemct -v $PWD:/home/dev --entrypoint=/home/dev/ci/in_container_tests.sh xacc/deploy-base "$@"
