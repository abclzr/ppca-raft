//
// Created by abclzr on 19-7-22.
//

#ifndef RAFT_RAFTRPCSERVICE_H
#define RAFT_RAFTRPCSERVICE_H

#include <functional>
#include <thread>

#include <grpc++/server.h>
#include <grpc++/server_builder.h>

#include "raft/Common/raft.grpc.pb.h"

namespace raft {

    class RaftRpcService : public rpc::RaftRpc::Service {
    public:
        template <class Func> void bindAppend(Func &&f) { append = std::forward<Func>(f); }
        template <class Func> void bindVote(Func &&f) { vote = std::forward<Func>(f); }
        grpc::Status AppendEntries(grpc::ServerContext *context, const rpc::AppendEntriesMessage *request, rpc::Reply *reply) override ;
        grpc::Status RequestVote(grpc::ServerContext *context, const rpc::RequestVoteMessage *request, rpc::Reply *reply) override ;

    private:
        std::function<void(const rpc::AppendEntriesMessage *, rpc::Reply *)> append;
        std::function<void(const rpc::RequestVoteMessage *, rpc::Reply *)> vote;
    };

}

#endif //RAFT_RAFTRPCSERVICE_H