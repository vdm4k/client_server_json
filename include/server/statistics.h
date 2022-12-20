#pragma once

#include <cstdint>

namespace server {
struct stats {
  uint64_t _succees_get_op = 0;
  uint64_t _failed_get_op = 0;
  uint64_t _succees_set_op = 0;
  uint64_t _failed_set_op = 0;
};
}  // namespace server
