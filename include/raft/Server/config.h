//
// Created by abclzr on 19-7-25.
//

#ifndef RAFT_CONFIG_H
#define RAFT_CONFIG_H

#include <cstdint>

namespace raft {

    enum State {Leader, Candidate, Follower};

    const bool DEBUG = false;
    const uint64_t RPC_TIME_OUT = 10;
    const uint64_t HEARTBEAT_TIME_OUT = 30;
    const uint64_t ELECTION_TIME_OUT_DOWN = 500;
    const uint64_t ELECTION_TIME_OUT_UP = 1000;

}

#endif //RAFT_CONFIG_H
