#ifndef HSMS_STATE_MACHINE_HPP
#define HSMS_STATE_MACHINE_HPP

#include "hsms/header.hpp"

namespace hsms {

enum class State {
    NotConnected,
    NotSelected,
    Selected
};

class StateMachine {
public:
    StateMachine() = default;

    [[nodiscard]] State get_state() const noexcept { return state_; }

    /// Event: TCP physical connection established
    void on_tcp_connect();

    /// Event: TCP connection dropped or closed
    void on_tcp_disconnect();

    /// Processes incoming control header and generates appropriate response header
    /// Returns true if a response header was generated
    bool process_incoming_header(const Header& req, Header& rsp);

private:
    State state_{State::NotConnected};
};

} // namespace hsms

#endif // HSMS_STATE_MACHINE_HPP