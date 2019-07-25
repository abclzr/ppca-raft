//
// Created by abclzr on 19-7-25.
//

#ifndef RAFT_HEARTBEATCONTROLLER_H
#define RAFT_HEARTBEATCONTROLLER_H

#include "config.h"
#include <functional>
#include <boost/thread.hpp>

namespace raft {

    class HeartBeatController {
    public:
        template<class Func>
        void bindElection(Func &&f) { election = std::forward<Func>(f); }

        void loop();

        void interrupt();

        void Run();

        void Stop();

        std::function<void()> election;
        boost::thread th;
        bool work_is_done;
        State state;

        HeartBeatController();
    };

}

#endif //RAFT_HEARTBEATCONTROLLER_H
