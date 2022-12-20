#pragma once
#include <any>
#include <cstdint>
#include <string>

#include "stream.h"

class stream_manager {
 public:
  virtual ~stream_manager() = default;
  virtual stream_ptr create_send_stream(std::string const &addr,
                                        uint16_t port) = 0;
  virtual stream_ptr create_listen_stream(std::string const &addr,
                                          uint16_t port,
                                          proccess_incoming_conn incom_con,
                                          std::any asoc_data) = 0;
  virtual void proceed() = 0;
};

using stream_manager_ptr = std::unique_ptr<stream_manager>;
