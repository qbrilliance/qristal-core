export FN_PREREQS=1
apt-get update
apt install software-properties-common
add-apt-repository -y ppa:ubuntu-toolchain-r/test
apt-get update
apt-get install -y openssh-client \
                   libcurl4-openssl-dev \
                   libopenmpi-dev \
                   libopenblas-base libopenblas-dev libblas-dev liblapack-dev libboost-dev libarmadillo-dev \
                   bc \
                   gcc-8 g++-8 gfortran-8 \
                   python3 libpython3-dev python3-pip \
                   libunwind-dev libgl1
python3 -m pip install --upgrade pip
python3 -m pip install --upgrade cmake
python3 -m pip install --upgrade pytest
python3 -m pip install conan
