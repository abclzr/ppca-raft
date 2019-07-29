//
// Created by abclzr on 19-7-22.
//

#ifndef RAFT_SERVER_H
#define RAFT_SERVER_H

#include <map>
#include <queue>
#include <string>
#include <iostream>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <grpc++/create_channel.h>
#include "ExternalRpcService.h"
#include "RaftRpcService.h"
#include "raft/Common/raft.pb.h"
#include "raft/Common/raft.grpc.pb.h"
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include "HeartBeatController.h"
#include "event.h"

namespace raft {

    class Server {
    private:
        std::map<std::string, std::string> table;
        struct Impl;
        std::unique_ptr<Impl> pImpl;
        std::string local_address;

        //communicate with client as a leader
        void put(std::string, std::string);
        void get(std::string);

        //communicate between servers
        void requestAE(const rpc::RequestAppendEntries *request, rpc::Reply *reply);
        void requestV(const rpc::RequestVote *request, rpc::Reply *reply);
        void replyAE(const rpc::ReplyAppendEntries *request, rpc::Reply *reply);
        void replyV(const rpc::ReplyVote *request, rpc::Reply *reply);

        uint64_t get_lastlogindex();
        uint64_t get_lastlogterm();

        //event_queue
        boost::condition_variable cv;
        boost::mutex mu;
        std::queue<event> q;

    public:
        Server(const std::string &, uint64_t, const std::string &);
        void RunExternal();
        void RunRaft();
        void RunProcess();
        //start the server
        void Run();
        void Stop();
        ~Server();

        boost::thread t1;
        boost::thread t2;
        boost::thread t3;
        std::unique_ptr<grpc::Server> serverExternal;
        std::unique_ptr<grpc::Server> serverRaft;
        boost::condition_variable cv;
        boost::mutex mu;
        std::queue<event> q;

        //date members required in raft-algorithm
        bool work_is_done;
        uint64_t clustsize;
        uint64_t currentTerm;
        std::string votedFor;
        struct LogEntry {
            uint64_t  term;
            uint64_t index;
            std::string key;
            std::string args;
            LogEntry(uint64_t, uint64_t, const std::string &, const std::string &);
        };
        std::vector<LogEntry> log;
        uint64_t commitIndex;
        uint64_t lasatApplied;
        std::vector<uint64_t> nextIndex;
        std::vector<uint64_t> matchIndex;

        HeartBeatController heart;
    };

}
#endif //RAFT_SERVER_H
