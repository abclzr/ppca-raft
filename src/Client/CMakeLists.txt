add_library(${PROJECT_NAME}-client
  ${CMAKE_SOURCE_DIR}/include/raft/Client/client.h
  Client.cpp
  )

target_compile_features(${PROJECT_NAME}-client PUBLIC cxx_std_14)
target_include_directories(${PROJECT_NAME}-client
  PUBLIC ${CMAKE_SOURCE_DIR}/include/)
target_link_libraries(${PROJECT_NAME}-client PUBLIC ${PROJECT_NAME}-common)
target_compile_options(${PROJECT_NAME}-client PRIVATE -o2 -Wall -pthread)

find_package(Boost REQUIRED)
target_include_directories(${PROJECT_NAME}-client PRIVATE ${Boost_INCLUDE_DIR})