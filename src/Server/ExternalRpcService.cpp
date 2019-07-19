#include "raft/Server/ExternalRpcService.h"

namespace raft {

grpc::Status ExternalRpcService::Put(grpc::ServerContext *context,
                                     const rpc::PutRequest *request,
                                     rpc::PutReply *response) {
  put(request->key(), request->value());
  return grpc::Status::OK;
}

grpc::Status ExternalRpcService::Get(grpc::ServerContext *context,
                                     const rpc::GetRequest *request,
                                     rpc::GetReply *response) {
  response->set_value(get(request->key()));
  return grpc::Status::OK;
}

} // namespace raft