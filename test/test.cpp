#include <iostream>
#include <algorithm>
#include "../include/raft/Server/Server.h"
#include "../include/raft/Client/Client.h"


int main() {
    std::cout << "Hello, world!" << std::endl;
    raft::Client client(std::string(CMAKE_SOURCE_DIR) + "RaftConfig0.json");
    raft::Server server1(std::string(CMAKE_SOURCE_DIR) + "RaftConfig1.json", 2);
    raft::Server server2(std::string(CMAKE_SOURCE_DIR) + "RaftConfig2.json", 2);
    boost::thread t1(&raft::Server::Run, &server1);
    boost::thread t2(&raft::Server::Run, &server2);

    for (int i = 1; i <= 10; ++i) {
        std::string a = std::to_string(rand());
        std::string b = std::to_string(rand());
        client.Put(a, b);
        sleep(1);
    }

    server1.work_is_done = true;
    server1.cv.notify_one();
    server2.work_is_done = true;
    server2.cv.notify_one();

    if (t1.joinable()) t1.join();
    if (t2.joinable()) t2.join();

    return 0;
}