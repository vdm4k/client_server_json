#include <server/dictionary.h>

std::optional<std::string> server::dictionary::get(std::string const& key) {
  auto it = _dict.find(key);
  if (it == _dict.end()) {
    ++_stat._failed_get_op;
    return {};
  }
  ++_stat._succees_get_op;
  return it->second;
}

bool server::dictionary::set(std::string const& key, std::string const& value) {
  if (_dict.insert({key, value}).second) {
    ++_stat._succees_set_op;
    return true;
  }
  ++_stat._failed_set_op;
  return false;
}

server::stats server::dictionary::get_stat() const noexcept { return _stat; }
