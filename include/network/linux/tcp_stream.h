#pragma once

#include <arpa/inet.h>
#include <network/stream.h>

#include <string>

#include "ev.h"

namespace lin {

class tcp_stream : public stream {
 public:
  ~tcp_stream();
  tcp_stream() = default;
  tcp_stream(tcp_stream const&) = delete;
  tcp_stream(tcp_stream&&) = delete;
  tcp_stream& operator=(tcp_stream&&) = delete;
  tcp_stream& operator=(tcp_stream const&) = delete;
  ssize_t send_data(std::byte* data, size_t data_size) override;
  ssize_t get_data(std::byte* buffer, size_t buffer_size) override;
  state get_state() const override;
  void set_received_data_cb(received_data_t cb, std::any user_data) override;
  void set_send_data_cb(send_data_t cb, std::any user_data) override;
  void set_state_changed_cb(state_changed_t cb, std::any user_data) override;

  bool connect_to_server(std::string const& peer_addr, uint16_t peer_port,
                         struct ev_loop* loop);
  bool bind_as_server(std::string const& peer_addr, uint16_t peer_port,
                      struct ev_loop* loop, proccess_incoming_conn incom_con,
                      std::any asoc_data);

 private:
  void cleanup();
  void init_events(struct ev_loop* loop);
  void stop_events();
  void receive_data();
  void send_data();
  void connection_established();
  bool connect();
  void set_detailed_error(const std::string& str);
  void set_connection_state(state st);
  bool create_socket();
  void set_socket_specific_options();
  bool fill_addr(std::string const& str_addr, uint16_t port, sockaddr_in& addr);
  void handle_incoming_connection(int file_descr, sockaddr_in peer_addr);
  bool create_listen_tcp_socket();

  friend void receive_data_cb(struct ev_loop*, ev_io* w, int);
  friend void send_data_cb(struct ev_loop*, ev_io* w, int);
  friend void connection_established_cb(struct ev_loop*, ev_io* w, int);
  friend void incoming_connection_cb(struct ev_loop* /*loop*/, ev_io* w,
                                     int /*revents*/);

  sockaddr_in _peer_addr;
  sockaddr_in _self_addr;
  ev_io _read_io;
  ev_io _write_io;
  ev_io _connect_io;
  received_data_t _received_data;
  std::any _received_user_data;
  send_data_t _send_data;
  std::any _send_user_data;
  state_changed_t _state_changed;
  std::any _state_changed_user_data;
  proccess_incoming_conn _incom_con;
  std::any _incom_con_data;

  std::string _detailed_error;
  int _file_descr = -1;
  state _state = state::e_closed;
  struct ev_loop* _loop = nullptr;
};

}  // namespace lin
