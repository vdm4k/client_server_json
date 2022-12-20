#include <client/client.h>
#include <client/connection.h>
#include <server/json_parser.h>

#include <cstring>
#include <iostream>

extern bool print_log;

namespace client {

connection::connection(stream_ptr &&stream, client *clnt)
    : ::connection(std::move(stream)), _client(clnt) {}

connection &connection::operator=(connection &&stream) {
  ::connection::operator=(std::move(stream));
  _client = stream._client;
  return *this;
}

connection::connection(connection &&stream)
    : ::connection(std::move(stream)), _client(stream._client) {}

void connection::parse_json(std::byte *msg_begin, uint32_t msg_size) {
  if (print_log)
    std::cout << "client receive response - "
              << std::string((char *)msg_begin, msg_size) << std::endl;
  json_parser response_parser;
  auto parse_res = response_parser.parse_response(msg_begin, msg_size);
  if (!parse_res) {
    if (print_log) std::cout << response_parser.get_error() << std::endl;
    return;
  }
  _client->inc_transactions();

  switch (_state) {
    case state::e_wait_set_reply: {
      if (parse_res->_type == json_parser::response_message_type::e_success) {
        _state = state::e_send_get_request;
      } else {
        _state = state::e_failed;
      }
      break;
    }
    case state::e_wait_get_reply: {
      if (parse_res->_type == json_parser::response_message_type::e_success) {
        _state = state::e_send_stat_request;
      } else {
        if (print_log)
          std::cerr << "server return error on get request for key "
                    << _last_key << std::endl;
        _state = state::e_failed;
      }
      break;
    }

    case state::e_wait_stat_reply: {
      if (parse_res->_type == json_parser::response_message_type::e_success) {
        if ((parse_res->_total_set !=
             parse_res->_failed_set + parse_res->_success_set) ||
            (_last_failed_set > parse_res->_failed_set) ||
            (_last_success_set > parse_res->_success_set) ||
            (_last_failed_set + _last_success_set > parse_res->_total_set)) {
          if (print_log)
            std::cerr << "server return non consistent data" << std::endl;
        }
        _last_success_set = parse_res->_success_set;
        _last_failed_set = parse_res->_failed_set;
        _state = state::e_start;
      } else {
        if (print_log)
          std::cerr << "server return error on stat request" << std::endl;
        _state = state::e_failed;
      }
      break;
    }
    default:
      break;
  }
}

bool connection::proceed() {
  switch (_state) {
    case state::e_start: {
      if (get_state() == stream::state::e_listen) return false;
      _state = state::e_send_set_request;
      return true;
    }
    case state::e_send_set_request: {
      char strUuid[128]{0};
      sprintf(strUuid, "%x%x-%x-%x-%x-%x%x%x", rand(),
              rand(),  // Generates a 64-bit Hex number
              rand(),  // Generates a 32-bit Hex number
              ((rand() & 0x0fff) |
               0x4000),  // Generates a 32-bit Hex number of the form 4xxx (4
                         // indicates the UUID version)
              rand() % 0x3fff + 0x8000,  // Generates a 32-bit Hex number in the
              // range [0x8000, 0xbfff]
              rand(), rand(), rand());  // Generates a 96-bit Hex number

      _last_key = strUuid;
      json_parser parser;
      send_message(parser.generate_set_request(_last_key, _last_key));
      _state = state::e_wait_set_reply;
      return true;
    }
    case state::e_wait_set_reply: {
      return false;
    }

    case state::e_send_get_request: {
      json_parser parser;
      send_message(parser.generate_get_request(_last_key));
      _state = state::e_wait_get_reply;
      return true;
    }
    case state::e_wait_get_reply: {
      return false;
    }

    case state::e_send_stat_request: {
      json_parser parser;
      send_message(parser.generate_stat_request());
      _state = state::e_wait_stat_reply;
      return true;
    }
    case state::e_wait_stat_reply: {
      return false;
    }
    default:
      break;
  }
  return false;
}

bool connection::is_active() {
  return get_state() == stream::state::e_established ||
         get_state() == stream::state::e_listen;
}

void connection::state_change() {}

}  // namespace client
