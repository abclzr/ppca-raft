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
    boost::this_thread::disable_interruption di;
    while (!work_is_done) {
        if (state == State::Follower) {
            try {
                boost::this_thread::restore_interruption ri(di);
                int randtime = ELECTION_TIME_OUT_DOWN + rand() % (ELECTION_TIME_OUT_UP - ELECTION_TIME_OUT_DOWN);
                boost::this_thread::sleep_for(boost::chrono::milliseconds(randtime));
            } catch (boost::thread_interrupted) {
                continue;
            }
            state = State::Candidate;
        } else if (state == State::Leader) {
            try {
                boost::this_thread::restore_interruption ri(di);
                boost::this_thread::sleep_for(boost::chrono::milliseconds(HEARTBEAT_TIME_OUT));
            } catch (boost::thread_interrupted) {
                continue;
            }
            heartBeat();
        } else if (state == State::Candidate) {
            try {
                election();
                boost::this_thread::restore_interruption ri(di);
                int randtime = ELECTION_TIME_OUT_DOWN + rand() % (ELECTION_TIME_OUT_UP - ELECTION_TIME_OUT_DOWN);
                boost::this_thread::sleep_for(boost::chrono::milliseconds(randtime));
            } catch (boost::thread_interrupted) {
                continue;
            }
        }
    }
}

void raft::HeartBeatController::Stop() {
    work_is_done = true;
    interrupt();
}

void raft::HeartBeatController::becomeLeader() {
    state = State::Leader;
    interrupt();
}

void raft::HeartBeatController::becomeFollower() {
    state = State::Follower;
    interrupt();
}
