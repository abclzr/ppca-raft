//
// Created by abclzr on 2019/8/5.
//

#include <iostream>
#include <algorithm>
#include "../include/raft/Server/Server.h"
#include "../include/raft/Client/Client.h"


int main(int argc, char *argv[]) {
    std::ios::sync_with_stdio(false);

    raft::Client client(argv[1]);
    client.Run();

    sleep(1);
    for (int i = 1; i <= 10000; ++i) {
        client.Put(std::to_string(i), std::to_string(i));
    }

    for (int i = 1; i <= 10000; ++i) {
        client.Get(std::to_string(i));
        sleep(0.3);
    }
    sleep(1);
    return 0;
}