add_executable(${PROJECT_NAME}-test test.cpp)

# link
target_link_libraries(${PROJECT_NAME}-test
  PRIVATE ${PROJECT_NAME}-client ${PROJECT_NAME}-server
  )

add_definitions(-DCMAKE_SOURCE_DIR="${CMAKE_SOURCE_DIR}")
