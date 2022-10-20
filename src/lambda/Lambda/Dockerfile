FROM nvidia/cuda:11.6.1-devel-ubuntu20.04

ENV DEBIAN_FRONTEND noninteractive

RUN apt -y update && apt-get install -y sudo vim cmake git libblas-dev liblapack-dev \
    python3 libcurl4-openssl-dev libunwind-dev libpython3-dev python3-pip openmpi-bin openmpi-common libopenmpi-dev \
    cuquantum cuquantum-dev cuquantum-doc \
    && python3 -m pip install conan qiskit flask

ENV LD_PRELOAD "/opt/nvidia/cuquantum/lib/libcustatevec.so"
COPY server.py /root/server.py

ENTRYPOINT ["tail", "-f", "/dev/null"]
