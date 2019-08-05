#include <iostream>
#include <algorithm>
#include "../include/raft/Server/Server.h"
#include "../include/raft/Client/Client.h"


int main(int argc, char *argv[]) {
    std::ios::sync_with_stdio(false);

    raft::Server server(argv[1]);
    server.Run();
    while (1) {

    }
    /*
    std::cout << "Hello, world!" << std::endl;
    raft::Client client(std::string(CMAKE_SOURCE_DIR) + "/cmake-build-debug/example/RaftConfig0.json");
    raft::Server server1(std::string(CMAKE_SOURCE_DIR) + "/cmake-build-debug/example/RaftConfig1.json");
    raft::Server server2(std::string(CMAKE_SOURCE_DIR) + "/cmake-build-debug/example/RaftConfig2.json");
    raft::Server server3(std::string(CMAKE_SOURCE_DIR) + "/cmake-build-debug/example/RaftConfig3.json");
    raft::Server server4(std::string(CMAKE_SOURCE_DIR) + "/cmake-build-debug/example/RaftConfig4.json");
    raft::Server server5(std::string(CMAKE_SOURCE_DIR) + "/cmake-build-debug/example/RaftConfig5.json");

//    client.Run();
    server1.Run();
    server2.Run();
    server3.Run();
    server4.Run();
    server5.Run();

    sleep(1);

    for (int i = 1; i <= 10; ++i) {
        std::string a = std::to_string(rand());
        std::string b = std::to_string(rand());
        std::cout << a + "  " + b << std::endl;
        client.Put(a, b);
        sleep(0.3);
    }

    sleep(1);
    server4.Stop();

    sleep(1);

    server4.WriteLog(std::cout);
    server4.Run();

    sleep(1);

    for (int i = 1; i <= 0; ++i) {
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
    server2.Stop();
    server3.Stop();
    server4.Stop();
    server5.Stop();

    server1.WriteLog(std::cout);
    server2.WriteLog(std::cout);
    server3.WriteLog(std::cout);
    server4.WriteLog(std::cout);
    server5.WriteLog(std::cout);
*/
    return 0;
}