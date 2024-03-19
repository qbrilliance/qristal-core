#!/bin/bash
# path looks like: /home/gitlab-runner/builds/EE2peA3u/0/qbau/software-and-apps/core
# we get the /EE2peA3u/0/ substring and replace it with the current path on the runner

oldpath=$(cat build/cmake_install.cmake | awk -v FS="(builds|qbau)" '{print $2; exit}')
newpath=$(pwd | awk -v FS="(builds|qbau)" '{print $2; exit}')

echo $oldpath $newpath

sed -ie "s|$oldpath|$newpath|g" build/afterCPMAddPackage.cmake
sed -ie "s|$oldpath|$newpath|g" build/coreDependencies.cmake
sed -ie "s|$oldpath|$newpath|g" build/cmake_install.cmake

for i in ./build/_deps/*build/cmake_install.cmake; do
    sed -ie "s|$oldpath|$newpath|g" $i
done

# restore old path in qasm-simulator
sed -ie "57s|$newpath|$oldpath|g" build/_deps/qasm_simulator-build/cmake_install.cmake