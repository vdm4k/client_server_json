#include <server/server.h>

#include <iostream>

extern bool print_log;

namespace server {

void add_connection_cd(stream_ptr &&ptr, std::any user_data) {
  auto *srv = std::any_cast<server *>(user_data);
  srv->add_connection(std::move(ptr));
}

bool server::start(stream_manager_ptr &&manager, std::string const &addr,
                   uint16_t port) {
  _listen_socket =
      manager->create_listen_stream(addr, port, add_connection_cd, this);
  if (!_listen_socket) return false;
  _manager = std::move(manager);
  _active.store(true, std::memory_order_release);
  _thread = std::thread(&server::serve, this);
  return true;
}

void server::stop() {
  _active.store(false, std::memory_order_release);
  _thread.join();
}

void server::add_event(connection *conn) { _events.push_back(conn); }

void server::inc_processed_messages() { ++_processed_messages; }

void server::serve() {
  while (_active.load(std::memory_order_acquire)) {
    _manager->proceed();
    if (!_processed_messages && !handle_connections())
      std::this_thread::sleep_for(std::chrono::microseconds(100));
    _processed_messages = 0;
  }
}

bool server::handle_connections() {
  if (_events.empty()) return false;
  for (connection *conn : _events) {
    if (_connections.find(conn) == _connections.end()) continue;
    if (!conn->is_active()) _connections.erase(conn);
  }
  _events.clear();
  return true;
}

dictionary server::get_dictionary() const { return _dictionary; }

void server::add_connection(stream_ptr &&strm) {
  if (stream::state::e_established == strm->get_state()) {
    auto conn =
        std::make_unique<connection>(std::move(strm), &_dictionary, this);
    _connections.insert({conn.get(), std::move(conn)});
  } else {
    if (print_log)
      std::cerr << "incomming connection failed - " << strm->get_error()
                << std::endl;
  }
}

}  // namespace server
