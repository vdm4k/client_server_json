#include <network/linux/stream_manager.h>
#include <server/server.h>
#include <signal.h>

#include <iostream>

#include "CLI/CLI.hpp"

static std::atomic_bool s_active{true};

void stop(int) {
  s_active.store(false, std::memory_order::memory_order_release);
}

bool print_log = false;

int main(int argc, char** argv) {
  CLI::App app{"remote_dictionary_server"};
  std::string server_addr;
  uint16_t server_port;
  signal(SIGQUIT, stop);

  app.add_option("-a,--addr", server_addr, "server address")->required();
  app.add_option("-p,--port", server_port, "server port")->required();
  app.add_option("-l,--log", print_log, "print log");
  CLI11_PARSE(app, argc, argv);
  server::server server;
  if (!server.start(std::make_unique<lin::ev_stream_manager>(), server_addr,
                    server_port)) {
    std::cerr << "couldn't start server on address - " << server_addr
              << ", and port - " << server_port << std::endl;
    return -1;
  }
  std::cout << "start server" << std::endl;

  while (s_active.load(std::memory_order::memory_order_acquire)) {
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
  std::cout << "stop server" << std::endl;
  server.stop();
  auto stat = server.get_dictionary().get_stat();
  std::cout << "succees_get_op - " << stat._succees_get_op << std::endl;
  std::cout << "failed_get_op - " << stat._failed_get_op << std::endl;
  std::cout << "total get_op - " << stat._failed_get_op + stat._succees_get_op
            << std::endl;
  std::cout << "succees_set_op - " << stat._succees_set_op << std::endl;
  std::cout << "failed_set_op - " << stat._failed_set_op << std::endl;
  std::cout << "total set_op - " << stat._failed_set_op + stat._succees_set_op
            << std::endl;
  std::cout << "total failed - " << stat._failed_set_op + stat._failed_get_op
            << std::endl;
  std::cout << "total success - " << stat._succees_set_op + stat._succees_get_op
            << std::endl;
  std::cout << "total - "
            << stat._failed_set_op + stat._succees_set_op +
                   stat._failed_get_op + stat._succees_get_op
            << std::endl;
  return 0;
}
