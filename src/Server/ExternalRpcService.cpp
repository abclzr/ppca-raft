#include "raft/Server/ExternalRpcService.h"

namespace raft {

    grpc::Status ExternalRpcService::Put(grpc::ServerContext *context,
                                         const rpc::PutRequest *request,
                                         rpc::Reply *response) {
        put(request->key(), request->value());
        return grpc::Status::OK;
    }

    grpc::Status ExternalRpcService::Get(grpc::ServerContext *context,
                                         const rpc::GetRequest *request,
                                         rpc::Reply *response) {
        get(request->key());
        return grpc::Status::OK;
    }

    grpc::Status ExternalRpcService::ReplyPut(grpc::ServerContext *context,
                                         const rpc::PutReply *request,
                                         rpc::Reply *response) {
        return grpc::Status::OK;
    }

    grpc::Status ExternalRpcService::ReplyGet(grpc::ServerContext *context,
                                         const rpc::GetReply *request,
                                         rpc::Reply *response) {
        return grpc::Status::OK;
    }
} // namespace raft