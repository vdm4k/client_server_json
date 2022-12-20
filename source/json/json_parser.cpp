#include <rapidjson/error/en.h>
#include <rapidjson/writer.h>
#include <server/json_parser.h>

#include <set>

//------ requests types -------------
static char const* const s_request_type = "type";
static char const* const s_request_set_type = "set";
static char const* const s_request_get_type = "get";
static char const* const s_request_stat_type = "stat";
//------ set result -------------
static char const* const s_success_set = "success_set";
static char const* const s_failed_set = "failed_set";
static char const* const s_total_set = "total_set";

static char const* const s_success = "success";
static char const* const s_result = "result";
static char const* const s_key_type = "key";
static char const* const s_value_type = "value";
static char const* const s_cause = "cause";

static std::set<std::string> s_responses{s_result, s_value_type, s_success_set,
                                         s_failed_set, s_total_set};

static std::set<std::string> s_requests{s_request_type, s_key_type,
                                        s_value_type};

void writeDocumentToString(const rapidjson::Document& document,
                           std::string& output) {
  class StringHolder {
   public:
    using Ch = char;
    StringHolder(std::string& s) : s_(s) { s_.reserve(2048); }
    void Put(char c) { s_.push_back(c); }
    void Clear() { s_.clear(); }
    void Flush() { return; }
    size_t Size() const { return s_.length(); }

   private:
    std::string& s_;
  };

  StringHolder os(output);
  rapidjson::Writer<StringHolder> writer(os);
  document.Accept(writer);
}

std::optional<json_parser::parse_request_result> json_parser::parse_request(
    std::byte* data, size_t data_size) {
  if (_document.Parse((char*)data, data_size).HasParseError()) {
    _error = "Error(offset " + std::to_string(_document.GetErrorOffset()) +
             " ): " + rapidjson::GetParseError_En(_document.GetParseError());
    return {};
  }

  request_message_type mes_type{request_message_type::e_none};
  char const* key = nullptr;
  char const* value = nullptr;
  for (auto it = _document.MemberBegin(); it != _document.MemberEnd(); ++it) {
    if (!s_requests.count(it->name.GetString())) {
      _error = std::string("unsupported value - ") + it->name.GetString();
      return {};
    }
    if (it->name == s_request_type) {
      if (it->value == s_request_set_type) {
        mes_type = request_message_type::e_set;
      } else if (it->value == s_request_get_type) {
        mes_type = request_message_type::e_get;
      } else if (it->value == s_request_stat_type) {
        mes_type = request_message_type::e_stat;
      } else {
        _error = std::string("unsupported operation type - ") +
                 it->value.GetString();
        return {};
      }
    } else if (it->name == s_key_type) {
      key = it->value.GetString();
    } else if (it->name == s_value_type) {
      value = it->value.GetString();
    }
  }

  switch (mes_type) {
    case request_message_type::e_get: {
      if (!key) {
        _error = std::string("key must be set in get operation");
        return {};
      }
      break;
    }
    case request_message_type::e_set: {
      if (!value) {
        _error = std::string("value must be set in set operation");
        return {};
      }
      if (!key) {
        _error = std::string("key must be set in set operation");
        return {};
      }
      break;
    }
    case request_message_type::e_stat: {
      break;
    }
    default: {
      _error = std::string("operation type not set");
      return {};
    }
  }

  return json_parser::parse_request_result{mes_type, key, value};
}

std::optional<json_parser::parse_response_result> json_parser::parse_response(
    std::byte* data, size_t data_size) {
  if (_document.Parse((char*)data, data_size).HasParseError()) {
    _error = "Error(offset " + std::to_string(_document.GetErrorOffset()) +
             " ): " + rapidjson::GetParseError_En(_document.GetParseError());
    return {};
  }

  response_message_type mes_type{response_message_type::e_none};
  char const* value = nullptr;
  uint32_t success_set = 0;
  uint32_t failed_set = 0;
  uint32_t total_set = 0;
  for (auto it = _document.MemberBegin(); it != _document.MemberEnd(); ++it) {
    if (!s_responses.count(it->name.GetString())) {
      _error = std::string("unsupported value - ") + it->name.GetString();
      return {};
    }
    if (it->name == s_result) {
      if (it->value == s_success) {
        mes_type = response_message_type::e_success;
      } else {
        mes_type = response_message_type::e_failed;
      }
    } else if (it->name == s_cause) {
      value = it->value.GetString();
    } else if (it->name == s_value_type) {
      value = it->value.GetString();
    } else if (it->name == s_success_set) {
      success_set = it->value.GetInt();
    } else if (it->name == s_failed_set) {
      failed_set = it->value.GetInt();
    } else if (it->name == s_total_set) {
      total_set = it->value.GetInt();
    }
  }

  return json_parser::parse_response_result{mes_type, value, success_set,
                                            failed_set, total_set};
}

