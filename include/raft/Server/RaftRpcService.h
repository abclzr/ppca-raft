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
        template <class Func> void bindrequestAE(Func &&f) { requestAE = std::forward<Func>(f); }
        template <class Func> void bindrequestV(Func &&f) { requestV = std::forward<Func>(f); }
        template <class Func> void bindreplyAE(Func &&f) { replyAE = std::forward<Func>(f); }
        template <class Func> void bindreplyV(Func &&f) { replyV = std::forward<Func>(f); }
        grpc::Status RequestAE(grpc::ServerContext *context, const rpc::RequestAppendEntries *request, rpc::Reply *reply) override ;
        grpc::Status RequestV(grpc::ServerContext *context, const rpc::RequestVote *request, rpc::Reply *reply) override ;
        grpc::Status ReplyAE(grpc::ServerContext *context, const rpc::ReplyAppendEntries *, rpc::Reply *reply) override ;
        grpc::Status ReplyV(grpc::ServerContext *context, const rpc::ReplyVote *, rpc::Reply *reply) override ;

    private:
        std::function<void(const rpc::RequestAppendEntries *, rpc::Reply *)> requestAE;
        std::function<void(const rpc::RequestVote *, rpc::Reply *)> requestV;
        std::function<void(const rpc::ReplyAppendEntries *, rpc::Reply *)> replyAE;
        std::function<void(const rpc::ReplyVote *, rpc::Reply *)> replyV;
    };

}

#endif //RAFT_RAFTRPCSERVICE_H