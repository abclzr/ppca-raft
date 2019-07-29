//
// Created by abclzr on 19-7-22.
//

#include "../../include/raft/Server/RaftRpcService.h"

namespace raft {

    grpc::Status RaftRpcService::RequestAE(grpc::ServerContext *context,
                                                      const raft::rpc::RequestAppendEntries *request,
                                                      raft::rpc::Reply *reply) {
        requestAE(request, reply);
        return grpc::Status::OK;
    }
    grpc::Status RaftRpcService::RequestV(grpc::ServerContext *context, const rpc::RequestVote *request,
                                             rpc::Reply *reply) {
        requestV(request, reply);
        return grpc::Status::OK;
    }
    grpc::Status RaftRpcService::ReplyAE(grpc::ServerContext *context,
                                           const raft::rpc::ReplyAppendEntries *request,
                                           raft::rpc::Reply *reply) {
        replyAE(request, reply);
        return grpc::Status::OK;
    }
    grpc::Status RaftRpcService::ReplyV(grpc::ServerContext *context, const rpc::ReplyVote *request,
                                          rpc::Reply *reply) {
        replyV(request, reply);
        return grpc::Status::OK;
    }
}