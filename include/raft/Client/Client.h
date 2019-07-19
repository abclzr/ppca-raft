#ifndef PPCA_RAFT_CLIENT_H
#define PPCA_RAFT_CLIENT_H

#include <cstdint>
#include <memory>
#include <string>
#include <exception>

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

  void Put(std::string key, std::string value, std::uint64_t timeout = 5000);

  std::string Get(std::string key, std::uint64_t timeout = 5000);

private:
  struct Impl;
  std::unique_ptr<Impl> pImpl;

}; // class Client

} // namespace ppca

#endif //PPCA_RAFT_CLIENT_H
