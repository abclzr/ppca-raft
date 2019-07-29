#include "raft/Server/ExternalRpcService.h"

namespace raft {

    grpc::Status ExternalRpcService::Put(grpc::ServerContext *context,
                                         const external::PutRequest *request,
                                         external::Reply *response) {
        put(request, response);
        return grpc::Status::OK;
    }

    grpc::Status ExternalRpcService::Get(grpc::ServerContext *context,
                                         const external::GetRequest *request,
                                         external::Reply *response) {
        get(request, response);
        return grpc::Status::OK;
    }

    grpc::Status ExternalRpcService::ReplyPut(grpc::ServerContext *context,
                                         const external::PutReply *request,
                                         external::Reply *response) {
        return grpc::Status::OK;
    }

    grpc::Status ExternalRpcService::ReplyGet(grpc::ServerContext *context,
                                         const external::GetReply *request,
                                         external::Reply *response) {
        return grpc::Status::OK;
    }
} // namespace raft