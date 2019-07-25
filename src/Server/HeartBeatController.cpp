//
// Created by abclzr on 19-7-25.
//

#include "raft/Server/HeartBeatController.h"

void raft::HeartBeatController::Run() {
    th = boost::thread(&HeartBeatController::loop, this);
}

raft::HeartBeatController::HeartBeatController() {
    state = State::Follower;
    work_is_done = false;
}

void raft::HeartBeatController::interrupt() {
    th.interrupt();
}

void raft::HeartBeatController::loop() {
    while (!work_is_done) {
        if (state == State::Follower) {
            try {

            } catch (...) {
                continue;
            }
        } else if (state == State::Leader) {
            try {

            } catch (...) {
                continue;
            }
        }
    }
}

void raft::HeartBeatController::Stop() {
    work_is_done = true;
    interrupt();
}
