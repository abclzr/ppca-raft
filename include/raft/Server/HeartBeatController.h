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
        void bindDeclareleader(Func &&f) { declareleader = std::forward<Func>(f); }
        template<class Func>
        void bindSendHeartBeat(Func &&f) { sendHeartBeat = std::forward<Func>(f); }

        void loop();

        void interrupt();

        void Run();

        void Stop();

        void electionResult(bool flag);

        std::function<bool(uint64_t)> election;
        std::function<void()> declareleader;
        std::function<void()> sendHeartBeat;

        boost::thread th;
        bool work_is_done;
        State state;

        HeartBeatController();
    };

}

#endif //RAFT_HEARTBEATCONTROLLER_H
