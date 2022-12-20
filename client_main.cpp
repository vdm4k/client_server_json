#include <client/client.h>
#include <network/linux/stream_manager.h>
#include <signal.h>

#include <iostream>

#include "CLI/CLI.hpp"

static std::atomic_bool s_active{true};

void stop(int) {
  s_active.store(false, std::memory_order::memory_order_release);
}

bool print_log = false;

int main(int argc, char** argv) {
  srand(time(NULL));
  signal(SIGQUIT, stop);

  CLI::App app{"remote_dictionary_client"};
  std::string server_addr;
  uint16_t server_port{0};
  uint32_t threads{std::thread::hardware_concurrency()};
  uint32_t total_request_per_thread{10};

  app.add_option("-a,--addr", server_addr, "server address")->required();
  app.add_option("-p,--port", server_port, "server port")->required();
  app.add_option("-l,--log", print_log, "print log");
  app.add_option("-t,--threads", threads, "concurrent threads/connections");
  app.add_option("-r,--requests", total_request_per_thread,
                 "requests per thread");
  CLI11_PARSE(app, argc, argv);

  std::vector<std::unique_ptr<client::client>> clients;
  for (size_t i = 0; i < threads; ++i) {
    auto clnt = std::make_unique<client::client>();
    if (!clnt->start(std::make_unique<lin::ev_stream_manager>(), server_addr,
                     server_port, total_request_per_thread)) {
      std::cerr << "couldn't create send stream" << std::endl;
    } else {
      clients.push_back(std::move(clnt));
    }
  }

  if (clients.empty()) {
    std::cerr << "couldn't create any send stream" << std::endl;
    return -1;
  }

  std::cout << "start client" << std::endl;

  while (s_active.load(std::memory_order::memory_order_acquire) &&
         clients.front()->is_active()) {
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }

  std::cout << "stop client" << std::endl;
  for (auto& client : clients) {
    client->stop();
  }
  return 0;
}
