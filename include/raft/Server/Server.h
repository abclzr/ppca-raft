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

namespace raft {

    class Server {
    private:
        std::map<std::string, std::string> table;
        struct Impl;
        std::unique_ptr<Impl> pImpl;
        std::string local_address;

        void sendAppendEntriesMessage(const std::unique_ptr<rpc::RaftRpc::Stub> &, const std::string &, const std::string &);

        void put(std::string, std::string);
        std::string get(std::string);

        void append(const rpc::AppendEntriesMessage *, rpc::Reply *);
        void vote(const rpc::RequestVoteMessage *, rpc::Reply *);


    public:
        Server(const std::string &filename);
        void RunExternal();
        void RunRaft();
        void Run();
        ~Server();

        std::unique_ptr<grpc::Server> serverExternal;
        std::unique_ptr<grpc::Server> serverRaft;

        boost::condition_variable cv;
        boost::mutex mu;
        bool work_is_done;

        uint64_t currentTerm;
        uint64_t votedFor;
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

        std::vector<LogEntry>::iterator findLog(uint64_t);
    };

}
#endif //RAFT_SERVER_H
