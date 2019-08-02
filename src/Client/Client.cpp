#include "raft/Client/Client.h"

#include <atomic>
#include <vector>
#include <chrono>

#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>

#include <grpc++/create_channel.h>
#include "raft/Server/config.h"
#include "raft/Common/external.grpc.pb.h"

namespace raft {

struct Client::Impl {
  std::vector<std::unique_ptr<external::External::Stub>> stubs;
  std::atomic<std::size_t> cur{0};
}; /* struct Client::Impl */


} // namespace ppca

namespace raft {

Client::Client(const std::string &filename) : pImpl(std::make_unique<Impl>()) {
  namespace pt = boost::property_tree;
  pt::ptree tree;
  pt::read_json(filename, tree);

    for (auto &&srv : tree.get_child("local"))
        local_address = srv.second.get_value<std::string>();
  std::vector<std::string> srvList;
  for (auto &&srv : tree.get_child("externalServerList"))
    srvList.emplace_back(srv.second.get_value<std::string>());

  for (const auto & srv : srvList) {
    pImpl->stubs.emplace_back(external::External::NewStub(grpc::CreateChannel(
        srv, grpc::InsecureChannelCredentials())));
  }
}

Client::~Client() = default;

template <class Tp>
decltype(auto) timeFrom(const Tp & tp) {
  return std::chrono::duration_cast<std::chrono::milliseconds>(
      std::chrono::system_clock::now() - tp).count();
}

void Client::Put(std::string key, std::string value, uint64_t timeout) {
  auto startTimePoint = std::chrono::system_clock::now();

  while ((uint64_t)timeFrom(startTimePoint) <= timeout) {
    auto & stub = pImpl->stubs[pImpl->cur % pImpl->stubs.size()];
    grpc::ClientContext ctx;
    ctx.set_deadline(startTimePoint + std::chrono::milliseconds(timeout));
    ctx.set_idempotent(true);

    external::PutRequest request;
    request.set_key(key);
    request.set_value(value);
    external::Reply reply;
    auto status = stub->Put(&ctx, request, &reply);

    return;
    pImpl->cur++;
  }

  throw RequestTimeout();
}

std::string Client::Get(std::string key, uint64_t timeout) {
  auto startTimePoint = std::chrono::system_clock::now();

  while ((uint64_t)timeFrom(startTimePoint) <= timeout) {
    auto & stub = pImpl->stubs[pImpl->cur % pImpl->stubs.size()];
    grpc::ClientContext ctx;
    ctx.set_deadline(startTimePoint + std::chrono::milliseconds(timeout));
    ctx.set_idempotent(true);

    external::GetRequest request;
    request.set_key(key);
    external::Reply reply;
    auto status = stub->Get(&ctx, request, &reply);

    if (status.ok())
      return "";
    pImpl->cur++;
  }

  throw RequestTimeout();
}

    void Client::Run() {
        boost::function0<void> f1 = boost::bind(&Client::RunThread, this);
        th = boost::thread(f1);
    }

    void Client::Stop() {
        ser->Shutdown();
        if (th.joinable()) th.join();
    }

    void Client::RunThread() {
        if (DEBUG) std::cout << std::setw(20) << local_address << " : Start RunThread" << std::endl;
        ExternalRpcService service;
        service.bindReplyPut([this](const external::PutReply *request, external::Reply *response) {
            replyput(request, response);
        });
        service.bindReplyGet([this](const external::GetReply *request, external::Reply *response) {
            replyget(request, response);
        });
        grpc::ServerBuilder builder;
        builder.AddListeningPort(local_address, grpc::InsecureServerCredentials());
        builder.RegisterService(&service);
        ser = builder.BuildAndStart();
        ser->Wait();
    }

    void Client::replyput(const external::PutReply *request, external::Reply *response) {

    }

    void Client::replyget(const external::GetReply *request, external::Reply *response) {
        std::cout << "Client Received : " << request->value() << std::endl;
    }

} // namespace ppca