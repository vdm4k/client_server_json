#pragma once
#include <network/stream.h>
#include <network/stream_manager.h>
#include <server/connection.h>
#include <server/dictionary.h>

#include <atomic>
#include <thread>

namespace server {

class server {
 public:
  server() = default;
  server &operator=(server &&server) = delete;
  server(server &&server) = delete;
  server(server &server) = delete;
  server &operator=(server &server) = delete;

  bool start(stream_manager_ptr &&manager, const std::string &addr,
             uint16_t port);
  void stop();
  void add_event(connection *conn);
  void inc_processed_messages();
  dictionary get_dictionary() const;

 private:
  void serve();
  friend void add_connection_cd(stream_ptr &&ptr, std::any user_data);
  void add_connection(stream_ptr &&ptr);
  bool handle_connections();

  std::vector<connection *> _events;
  std::atomic_bool _active{false};
  stream_manager_ptr _manager;
  std::unordered_map<connection *, connection_ptr> _connections;
  stream_ptr _listen_socket;
  std::thread _thread;
  dictionary _dictionary;
  uint64_t _processed_messages = 0;
};
}  // namespace server
