cmake_minimum_required(VERSION 3.8)

project(HelloWorld C CXX)

# compile_commands
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(./cmake/common.cmake)

# C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

# Proto file
get_filename_component(hw_proto "./protos/helloworld.proto" ABSOLUTE)
get_filename_component(hw_proto_path "${hw_proto}" PATH)

# facts
get_filename_component(GOOGLEAPIS_PB_DIR "/usr/local/include/googleapis" ABSOLUTE)
get_filename_component(GOOGLEAPIS_HDRS_DIR "/usr/local/include/googleapis/gens" ABSOLUTE)

# Generated sources
set(hw_proto_srcs "${CMAKE_CURRENT_BINARY_DIR}/helloworld.pb.cc")
set(hw_proto_hdrs "${CMAKE_CURRENT_BINARY_DIR}/helloworld.pb.h")
set(hw_grpc_srcs "${CMAKE_CURRENT_BINARY_DIR}/helloworld.grpc.pb.cc")
set(hw_grpc_hdrs "${CMAKE_CURRENT_BINARY_DIR}/helloworld.grpc.pb.h")
add_custom_command(
      OUTPUT "${hw_proto_srcs}" "${hw_proto_hdrs}" "${hw_grpc_srcs}" "${hw_grpc_hdrs}"
      COMMAND ${_PROTOBUF_PROTOC}
      ARGS --grpc_out "${CMAKE_CURRENT_BINARY_DIR}"
        --cpp_out "${CMAKE_CURRENT_BINARY_DIR}"
        -I "${hw_proto_path}"
        -I "${GOOGLEAPIS_PB_DIR}" # googleapis's protos
        --plugin=protoc-gen-grpc="${_GRPC_CPP_PLUGIN_EXECUTABLE}"
        "${hw_proto}"
      DEPENDS "${hw_proto}")

# Include generated *.pb.h files
include_directories("${CMAKE_CURRENT_BINARY_DIR}")
include_directories("${GOOGLEAPIS_HDRS_DIR}") # and googleapis's headers

# debug print
message(STATUS "[INFO] _REFLECTION: ${_REFLECTION}")
message(STATUS "[INFO] _PROTOBUF_LIBPROTOBUF: ${_PROTOBUF_LIBPROTOBUF}")
message(STATUS "[INFO] _GRPC_GRPCPP: ${_GRPC_GRPCPP}")
message(STATUS "[INFO] GOOGLEAPIS_PB_DIR: ${GOOGLEAPIS_PB_DIR}")

set(google_api_srcs
  "${GOOGLEAPIS_HDRS_DIR}/google/api/annotations.pb.cc"
  "${GOOGLEAPIS_HDRS_DIR}/google/api/http.pb.cc")
set(google_api_hdrs
  "${GOOGLEAPIS_HDRS_DIR}/google/api/annotations.pb.h"
  "${GOOGLEAPIS_HDRS_DIR}/google/api/http.pb.h")

message(STATUS "[INFO] google_api_srcs: ${google_api_srcs}")

# hw_grpc_proto
add_library(hw_grpc_proto
  ${hw_grpc_srcs}
  ${hw_grpc_hdrs}
  ${hw_proto_srcs}
  ${hw_proto_hdrs}
  ${google_api_srcs}
  ${google_api_hdrs})
target_link_libraries(hw_grpc_proto
  absl::check
  ${_REFLECTION}
  ${_GRPC_GRPCPP}
  ${_PROTOBUF_LIBPROTOBUF})

# Targets greeter_[async_](client|server)
foreach(_target
  greeter_client greeter_server)
  add_executable(${_target} "${_target}.cc")
  target_link_libraries(${_target}
    hw_grpc_proto
    absl::check
    absl::flags
    absl::flags_parse
    absl::log
    ${_REFLECTION}
    ${_GRPC_GRPCPP}
    ${_PROTOBUF_LIBPROTOBUF})
endforeach()