std::string json_parser::generate_stat_response(uint64_t success,
                                                uint64_t failed) {
  rapidjson::Document stat_doc;

  rapidjson::Value success_val;
  success_val.SetInt64(success);
  rapidjson::Value failed_val;
  failed_val.SetInt64(failed);
  rapidjson::Value total_val;
  total_val.SetInt64(failed + success);
  rapidjson::Value result_val;
  result_val.SetString(s_success, stat_doc.GetAllocator());

  stat_doc.SetObject();
  stat_doc.AddMember(rapidjson::StringRef(s_success_set), success_val,
                     stat_doc.GetAllocator());
  stat_doc.AddMember(rapidjson::StringRef(s_failed_set), failed_val,
                     stat_doc.GetAllocator());
  stat_doc.AddMember(rapidjson::StringRef(s_total_set), total_val,
                     stat_doc.GetAllocator());
  stat_doc.AddMember(rapidjson::StringRef(s_result), result_val,
                     stat_doc.GetAllocator());

  std::string json_body;
  writeDocumentToString(stat_doc, json_body);
  return json_body;
}

std::string json_parser::generate_stat_request() {
  rapidjson::Document req_doc;

  rapidjson::Value req_type;
  req_type.SetString(s_request_stat_type, req_doc.GetAllocator());

  req_doc.SetObject();
  req_doc.AddMember(rapidjson::StringRef(s_request_type), req_type,
                    req_doc.GetAllocator());

  std::string json_body;
  writeDocumentToString(req_doc, json_body);
  return json_body;
}

std::string json_parser::generate_set_request(std::string const& key,
                                              std::string const& value) {
  rapidjson::Document set_req_doc;

  rapidjson::Value req_type;
  req_type.SetString(s_request_set_type, set_req_doc.GetAllocator());
  rapidjson::Value key_type;
  key_type.SetString(key.c_str(), set_req_doc.GetAllocator());
  rapidjson::Value value_type;
  value_type.SetString(value.c_str(), set_req_doc.GetAllocator());

  set_req_doc.SetObject();
  set_req_doc.AddMember(rapidjson::StringRef(s_request_type), req_type,
                        set_req_doc.GetAllocator());
  set_req_doc.AddMember(rapidjson::StringRef(s_key_type), key_type,
                        set_req_doc.GetAllocator());
  set_req_doc.AddMember(rapidjson::StringRef(s_value_type), value_type,
                        set_req_doc.GetAllocator());

  std::string json_body;
  writeDocumentToString(set_req_doc, json_body);
  return json_body;
}

std::string json_parser::generate_get_request(std::string const& key) {
  rapidjson::Document get_req_doc;

  rapidjson::Value req_type;
  req_type.SetString(s_request_get_type, get_req_doc.GetAllocator());
  rapidjson::Value key_type;
  key_type.SetString(key.c_str(), get_req_doc.GetAllocator());

  get_req_doc.SetObject();
  get_req_doc.AddMember(rapidjson::StringRef(s_request_type), req_type,
                        get_req_doc.GetAllocator());
  get_req_doc.AddMember(rapidjson::StringRef(s_key_type), key_type,
                        get_req_doc.GetAllocator());

  std::string json_body;
  writeDocumentToString(get_req_doc, json_body);
  return json_body;
}

std::string json_parser::generate_error_response(std::string const& cause) {
  rapidjson::Document error_doc;

  rapidjson::Value cause_val;
  cause_val.SetString(cause.c_str(), error_doc.GetAllocator());
  rapidjson::Value result_val;
  result_val.SetString("failed", error_doc.GetAllocator());

  error_doc.SetObject();
  error_doc.AddMember(rapidjson::StringRef(s_cause), cause_val,
                      error_doc.GetAllocator());
  error_doc.AddMember(rapidjson::StringRef(s_result), result_val,
                      error_doc.GetAllocator());
  std::string json_body;
  writeDocumentToString(error_doc, json_body);
  return json_body;
}

std::string json_parser::generate_success_set_response() {
  rapidjson::Document set_resp_doc;

  rapidjson::Value result_val;
  result_val.SetString(s_success, set_resp_doc.GetAllocator());

  set_resp_doc.SetObject();
  set_resp_doc.AddMember(rapidjson::StringRef(s_result), result_val,
                         set_resp_doc.GetAllocator());
  std::string json_body;
  writeDocumentToString(set_resp_doc, json_body);
  return json_body;
}

std::string json_parser::generate_get_response(std::string const& value) {
  rapidjson::Document get_resp_doc;

  rapidjson::Value result_val;
  result_val.SetString(s_success, get_resp_doc.GetAllocator());
  rapidjson::Value value_val;
  value_val.SetString(value.c_str(), get_resp_doc.GetAllocator());

  get_resp_doc.SetObject();
  get_resp_doc.AddMember(rapidjson::StringRef(s_result), result_val,
                         get_resp_doc.GetAllocator());
  get_resp_doc.AddMember(rapidjson::StringRef(s_value_type), value_val,
                         get_resp_doc.GetAllocator());

  std::string json_body;
  writeDocumentToString(get_resp_doc, json_body);
  return json_body;
}
