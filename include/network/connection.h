#pragma once
#include <network/stream.h>

#include <string>
#include <vector>

class connection {
 public:
  virtual ~connection() {}
  connection() = default;
  connection &operator=(connection &&conn);
  connection(connection &&conn);
  connection(connection &conn) = delete;
  connection &operator=(connection &conn) = delete;
  connection(stream_ptr &&stream);

  virtual void parse_json(std::byte *msg_begin, uint32_t msg_size) = 0;
  virtual void state_change() = 0;

  stream::state get_state() const;

 protected:
  void send_data(std::byte *data, uint32_t data_size);
  void send_message(std::string const &msg);

 private:
  void receive_data();
  bool parse_data(ssize_t rec_size, std::byte *buffer, std::byte *&msg_begin,
                  uint32_t &message_size, uint32_t &total_size);
  bool parse_data_from_internal_buffer(std::byte *&msg_begin,
                                       uint32_t &message_size,
                                       uint32_t &total_size);
  friend void receive_data_cb(stream *strm, std::any param);
  void send_data();

  friend void send_data_cb(stream *strm, std::any param);
  friend void state_change_cb(stream *strm, std::any param);
  stream_ptr _stream;
  std::vector<std::byte> _in_data;
  std::vector<std::byte> _out_data;
};
