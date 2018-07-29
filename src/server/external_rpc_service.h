#ifndef PPCA_RAFT_EXTERNAL_RPC_SERVICE_H
#define PPCA_RAFT_EXTERNAL_RPC_SERVICE_H

#include <functional>
#include <thread>

#include <grpc++/server.h>
#include <grpc++/server_builder.h>

#include "common/rpc/external.grpc.pb.h"

namespace ppca {

class ExternalRpcService : public rpc::External::Service {
public:
  template <class Func>
  void bindPut(Func && f) { put = std::forward<Func>(f); }

  template <class Func>
  void bindGet(Func && f) { get = std::forward<Func>(f); }

  grpc::Status Put(grpc::ServerContext *context,
                   const rpc::PutRequest *request,
                   rpc::PutReply *response) override {
    put(request->key(), request->value());
    return grpc::Status::OK;
  }

  grpc::Status Get(grpc::ServerContext * context,
                   const rpc::GetRequest *request,
                   rpc::GetReply * response) override  {
    response->set_value(get(request->key()));
    return grpc::Status::OK;
  }

private:
  std::function<void(std::string, std::string)> put;
  std::function<std::string(std::string)> get;
};

} // namespace ppca

namespace ppca {

class ExternalRpcServer {
public:
  template <class Func>
  void bindPut(Func && f) { service.bindPut(std::forward<Func>(f)); }

  template <class Func>
  void bindGet(Func && f) { service.bindGet(std::forward<Func>(f)); }

  void Start(std::uint16_t port) {
    std::string addr = "0.0.0.0:" + std::to_string(port);
    grpc::ServerBuilder builder;
    builder.AddListeningPort(addr, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);
    server = builder.BuildAndStart();
    runningThread = std::thread([this] { server->Wait(); });
  }

  void Shutdown() {
    if (server)
      server->Shutdown();
    runningThread.join();
  }

private:
  ExternalRpcService service;
  std::unique_ptr<grpc::Server> server;
  std::thread runningThread;
};

}

#endif //PPCA_RAFT_EXTERNAL_RPC_SERVICE_H
