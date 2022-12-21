#pragma once
#include <client/connection.h>
#include <network/stream_manager.h>

#include <atomic>
#include <thread>

namespace client {

class client {
 public:
  client() = default;
  client &operator=(client &&cl) = delete;
  client(client &&cl) = delete;
  client(client &cl) = delete;
  client &operator=(client &cl) = delete;

  bool start(stream_manager_ptr &&stream, std::string server_addr,
             uint16_t server_port, size_t max_messages);
  void stop();
  bool is_active() const noexcept;
  void inc_transactions();
  size_t get_transactions() const noexcept;

 private:
  void serve();

  std::atomic_bool _active{false};
  stream_manager_ptr _stream_manager;
  std::unique_ptr<connection> _connection;
  std::thread _thread;
  size_t _max_transactions = 0;
  size_t _transactions = 0;
};
}  // namespace client
