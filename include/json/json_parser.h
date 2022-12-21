#pragma once
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#include "rapidjson/document.h"
#pragma GCC diagnostic pop

#include <cstddef>
#include <optional>

class json_parser {
 public:
  enum class request_message_type { e_none, e_get, e_set, e_stat };
  enum class response_message_type { e_none, e_success, e_failed };

  struct parse_request_result {
    request_message_type _type = request_message_type::e_none;
    char const* _key = nullptr;
    char const* _value = nullptr;
  };
  struct parse_response_result {
    response_message_type _type = response_message_type::e_none;
    char const* _result = nullptr;
    uint64_t _success_set = 0;
    uint64_t _failed_set = 0;
    uint64_t _total_set = 0;
  };

  std::optional<parse_request_result> parse_request(std::byte* data,
                                                    size_t data_size);
  std::optional<parse_response_result> parse_response(std::byte* data,
                                                      size_t data_size);
  std::string generate_stat_request() const;
  std::string generate_set_request(std::string const& key,
                                   std::string const& value) const;
  std::string generate_get_request(std::string const& key) const;
  std::string generate_error_response(const std::string& cause) const;
  std::string generate_stat_response(uint64_t success, uint64_t failed) const;
  std::string generate_set_success_response() const;
  std::string generate_get_response(std::string const& value) const;
  std::string const& get_error() const { return _error; }

 private:
  rapidjson::Document _document;
  std::string _error;
};
