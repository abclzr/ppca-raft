//
// Created by abclzr on 19-7-22.
//

#ifndef RAFT_SERVER_H
#define RAFT_SERVER_H

#include <map>
#include <string>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <grpc++/create_channel.h>
#include "ExternalRpcService.h"
#include "RaftRpcService.h"
#include "raft/Common/raft.pb.h"
#include "raft/Common/raft.grpc.pb.h"
#include <boost/thread.hpp>

namespace raft {

    class Server {
    private:
        std::map<std::string, std::string> table;
        struct Impl;
        std::unique_ptr<Impl> pImpl;
        std::string local_address;

        void send(const std::unique_ptr<rpc::RaftRpc::Stub> &, const std::string &, const std::string &);

        void put(std::string, std::string);
        std::string get(std::string);

        void append(const rpc::AppendEntriesMessage *, rpc::Reply *);
        void vote(const rpc::RequestVoteMessage *, rpc::Reply *);

    public:
        Server(const std::string &filename);
        void RunExternal();
        void RunRaft();
        void Run();
        void Stop();
        ~Server();
    };

}
#endif //RAFT_SERVER_H
