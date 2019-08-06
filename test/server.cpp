//
// Created by abclzr on 2019/8/5.
//

#include <iostream>
#include <algorithm>
#include "../include/raft/Server/Server.h"
#include "../include/raft/Client/Client.h"


int main(int argc, char *argv[]) {
    std::ios::sync_with_stdio(false);

    raft::Server server(std::string(CMAKE_SOURCE_DIR) + "/cmake-build-debug/example/RaftConfig" + std::string(argv[1]) + ".json");
    server.Run();
    while (1) {

    }
    return 0;
}