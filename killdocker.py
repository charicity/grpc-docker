#!/usr/bin/env python3
import os
import subprocess

dockerName = "grpc-dev:latest"
containerName = "mycontainer"
execution = f"docker ps | grep '{containerName}' | awk '{{print $1}}'"

print("executing:", execution)
dpid = subprocess.check_output(execution, shell=True, text=True).strip()
print("container id:", dpid)
os.system(f"docker stop {dpid}")
print("docker process killed")

op = input("remove docker container? (y/n) ")
if op == "y":
    os.system(f"docker rm {dpid}")
    print("docker container removed")
