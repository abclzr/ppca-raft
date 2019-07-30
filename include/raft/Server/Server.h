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
#include "raft/Common/external.pb.h"
#include "raft/Common/external.grpc.pb.h"
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include "HeartBeatController.h"
#include "event.h"

namespace raft {

    class Server {
    private:
        std::map<std::string, std::string> table;
        std::map<std::string, uint32_t> getServer;
        State getState();
        uint64_t get_lastlogindex();
        uint64_t get_lastlogterm();
        uint64_t getTerm(uint64_t index);

        struct Impl;
        std::unique_ptr<Impl> pImpl;
        std::string local_address;

        //communicate with client as a leader
        void put(const external::PutRequest *request, external::Reply *response);
        void get(const external::GetRequest *request, external::Reply *response);

        //communicate between servers
        void requestAE(const rpc::RequestAppendEntries *request, rpc::Reply *reply);
        void requestV(const rpc::RequestVote *request, rpc::Reply *reply);
        void replyAE(const rpc::ReplyAppendEntries *request, rpc::Reply *reply);
        void replyV(const rpc::ReplyVote *request, rpc::Reply *reply);
        void pushElection();
        void pushElectionDone();
        void pushHearBeat();

        //event_queue
        boost::condition_variable cv;
        boost::mutex mu;
        std::queue<event> q;
        void processEvent(const event &);

        //process event
        void election();
        void electionDone();
        void heartBeat();
        void AppendEntries(const event::RequestAppendEntries *p);
        void Vote(const event::RequestVote *p);
        void replyAppendEntries(const event::ReplyAppendEntries *p);
        void replyVote(const event::ReplyVote *p);
        void Put(const event::Put *p);
        void Get(const event::Get *p);

        //pass message to HeartBeatController
        void becomeLeader();
        void becomeCandidate();
        void becomeFollower();

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

        //date members required in raft-algorithm
        bool work_is_done;
        uint64_t votesnum;
        uint64_t clustsize;
        std::string leaderAddress;

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
        uint64_t lastApplied;
        std::vector<uint64_t> nextIndex;
        std::vector<uint64_t> matchIndex;

        HeartBeatController heart;
    };

}
#endif //RAFT_SERVER_H
