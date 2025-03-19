#!/usr/bin/env python3
import os
import subprocess

dockerName = "grpc-dev:latest"
containerName = "mycontainer"
query_container = f"docker ps -a | grep '{containerName}'"

dpid = subprocess.check_output(query_container, shell=True, text=True).strip()
if dpid:
  print("container already exists")
  os.system(f"docker start {containerName}")
else:
  create_and_run = f"docker run -it --name {containerName} {dockerName} tail -f /dev/null"
  print("create_and_run: ", create_and_run)
  subprocess.Popen(create_and_run, shell=True)