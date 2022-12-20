#pragma once
#include <network/connection.h>

#include <string>
#include <vector>

namespace client {

class client;

class connection : public ::connection {
 public:
  enum class state {
    e_start,
    e_send_set_request,
    e_wait_set_reply,
    e_send_get_request,
    e_wait_get_reply,
    e_send_stat_request,
    e_wait_stat_reply,
    e_failed,
  };
  connection() = default;
  connection(stream_ptr &&stream, client *clnt);
  connection &operator=(connection &&stream);
  connection(connection &&stream);
  connection(connection &stream) = delete;
  connection &operator=(connection &stream) = delete;
  bool proceed();
  bool is_active();

 protected:
  void parse_json(std::byte *msg_begin, uint32_t msg_size) override;
  void state_change() override;

 private:
  client *_client = nullptr;
  state _state = state::e_start;
  std::string _last_key;
  uint64_t _last_success_set = 0;
  uint64_t _last_failed_set = 0;
};

using connection_ptr = std::unique_ptr<connection>;
}  // namespace client
