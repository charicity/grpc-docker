#!/usr/bin/env python3
import os
buildCommand = "DOCKER_BUILDKIT=0 docker build -t grpc-dev:latest ."

print("check your dockerfile & .dockerignore and press enter to continue")
print("buildCommand: ", buildCommand)
input()
os.system(buildCommand)