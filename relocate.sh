#!/bin/bash
# path looks like: /home/gitlab-runner/builds/EE2peA3u/0/qbau/software-and-apps/core
# we get the /EE2peA3u/0/ substring and replace it with the current path on the runner
# Note that this *does not* fix the value of SDK_DIR compiled into the core binary.

oldpath=$(cat build/cmake_install.cmake | awk -v FS="(builds|qbau)" '{print $2; exit}')
newpath=$(pwd | awk -v FS="(builds|qbau)" '{print $2; exit}')

echo "Switching paths from $oldpath to $newpath"

sed -ie "s|$oldpath|$newpath|g" build/afterCPMAddPackage.cmake
sed -ie "s|$oldpath|$newpath|g" build/coreDependencies.cmake

for i in $(find ./build -name "cmake_install.cmake"); do
    sed -ie "/OLD_RPATH/n;s|$oldpath|$newpath|g" $i
done

for i in build/*Tests; do
    patchelf --set-rpath $(patchelf --print-rpath $i | sed -e "s|$oldpath|$newpath|g") $i
done
