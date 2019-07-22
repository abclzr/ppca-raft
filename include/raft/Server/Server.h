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

namespace raft {

    class Server {
    private:
        std::map<std::string, std::string> table;
        struct Impl;
        std::unique_ptr<Impl> pImpl;
        std::string local_address;


        void put(std::string, std::string);
        std::string get(std::string);

    public:
        Server(const std::string &filename);
        void RunExternal();
        void RunRaft();
        ~Server();
    };

}
#endif //RAFT_SERVER_H
