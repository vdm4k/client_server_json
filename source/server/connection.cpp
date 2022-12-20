#include <server/connection.h>
#include <server/json_parser.h>
#include <server/server.h>

#include <cstring>
#include <iostream>
#include <ostream>

extern bool print_log;

namespace server {

connection::connection(stream_ptr &&stream, dictionary *dict, server *serv)
    : ::connection(std::move(stream)), _dictionary(dict), _server(serv) {}

connection &connection::operator=(connection &&stream) {
  ::connection::operator=(std::move(stream));
  _dictionary = stream._dictionary;
  _server = stream._server;
  return *this;
}

connection::connection(connection &&stream)
    : ::connection(std::move(stream)),
      _dictionary(stream._dictionary),
      _server(stream._server) {}

bool connection::is_active() const {
  return get_state() == stream::state::e_established;
}

void connection::parse_json(std::byte *msg_begin, uint32_t msg_size) {
  if (print_log)
    std::cout << "server receive request - "
              << std::string((char *)msg_begin, msg_size) << std::endl;
  json_parser req_parser;
  json_parser resp_generator;
  auto parse_res = req_parser.parse_request(msg_begin, msg_size);
  if (parse_res) {
    switch (parse_res->_type) {
      case json_parser::request_message_type::e_get: {
        auto get_res = _dictionary->get(parse_res->_key);
        if (get_res) {
          auto json = resp_generator.generate_get_response(*get_res);
          send_data((std::byte *)json.data(), json.size());

        } else {
          auto json = resp_generator.generate_error_response(
              std::string("couldn't find value in dictionary by key - ") +
              parse_res->_key);
          send_data((std::byte *)json.data(), json.size());
        }
        break;
      }
      case json_parser::request_message_type::e_set: {
        if (_dictionary->set(parse_res->_key, parse_res->_value)) {
          auto json = resp_generator.generate_success_set_response();
          send_data((std::byte *)json.data(), json.size());
        } else {
          auto json = resp_generator.generate_error_response(
              "couldn't add new value in dictionary (collission)");
          send_data((std::byte *)json.data(), json.size());
        }
        break;
      }
      case json_parser::request_message_type::e_stat: {
        auto stat = _dictionary->get_stat();
        auto json = resp_generator.generate_stat_response(stat._succees_get_op,
                                                          stat._failed_get_op);
        send_data((std::byte *)json.data(), json.size());
        break;
      }
      default:
        break;
    }
  } else {
    auto json = resp_generator.generate_error_response(req_parser.get_error());
    send_data((std::byte *)json.data(), json.size());
  }
}

void connection::state_change() { _server->add_event(this); }

}  // namespace server
