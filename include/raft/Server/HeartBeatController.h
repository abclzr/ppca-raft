//
// Created by abclzr on 19-7-25.
//

#ifndef RAFT_HEARTBEATCONTROLLER_H
#define RAFT_HEARTBEATCONTROLLER_H

#include "config.h"
#include <functional>
#include <boost/thread.hpp>
#include <boost/chrono.hpp>
#include <algorithm>

namespace raft {

    class HeartBeatController {
    public:
        template<class Func>
        void bindElection(Func &&f) { election = std::forward<Func>(f); }
        template<class Func>
        void bindElectionDone(Func &&f) { electionDone = std::forward<Func>(f); }
        template<class Func>
        void bindHeartBeat(Func &&f) { heartBeat = std::forward<Func>(f); }

        void loop();

        void interrupt();

        void Run();

        void Stop();

        void becomeLeader();
        void becomeCandidate();
        void becomeFollower();

        std::function<void()> election;
        std::function<void()> electionDone;
        std::function<void()> heartBeat;

        boost::thread th;
        bool work_is_done;
        boost::atomic<State> state;

        HeartBeatController();
    };

}

#endif //RAFT_HEARTBEATCONTROLLER_H
