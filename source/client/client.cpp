#include <client/client.h>

namespace client {

bool client::start(stream_manager_ptr &&stream_manager, std::string server_addr,
                   uint16_t server_port, size_t max_messages) {
  _stream_manager = std::move(stream_manager);
  _max_transactions = max_messages;
  auto stream = _stream_manager->create_send_stream(server_addr, server_port);
  if (!stream) return false;
  _connection = std::make_unique<connection>(std::move(stream), this);
  _active.store(true, std::memory_order_release);
  _thread = std::thread(&client::serve, this);
  return true;
}

void client::stop() {
  _active.store(false, std::memory_order_release);
  _thread.join();
}

bool client::is_active() const noexcept {
  return _active.load(std::memory_order_acquire);
}

void client::inc_transactions() { ++_transactions; }

void client::serve() {
  while (is_active() && _transactions < _max_transactions) {
    _stream_manager->proceed();
    if (!_connection->proceed())
      std::this_thread::sleep_for(std::chrono::microseconds(100));
    if (!_connection->is_active()) break;
  }
  _active.store(false, std::memory_order_release);
}

}  // namespace client
