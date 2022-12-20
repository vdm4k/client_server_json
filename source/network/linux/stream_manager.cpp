#include <network/linux/libev.h>
#include <network/linux/stream_manager.h>
#include <network/linux/tcp_stream.h>

namespace lin {

ev_stream_manager::ev_stream_manager() : _ev_loop{ev::init()} {}

ev_stream_manager::~ev_stream_manager() { ev::clean_up(_ev_loop); }

stream_ptr ev_stream_manager::create_send_stream(std::string const &addr,
                                                 uint16_t port) {
  auto sck = std::make_unique<tcp_stream>();
  if (!sck->connect_to_server(addr, port, _ev_loop)) return nullptr;
  return sck;
}
stream_ptr ev_stream_manager::create_listen_stream(
    std::string const &addr, uint16_t port, proccess_incoming_conn incom_con,
    std::any asoc_data) {
  auto sck = std::make_unique<tcp_stream>();
  if (!sck->bind_as_server(addr, port, _ev_loop, incom_con, asoc_data))
    return nullptr;
  return sck;
}

void ev_stream_manager::proceed() { ev::proceed(_ev_loop); }

}  // namespace lin
