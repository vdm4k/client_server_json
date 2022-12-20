#pragma once
#include <network/stream.h>
#include <network/stream_manager.h>

#include <atomic>
#include <condition_variable>
#include <cstring>
#include <mutex>

namespace test {

class test_stream : public stream {
 public:
  ssize_t send_data(std::byte* data, size_t data_size) override {
    std::unique_lock lk(m);
    _cv.wait(lk, [this] { return !_ready; });
    _ready = true;
    if (_send_failed) {
      _state = stream::state::e_failed;
      lk.unlock();
      _cv.notify_one();
      return -1;
    }
    _send.insert(_send.end(), data, data + data_size);
    lk.unlock();
    _cv.notify_one();
    return data_size;
  }

  void wait_sended_data(std::vector<std::byte>& send) {
    std::unique_lock lk(m);
    _cv.wait(lk, [this] { return _ready; });
    _ready = false;
    send = _send;
    _send.clear();
    lk.unlock();
    _cv.notify_one();
  }

  ssize_t get_data(std::byte* buffer, size_t /*buffer_size*/) override {
    if (_receive.empty()) return 0;
    if (_parts == 1) {
      memcpy(buffer, _receive.data(), _receive.size());
      _data_received = false;
      return _receive.size();
    }

    size_t part_size = _receive.size() / _parts;
    memcpy(buffer, _receive.data(), part_size);
    _receive = std::vector<std::byte>(_receive.data() + part_size,
                                      _receive.data() + _receive.size());
    --_parts;
    return part_size;
  }

  void set_receive_data(std::string const& data, size_t part = 1,
                        size_t times = 1) {
    uint32_t total_size = 4 + data.size();
    std::byte message[total_size];
    memcpy(message + 4, data.data(), data.size());
    uint32_t data_size = __builtin_bswap32(data.size());
    memcpy(message, &data_size, 4);
    for (size_t i = 0; i < times; ++i)
      _receive.insert(_receive.end(), message, message + total_size);
    _parts = part;
    _data_received = true;
  }

  state get_state() const override { return _state; }

  void set_received_data_cb(received_data_t cb, std::any user_data) override {
    _received_data = cb;
    _received_user_data = user_data;
  }

  void set_send_data_cb(send_data_t cb, std::any user_data) override {
    _send_data = cb;
    _send_user_data = user_data;
  }

  void set_state_changed_cb(state_changed_t cb, std::any user_data) override {
    _state_changed = cb;
    _state_changed_user_data = user_data;
  }

  void process() {
    if (_data_received) {
      _received_data(this, _received_user_data);
    }
  }

  bool _ready = false;
  std::mutex m;
  std::condition_variable _cv;
  size_t _parts = 0;
  bool _send_failed = false;
  std::atomic_size_t _sent;
  std::vector<std::byte> _send;
  std::vector<std::byte> _receive;
  std::atomic_bool _data_received{false};
  received_data_t _received_data;
  std::any _received_user_data;
  send_data_t _send_data;
  std::any _send_user_data;
  state_changed_t _state_changed;
  std::any _state_changed_user_data;
  proccess_incoming_conn _incom_con;
  std::any _incom_con_data;

  std::string _detailed_error;
  state _state = state::e_established;
  struct ev_loop* _loop = nullptr;
};

class test_stream_manager : public stream_manager {
 public:
  stream_ptr create_send_stream(std::string const& /*addr*/,
                                uint16_t /*port*/) override {
    auto sck = std::make_unique<test_stream>();
    sck->_send_failed = send_failed;
    _activated_endpoints[0] = sck.get();
    return sck;
  }
  stream_ptr create_listen_stream(std::string const& /*addr*/,
                                  uint16_t /*port*/,
                                  proccess_incoming_conn incom_con,
                                  std::any asoc_data) override {
    auto sck = std::make_unique<test_stream>();
    _listen_socket = sck.get();
    _proccess_incoming_conn = incom_con;
    _asoc_data = asoc_data;
    return sck;
  }
  void proceed() override {
    if (_activated_endpoints[0]) _activated_endpoints[0]->process();
  }

  bool send_failed = false;
  std::vector<test_stream*> _activated_endpoints{nullptr};
  test_stream* _listen_socket;
  proccess_incoming_conn _proccess_incoming_conn;
  std::any _asoc_data;
};

}  // namespace test
