add_library(${PROJECT_NAME}-common
  ${CMAKE_SOURCE_DIR}/include/raft/Common/external.grpc.pb.h
  ${CMAKE_SOURCE_DIR}/include/raft/Common/external.pb.h
  external.grpc.pb.cc
  external.pb.cc
  )

target_compile_features(${PROJECT_NAME}-common PUBLIC cxx_std_14)
target_include_directories(${PROJECT_NAME}-common
  PUBLIC
  ${CMAKE_SOURCE_DIR}/include/
  PRIVATE
  ${CMAKE_SOURCE_DIR}/include/raft/Common
  )

find_package(Protobuf REQUIRED)
find_package(GRPC REQUIRED)
target_include_directories(${PROJECT_NAME}-common
  PUBLIC ${PROTOBUF_INCLUDE_DIR} ${GRPC_INCLUDE_DIR}
  )
target_link_libraries(${PROJECT_NAME}-common
  PUBLIC ${PROTOBUF_LIBRARIES} ${GRPC_LIBRARIES}
  )
target_compile_options(${PROJECT_NAME}-common PRIVATE -o2 -Wall -pthread)

if (GEN_GRPC)
  message(STATUS "generating gRPC files...")
  execute_process(
    COMMAND bash ${CMAKE_SOURCE_DIR}/protobuf/gen_grpc_files.sh
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/protobuf)
endif ()