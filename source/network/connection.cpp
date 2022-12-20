#include <network/connection.h>

#include <cstring>

enum { e_msg_tag_size = sizeof(uint32_t) };

void receive_data_cb(stream * /*strm*/, std::any param) {
  connection *conn = std::any_cast<connection *>(param);
  conn->receive_data();
}

void send_data_cb(stream * /*strm*/, std::any param) {
  connection *conn = std::any_cast<connection *>(param);
  conn->send_data();
}

void state_change_cb(stream * /*strm*/, std::any param) {
  connection *conn = std::any_cast<connection *>(param);
  conn->state_change();
}

connection::connection(stream_ptr &&stream) : _stream(std::move(stream)) {
  _stream->set_received_data_cb(receive_data_cb, this);
}

connection &connection::operator=(connection &&conn) {
  _stream = (std::move(conn._stream));
  return *this;
}

connection::connection(connection &&conn) : _stream(std::move(conn._stream)) {}

stream::state connection::get_state() const { return _stream->get_state(); }

void connection::receive_data() {
  std::byte buffer[2000];
  ssize_t rec_size = _stream->get_data(buffer, sizeof(buffer));
  size_t buffer_shift = 0;
  if (rec_size <= 0) return;

  while (true) {
    uint32_t message_size{0};
    uint32_t total_size{0};
    std::byte *msg_begin = nullptr;
    if (!parse_data(rec_size - buffer_shift, buffer + buffer_shift, msg_begin,
                    message_size, total_size)) {
      return;
    }
    parse_json(msg_begin, message_size);

    if (!_in_data.empty()) {
      if (_in_data.size() == total_size) {
        _in_data.clear();
        break;
      } else {
        _in_data = std::vector<std::byte>(_in_data.begin() + total_size,
                                          _in_data.end());
        if (!parse_data_from_internal_buffer(msg_begin, message_size,
                                             total_size))
          break;
      }
    } else {
      if (rec_size == (buffer_shift + total_size)) break;
      buffer_shift += total_size;
    }
  }
}

bool connection::parse_data_from_internal_buffer(std::byte *&msg_begin,
                                                 uint32_t &message_size,
                                                 uint32_t &total_size) {
  if (_in_data.size() < e_msg_tag_size) return false;

  memcpy(&message_size, _in_data.data(), e_msg_tag_size);
  message_size = __builtin_bswap32(message_size);
  total_size = message_size + e_msg_tag_size;
  if (total_size > _in_data.size()) return false;
  msg_begin = _in_data.data() + e_msg_tag_size;
  return true;
}

bool connection::parse_data(ssize_t rec_size, std::byte *buffer,
                            std::byte *&msg_begin, uint32_t &message_size,
                            uint32_t &total_size) {
  if (_in_data.empty()) {
    if (rec_size < e_msg_tag_size) {
      _in_data.assign(buffer, buffer + rec_size);
      return false;
    }

    memcpy(&message_size, buffer, e_msg_tag_size);
    message_size = __builtin_bswap32(message_size);
    total_size = message_size + e_msg_tag_size;
    if (total_size > rec_size) {
      _in_data.insert(_in_data.end(), buffer, buffer + rec_size);
      return false;
    }

    msg_begin = buffer + e_msg_tag_size;

  } else {
    _in_data.insert(_in_data.end(), buffer, buffer + rec_size);
    return parse_data_from_internal_buffer(msg_begin, message_size, total_size);
  }
  return true;
}

void connection::send_data(std::byte *data, uint32_t data_size) {
  uint32_t total_size = e_msg_tag_size + data_size;
  std::byte message[total_size];
  memcpy(message + e_msg_tag_size, data, data_size);
  data_size = __builtin_bswap32(data_size);
  memcpy(message, &data_size, e_msg_tag_size);

  ssize_t send_size = _stream->send_data(message, total_size);
  if (send_size <= 0 || send_size == total_size) return;
  _out_data.insert(_out_data.end(), data + send_size,
                   data + (total_size - send_size));
  _stream->set_send_data_cb(send_data_cb, this);
}

void connection::send_message(std::string const &msg) {
  send_data((std::byte *)msg.c_str(), msg.size());
}

void connection::send_data() {
  if (_out_data.empty()) {
    _stream->set_send_data_cb(nullptr, nullptr);
    return;
  }

  ssize_t send_size = _stream->send_data(_out_data.data(), _out_data.size());
  if (send_size <= 0) return;

  if ((size_t)send_size == _out_data.size()) {
    _out_data.clear();
    _stream->set_send_data_cb(nullptr, nullptr);
    return;
  }

  _out_data =
      std::vector<std::byte>(_out_data.begin() + send_size, _out_data.end());
}
