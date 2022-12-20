#include <network/stream_manager.h>
struct ev_loop;

namespace lin {

class ev_stream_manager : public stream_manager {
 public:
  ev_stream_manager();
  ev_stream_manager(ev_stream_manager const &) = delete;
  ev_stream_manager(ev_stream_manager &&) = delete;
  ev_stream_manager &operator=(ev_stream_manager &&) = delete;
  ev_stream_manager &operator=(ev_stream_manager const &) = delete;
  ~ev_stream_manager();
  stream_ptr create_send_stream(std::string const &addr,
                                uint16_t port) override;
  stream_ptr create_listen_stream(std::string const &addr, uint16_t port,
                                  proccess_incoming_conn incom_con,
                                  std::any asoc_data) override;
  void proceed() override;

 private:
  struct ev_loop *_ev_loop = nullptr;
};

}  // namespace lin
