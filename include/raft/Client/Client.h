#ifndef PPCA_RAFT_CLIENT_H
#define PPCA_RAFT_CLIENT_H

#include <cstdint>
#include <memory>
#include <string>
#include <exception>
#include "../Server/ExternalRpcService.h"
#include "raft/Common/external.pb.h"
#include "raft/Common/external.grpc.pb.h"
#include <boost/thread.hpp>

namespace raft {

class RequestTimeout : public std::exception {};

class Client {
public:
  explicit Client(const std::string & filename);
  Client(const Client &) = delete;
  Client(Client &&) = delete;
  Client& operator=(const Client &) = delete;
  Client& operator=(Client &&) = delete;
  ~Client();

  std::string local_address;
  std::unique_ptr<grpc::Server> ser;
  boost::thread th;
  void RunThread();
  void Run();
  void Stop();

  void Put(std::string key, std::string value, std::uint64_t timeout = 10000);

  std::string Get(std::string key, std::uint64_t timeout = 10000);

  void replyput(const external::PutReply *request, external::Reply *response);

  void replyget(const external::GetReply *request, external::Reply *response);
private:
  struct Impl;
  boost::mutex mu;
  boost::condition_variable cv;
  std::unique_ptr<Impl> pImpl;
  uint64_t timeStamp;

}; // class Client

} // namespace ppca

#endif //PPCA_RAFT_CLIENT_H
