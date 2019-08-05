add_executable(${PROJECT_NAME}-test test.cpp)
add_executable(${PROJECT_NAME}-svr server.cpp)
add_executable(${PROJECT_NAME}-clt client.cpp)
# link
target_link_libraries(${PROJECT_NAME}-test
        PRIVATE ${PROJECT_NAME}-client ${PROJECT_NAME}-server
        )
target_link_libraries(${PROJECT_NAME}-svr
        PRIVATE ${PROJECT_NAME}-client ${PROJECT_NAME}-server
        )
target_link_libraries(${PROJECT_NAME}-clt
        PRIVATE ${PROJECT_NAME}-client ${PROJECT_NAME}-server
        )

add_definitions(-DCMAKE_SOURCE_DIR="${CMAKE_SOURCE_DIR}")
