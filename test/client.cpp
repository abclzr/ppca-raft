//
// Created by abclzr on 2019/8/5.
//

#include <iostream>
#include <algorithm>
#include "../include/raft/Server/Server.h"
#include "../include/raft/Client/Client.h"


int main(int argc, char *argv[]) {
    std::ios::sync_with_stdio(false);

    raft::Client client(std::string(CMAKE_SOURCE_DIR) + "/cmake-build-debug/example/ClientConfig" + std::string(argv[1]) + ".json");
    client.Run();

    std::string op, k, v;
    uint32_t cnt = 0;
    while (std::cin.peek() != EOF) {
        std::cerr << "ID : " << ++cnt << std::endl;
        std::cin >> op;
        if (op == "put") {
            std::cin >> k >> v;
            client.Put(k, v);
        } else {
            std::cin >> k;
            std::cout << k << " ";
            client.Get(k);
        }
        while (std::isspace(std::cin.peek()))
            std::cin.get();
    }

    return 0;
}