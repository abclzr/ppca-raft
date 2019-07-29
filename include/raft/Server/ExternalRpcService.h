#ifndef PPCA_RAFT_EXTERNAL_RPC_SERVICE_H
#define PPCA_RAFT_EXTERNAL_RPC_SERVICE_H

#include <functional>
#include <thread>

#include <grpc++/server.h>
#include <grpc++/server_builder.h>

#include "raft/Common/external.grpc.pb.h"

namespace raft {

class ExternalRpcService : public rpc::External::Service {
public:
  template <class Func> void bindPut(Func &&f) { put = std::forward<Func>(f); }

  template <class Func> void bindGet(Func &&f) { get = std::forward<Func>(f); }

  grpc::Status Put(grpc::ServerContext *context, const rpc::PutRequest *request, rpc::Reply *response) override;
  grpc::Status Get(grpc::ServerContext *context, const rpc::GetRequest *request, rpc::Reply *response) override;
  grpc::Status ReplyPut(grpc::ServerContext *context, const rpc::PutReply *request, rpc::Reply *response) override;
  grpc::Status ReplyGet(grpc::ServerContext *context, const rpc::GetReply *request, rpc::Reply *response) override;

private:
  std::function<void(std::string, std::string)> put;
  std::function<void(std::string)> get;
};

} // namespace raft

/* Example:
 *
 * namespace raft {
 *
 * class ExternalRpcServer {
 * public:
 *   template <class Func>
 *   void bindPut(Func && f) { service.bindPut(std::forward<Func>(f)); }
 *
 * template <class Func>
 * void bindGet(Func && f) { service.bindGet(std::forward<Func>(f)); }
 *
 * void Start(std::uint16_t port) {
 *   std::string addr = "0.0.0.0:" + std::to_string(port);
 *   grpc::ServerBuilder builder;
 *   builder.AddListeningPort(addr, grpc::InsecureServerCredentials());
 *   builder.RegisterService(&service);
 *   Server = builder.BuildAndStart();
 *   runningThread = std::thread([this] { Server->Wait(); });
 * }
 *
 * void Shutdown() {
 *   if (Server)
 *     Server->Shutdown();
 *   runningThread.join();
 * }
 *
 * private:
 * ExternalRpcService service;
 * std::unique_ptr<grpc::Server> Server;
 * std::thread runningThread;
 * };
 *
 * } // namespace rafr
 */

#endif // PPCA_RAFT_EXTERNAL_RPC_SERVICE_H
