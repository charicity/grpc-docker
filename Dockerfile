FROM ubuntu:22.04
WORKDIR /home

# update apt
RUN apt update
# Install c++
RUN apt install -y build-essential autoconf libtool pkg-config
RUN apt install -y cmake

# Install dev-tools
RUN apt install -y git

# Install tar
RUN apt install -y tar

# Prepare grpc path
ENV MY_INSTALL_DIR=/usr/local 
RUN echo "${MY_INSTALL_DIR}"
RUN mkdir -p ${MY_INSTALL_DIR}
ENV PATH="$MY_INSTALL_DIR/bin:$PATH"

# Clone grpc
# RUN git clone --recurse-submodules -b v1.71.0 --depth 1 --shallow-submodules https://github.com/grpc/grpc
COPY ./zips/grpc.tar.gz .
RUN tar -xzvf ./grpc.tar.gz && rm grpc.tar.gz
RUN rm -rf ./grpc/cmake/build
RUN cd grpc && \
  mkdir -p cmake/build && \
  cd cmake/build && \
  cmake -DgRPC_INSTALL=ON \
  -DgRPC_BUILD_TESTS=OFF \
  -DCMAKE_CXX_STANDARD=17 \
  -DCMAKE_INSTALL_PREFIX=$MY_INSTALL_DIR \
  ../.. && \
  make -j 8 && \
  make install
RUN rm -r ./grpc

# install boost
RUN apt install -y libboost-all-dev

# install jwt-cpp
RUN apt install -y libssl-dev
RUN apt install -y git cmake build-essential && \
  git clone https://github.com/Thalhammer/jwt-cpp.git /tmp/jwt-cpp && \
  cd /tmp/jwt-cpp && mkdir build && cd build && \
  cmake .. && make -j8 && make install && \
  cd / && rm -rf /tmp/jwt-cpp

# install clangd
RUN apt install -y clangd 

# install googleapis
RUN git clone --depth=1 https://github.com/googleapis/googleapis.git /usr/local/include/googleapis

RUN cd /usr/local/include/googleapis && \
  make -j8 LANGUAGE=cpp all

# -----cache end-----
COPY . .
RUN rm -r ./zips