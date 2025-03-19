#!/usr/bin/env python3
import os

dockerName = "grpc-dev:latest"
containerName = "mycontainer"
execution = f"docker run -it --name {containerName} {dockerName} tail -f /dev/null"

print("executing: ", execution)
os.system(execution)