//
// Created by abclzr on 19-7-22.
//

#include "../../include/raft/Server/RaftRpcService.h"

namespace raft {

    grpc::Status RaftRpcService::AppendEntries(grpc::ServerContext *context, const rpc::AppendEntriesMessage *request,
                                               rpc::Reply *reply) {
        return grpc::Status::OK;
    }

    grpc::Status RaftRpcService::RequestVote(grpc::ServerContext *context, const rpc::RequestVoteMessage *request,
                                             rpc::Reply *reply) {
        return grpc::Status::OK;
    }
}