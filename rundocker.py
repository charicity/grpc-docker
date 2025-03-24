#!/usr/bin/env python3
import os
import subprocess

dockerName = "grpc-dev:latest"
containerName = "mycontainer"
query_container = f"docker ps -a | grep '{containerName}'"

try:
    dpid = subprocess.check_output(query_container, shell=True, text=True).strip()
    print("Container already exists")
    os.system(f"docker start {containerName}")
except subprocess.CalledProcessError:
    print("Container does not exist, creating and running it")
    create_and_run = f"docker run -d --name {containerName} {dockerName} tail -f /dev/null"
    print("create_and_run: ", create_and_run)
    os.system(create_and_run)