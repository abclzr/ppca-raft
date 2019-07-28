//
// Created by abclzr on 19-7-25.
//

#ifndef RAFT_CONFIG_H
#define RAFT_CONFIG_H

#include <cstdint>

namespace raft {

    enum State {Leader, Candidate, Follower};

    const uint64_t HEARTBEAT_TIME_OUT = 300;
    const uint64_t CANDIDATE_TIME_OUT = 200;
    const uint64_t ELECTION_TIME_OUT_DOWN = 3000;
    const uint64_t ELECTION_TIME_OUT_UP = 5000;

}

#endif //RAFT_CONFIG_H
