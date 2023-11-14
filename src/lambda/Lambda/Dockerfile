FROM nvidia/cuda:11.6.2-devel-ubuntu20.04

ENV DEBIAN_FRONTEND noninteractive

RUN apt -y update && apt-get install -y sudo vim cmake git tmux libblas-dev liblapack-dev libspdlog-dev \
    python3 libcurl4-openssl-dev libunwind-dev libpython3-dev python3-pip openmpi-bin openmpi-common libopenmpi-dev \
    cuda cuquantum cuquantum-dev cuquantum-doc \
    && python3 -m pip install numpy qiskit flask

ENV \
  DEPS_DIR=/mnt/qb

RUN mkdir -p ${DEPS_DIR}
WORKDIR ${DEPS_DIR}

ENV \
  JSON_DIR=${DEPS_DIR}/nlohmann_json-local \

RUN git clone https://github.com/nlohmann/json.git \
  && cd json \
  && git reset --hard c8ea63a \
  && git submodule init \
  && git submodule update --init --recursive \
  && cmake -B build . \
  -DJSON_BuildTests=OFF \
  -DCMAKE_INSTALL_PREFIX=${JSON_DIR} \
  && cmake --build build -- -j$(nproc) \
  && cmake --install build \
  && rm -rf ../json


COPY server.py /root/server.py

ENTRYPOINT ["tail", "-f", "/dev/null"]
