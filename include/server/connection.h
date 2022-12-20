#pragma once
#include <network/connection.h>
#include <network/stream.h>
#include <server/dictionary.h>

#include <string>

namespace server {

class server;

class connection : public ::connection {
 public:
  connection() = default;
  connection(stream_ptr &&stream, dictionary *dict, server *serv);
  connection &operator=(connection &&stream);
  connection(connection &&stream);
  connection(connection &stream) = delete;
  connection &operator=(connection &stream) = delete;

  bool is_active() const;
  void parse_json(std::byte *msg_begin, uint32_t msg_size) override;
  void state_change() override;

 private:
  dictionary *_dictionary = nullptr;
  server *_server = nullptr;
};

using connection_ptr = std::unique_ptr<connection>;

}  // namespace server
