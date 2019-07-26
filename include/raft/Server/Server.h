//
// Created by abclzr on 19-7-22.
//

#ifndef RAFT_SERVER_H
#define RAFT_SERVER_H

#include <map>
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

namespace raft {

    class Server {
    private:
        std::map<std::string, std::string> table;
        struct Impl;
        std::unique_ptr<Impl> pImpl;
        std::string local_address;

        //send AppendEntriesRPC/RequestVoteRPC to a certain server
        std::unique_ptr<rpc::Reply> sendAppendEntriesMessage(const std::unique_ptr<rpc::RaftRpc::Stub> &, uint64_t);
        std::unique_ptr<rpc::Reply> requestVote(const std::unique_ptr<rpc::RaftRpc::Stub> &);

        //communicate with client as a leader
        void put(std::string, std::string);
        std::string get(std::string);

        //communicate with leader(not itself) as a follower
        void append(const rpc::AppendEntriesMessage *, rpc::Reply *);
        void vote(const rpc::RequestVoteMessage *, rpc::Reply *);

        uint64_t get_lastlogindex();
        uint64_t get_lastlogterm();

    public:
        Server(const std::string &filename, uint64_t);
        void RunExternal();
        void RunRaft();

        //start the server
        void Run();

        void Stop();
        ~Server();

        boost::thread t1;
        boost::thread t2;
        std::unique_ptr<grpc::Server> serverExternal;
        std::unique_ptr<grpc::Server> serverRaft;
        boost::condition_variable cv;
        boost::mutex mu;

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

        //heartbeatcontroller, when run it, it will start a new thread to control its state and time out
        //it needs to bind three local function : election, declare leader and send heartbeats
        HeartBeatController heart;
        
        std::vector<LogEntry>::iterator findLog(uint64_t);
        bool election(uint64_t);
        void declareleader();
        void sendHeartBeat();
    };

}
#endif //RAFT_SERVER_H
