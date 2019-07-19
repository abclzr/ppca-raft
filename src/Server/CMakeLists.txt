set(SERVER_SRC
  ${CMAKE_SOURCE_DIR}/include/raft/Server/ExternalRpcService.h

  ExternalRpcService.cpp
  )

add_library(${PROJECT_NAME}-server ${SERVER_SRC})

target_compile_features(${PROJECT_NAME}-server PUBLIC cxx_std_14)
target_include_directories(${PROJECT_NAME}-server
  PUBLIC ${CMAKE_SOURCE_DIR}/include/)
target_link_libraries(${PROJECT_NAME}-server PUBLIC ${PROJECT_NAME}-common)
target_compile_options(${PROJECT_NAME}-server PRIVATE -o2 -Wall -pthread)
