RUN git clone --recurse-submodules -b v1.71.0 --depth 1 --shallow-submodules https://github.com/grpc/grpc
tar -cf grpc.tar.gz grpc/