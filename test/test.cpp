#include <iostream>
#include <algorithm>
#include "../include/raft/Server/Server.h"
#include "../include/raft/Client/Client.h"


int main() {
    std::ios::sync_with_stdio(false);
    std::cout << "Hello, world!" << std::endl;
    raft::Client client(std::string(CMAKE_SOURCE_DIR) + "/cmake-build-debug/example/RaftConfig0.json");
    raft::Server server1(std::string(CMAKE_SOURCE_DIR) + "/cmake-build-debug/example/RaftConfig1.json", 2, "");
    raft::Server server2(std::string(CMAKE_SOURCE_DIR) + "/cmake-build-debug/example/RaftConfig2.json", 2, "");

    client.Run();
    server1.Run();
    server2.Run();

    sleep(10);
    std::cout << "Sleep Over." << std::endl;

    client.Stop();
    std::cout << "Finish Client" << std::endl;
    server1.Stop();
    std::cout << "Finish Server1" << std::endl;
    server2.Stop();
    std::cout << "Finish Server2" << std::endl;

    return 0;
}