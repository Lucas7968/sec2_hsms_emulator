#ifndef HSMS_TCP_SERVER_HPP
#define HSMS_TCP_SERVER_HPP

#include <cstdint>
#include <atomic>
#include <thread>
#include <memory>
#include "hsms/state_machine.hpp"
#include "hsms/secs2.hpp"
#include "hsms/thread_safe_queue.hpp"

namespace hsms {

class TcpServer {
public:
    explicit TcpServer(uint16_t port);
    ~TcpServer();

    /// Starts socket listener and worker threads
    bool start();

    /// Stops server and terminates active client threads cleanly
    void stop();

    [[nodiscard]] bool is_running() const noexcept { return is_running_; }
    [[nodiscard]] State get_session_state() const { return state_machine_.get_state(); }

    /// Enqueues an outgoing SECS-II message to send to connected host
    void send_secs_message(const SecsMessage& msg);

private:
    void accept_loop();
    void client_handler_loop(uint64_t socket_fd);

    uint16_t port_;
    std::atomic<bool> is_running_{false};
    StateMachine state_machine_{};
    ThreadSafeQueue<SecsMessage> tx_queue_{};
    std::thread listener_thread_{};
    uint64_t server_socket_{0};
};

} // namespace hsms

#endif // HSMS_TCP_SERVER_HPP