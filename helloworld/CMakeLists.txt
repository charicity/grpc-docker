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
# 定义所有的 .proto 文件
set(proto_files
  "./protos/helloworld.proto"
  # 如果有其他 .proto 文件，可以在这里添加
)

# 遍历每个 .proto 文件
foreach(proto_file ${proto_files})
  # 获取 .proto 文件的绝对路径和目录
  get_filename_component(proto_abs "${proto_file}" ABSOLUTE)
  get_filename_component(proto_path "${proto_abs}" PATH)

  # 定义生成的文件路径
  get_filename_component(proto_name "${proto_file}" NAME_WE) # 获取文件名（无扩展名）
  set(proto_srcs "${CMAKE_CURRENT_BINARY_DIR}/${proto_name}.pb.cc")
  set(proto_hdrs "${CMAKE_CURRENT_BINARY_DIR}/${proto_name}.pb.h")
  set(grpc_srcs "${CMAKE_CURRENT_BINARY_DIR}/${proto_name}.grpc.pb.cc")
  set(grpc_hdrs "${CMAKE_CURRENT_BINARY_DIR}/${proto_name}.grpc.pb.h")
  set(proto_pb "${CMAKE_CURRENT_BINARY_DIR}/${proto_name}.pb")

  # 添加生成命令
  add_custom_command(
    OUTPUT "${proto_srcs}" "${proto_hdrs}" "${grpc_srcs}" "${grpc_hdrs}" "${proto_pb}"
    COMMAND ${_PROTOBUF_PROTOC}
    ARGS --grpc_out "${CMAKE_CURRENT_BINARY_DIR}"
         --cpp_out "${CMAKE_CURRENT_BINARY_DIR}"
         --include_imports
         --include_source_info
         --descriptor_set_out="${proto_name}.pb"
         -I "${proto_path}"
         -I "${GOOGLEAPIS_PB_DIR}" # googleapis's protos
         --plugin=protoc-gen-grpc="${_GRPC_CPP_PLUGIN_EXECUTABLE}"
         "${proto_abs}"
    DEPENDS "${proto_abs}"
  )

  # 将生成的文件添加到列表中
  list(APPEND all_proto_srcs "${proto_srcs}" "${grpc_srcs}")
  list(APPEND all_proto_hdrs "${proto_hdrs}" "${grpc_hdrs}")
endforeach()

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

# 添加库
add_library(hw_grpc_proto
  ${all_proto_srcs}
  ${all_proto_hdrs}
  ${google_api_srcs}
  ${google_api_hdrs}
)

# 链接库
target_link_libraries(hw_grpc_proto
  absl::check
  ${_REFLECTION}
  ${_GRPC_GRPCPP}
  ${_PROTOBUF_LIBPROTOBUF}
)

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
