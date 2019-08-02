#include <iostream>
#include <algorithm>
#include "../include/raft/Server/Server.h"
#include "../include/raft/Client/Client.h"


int main() {
    std::ios::sync_with_stdio(false);
    std::cout << "Hello, world!" << std::endl;
    raft::Client client(std::string(CMAKE_SOURCE_DIR) + "/cmake-build-debug/example/RaftConfig0.json");
    raft::Server server1(std::string(CMAKE_SOURCE_DIR) + "/cmake-build-debug/example/RaftConfig1.json");
    raft::Server server2(std::string(CMAKE_SOURCE_DIR) + "/cmake-build-debug/example/RaftConfig2.json");
    raft::Server server3(std::string(CMAKE_SOURCE_DIR) + "/cmake-build-debug/example/RaftConfig3.json");

//    client.Run();
    server1.Run();
    server2.Run();
    server3.Run();

    sleep(1);

    for (int i = 1; i <= 30; ++i) {
        std::string a = std::to_string(rand());
        std::string b = std::to_string(rand());
        std::cout << a + "  " + b << std::endl;
        client.Put(a, b);
        sleep(0.3);
    }

    sleep(1);
    std::cout << "Sleep Over." << std::endl;

//    client.Stop();
//    std::cout << "Finish Client" << std::endl;
    server1.Stop();
    std::cout << "Finish Server1" << std::endl;
    server2.Stop();
    std::cout << "Finish Server2" << std::endl;
    server3.Stop();
    std::cout << "Finish Server3" << std::endl;

    server1.WriteLog(std::cout);
    server2.WriteLog(std::cout);
    server3.WriteLog(std::cout);

    return 0;
}