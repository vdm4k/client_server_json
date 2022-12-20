#pragma once

#include <optional>
#include <string>
#include <unordered_map>

#include "statistics.h"

namespace server {
class dictionary {
 public:
  std::optional<std::string> get(std::string const& key);
  bool set(std::string const& key, std::string const& value);
  stats get_stat() const noexcept;

 private:
  stats _stat;
  std::unordered_map<std::string, std::string> _dict;
};
}  // namespace server
