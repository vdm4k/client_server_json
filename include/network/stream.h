#pragma once

#include <any>
#include <cstddef>
#include <functional>
#include <memory>

class stream;
using received_data_t = std::function<void(stream *, std::any)>;
using send_data_t = std::function<void(stream *, std::any)>;
using state_changed_t = std::function<void(stream *, std::any)>;

class stream {
 public:
  enum class state : uint8_t { e_closed, e_listen, e_established, e_failed };

  virtual ~stream() = default;
  virtual ssize_t send_data(std::byte *data, size_t data_size) = 0;
  virtual ssize_t get_data(std::byte *buffer, size_t buffer_size) = 0;
  virtual state get_state() const = 0;
  virtual void set_received_data_cb(received_data_t cb, std::any user_data) = 0;
  virtual void set_send_data_cb(send_data_t cb, std::any user_data) = 0;
  virtual void set_state_changed_cb(state_changed_t cb, std::any user_data) = 0;
  virtual std::string const &get_error() const = 0;
};

using stream_ptr = std::unique_ptr<stream>;
using proccess_incoming_conn =
    std::function<void(stream_ptr &&new_stream, std::any asoc_data)>;

[[maybe_unused]] static inline const char *connection_state_to_str(
    stream::state st) {
  switch (st) {
    case stream::state::e_closed:
      return "closed";
    case stream::state::e_listen:
      return "listen";
    case stream::state::e_established:
      return "established";
    case stream::state::e_failed:
      return "failed";
    default:
      return "unknown";
  }
}
