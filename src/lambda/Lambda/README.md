In this directory are resources required to start a standalone AER simulator on the QB Lambda workstation.
This procedure can also be adapted to GPU-based AWS instances.  This system is currently experimental, and
is likely to change between the beta release and version 1.0.

# File Descriptions

- `Dockerfile`: based on the NVIDIA's docker image and install some additional dependencies for the standalone AER simulator.

- `server.py`: simple HTTP server to handle job requests (create QObj and invoke the `qasm_simulator` executable). This file will be copied into the Docker image during the build procedure.

# Instructions

On the Lambda machine, navigating to this folder:

- Step 1: Build and run the Docker image (attaching all GPUs to the Docker container):
```
docker build --tag lambda .
docker run -d -t --gpus all lambda
```

- Step 2: Compile AER inside the container:

```
cd /mnt/qb
git clone https://github.com/Qiskit/qiskit-aer.git
cd qiskit-aer/
git reset --hard 0.10.4 
git submodule init
git submodule update --init --recursive
cmake -B build . -DAER_THRUST_BACKEND=CUDA -DBUILD_TESTS=True -DAER_MPI=True -DDISABLE_CONAN=ON -DCMAKE_PREFIX_PATH=/mnt/qb -DCMAKE_BUILD_TYPE=Release ..
cd build && make
make install
```

- Step 3: Start the server inside the container (at `localhost:5000`) in detachable mode (e.g.
  tmux):

```
python3 server.py
```

- Step 4: ssh from inside the container to the permanent AWS reverse proxy instance (forward port 5000) in detachable mode (e.g., tmux):

```
ssh -o ServerAliveInterval=30 -R 5000:localhost:5000 ubuntu@ec2-3-26-79-252.ap-southeast-2.compute.amazonaws.com
```

- Step 5: To test using the qb-lambda backend, run `run_lambda.py` and `run_async.py` in the examples folder.

# Notes

- The Lambda simulator accelerator is now available at: ec2-3-26-79-252.ap-southeast-2.compute.amazonaws.com.  Note that this server will be available throughout the beta testing period as a service to testers; if it seems to be down, please let us know and we will restart it.

- Step 3 and 4 above should be executed in detachable mode, i.e., the REST server and ssh tunnel should still be alive inside the container after we detach those terminals.

- The AWS proxy server (NGINX) is a `t2.micro` instance with the public URL: ec2-3-26-79-252.ap-southeast-2.compute.amazonaws.com. The `/etc/nginx/conf.d/server.conf` settings are:

```
upstream tunnel {
  server 127.0.0.1:5000;
}

server {
  server_name ec2-3-26-79-252.ap-southeast-2.compute.amazonaws.com;

  location / {
    proxy_set_header X-Real-IP $remote_addr;
    proxy_set_header X-Forwarded-For $proxy_add_x_forwarded_for;
    proxy_set_header Host $http_host;
    proxy_redirect off;
    proxy_pass http://tunnel;
  }
}
```

- Calling the qb-lambda backend makes a json request to the server, a qobj.json specifying the job and a config.json specifying settings. Then qasm_simulator is used for execution. 
