#include <iostream>
#include <algorithm>
#include "../include/raft/Server/Server.h"
#include "../include/raft/Client/Client.h"


int main(int argc, char *argv[]) {
    std::ios::sync_with_stdio(false);
    std::cout << "Hello, world!" << std::endl;
    raft::Client client1(std::string(CMAKE_SOURCE_DIR) + "/cmake-build-debug/example/ClientConfig1.json");
    raft::Client client2(std::string(CMAKE_SOURCE_DIR) + "/cmake-build-debug/example/ClientConfig2.json");
    raft::Client client3(std::string(CMAKE_SOURCE_DIR) + "/cmake-build-debug/example/ClientConfig3.json");
    raft::Client client4(std::string(CMAKE_SOURCE_DIR) + "/cmake-build-debug/example/ClientConfig4.json");
    raft::Client client5(std::string(CMAKE_SOURCE_DIR) + "/cmake-build-debug/example/ClientConfig0.json");
    raft::Server server1(std::string(CMAKE_SOURCE_DIR) + "/cmake-build-debug/example/RaftConfig1.json");
    raft::Server server2(std::string(CMAKE_SOURCE_DIR) + "/cmake-build-debug/example/RaftConfig2.json");
    raft::Server server3(std::string(CMAKE_SOURCE_DIR) + "/cmake-build-debug/example/RaftConfig3.json");
    raft::Server server4(std::string(CMAKE_SOURCE_DIR) + "/cmake-build-debug/example/RaftConfig4.json");
    raft::Server server5(std::string(CMAKE_SOURCE_DIR) + "/cmake-build-debug/example/RaftConfig0.json");

    client1.Run();
    client2.Run();
    client3.Run();
    client4.Run();
    client5.Run();
    server1.Run();
    server2.Run();
    server3.Run();
    server4.Run();
    server5.Run();

    sleep(1);

    std::vector<std::pair<std::string, std::string>> p;
    for (int i = 0; i < 10; ++i) {
        std::string a = std::to_string(rand());
        std::string b = std::to_string(rand());
        p.emplace_back(std::make_pair(a, b));
        std::cout << a + "  " + b << std::endl;
        client1.Put(a, b);
        client2.Put(a, b);
        client3.Put(a, b);
        client4.Put(a, b);
        client5.Put(a, b);
    }

    sleep(1);

    for (int i = 0; i < 10; ++i) {
        client1.Get(p[i].first);
        client2.Get(p[i].first);
        client3.Get(p[i].first);
        client4.Get(p[i].first);
        client5.Get(p[i].first);
    }

    sleep(1);

    std::cout << "Sleep Over." << std::endl;

    client1.Stop();
    client2.Stop();
    client3.Stop();
    client4.Stop();
    client5.Stop();
    server1.Stop();
    server2.Stop();
    server3.Stop();
    server4.Stop();
    server5.Stop();

    server1.WriteLog(std::cout);
    server2.WriteLog(std::cout);
    server3.WriteLog(std::cout);
    server4.WriteLog(std::cout);
    server5.WriteLog(std::cout);

    return 0;
}