#include "hsms/state_machine.hpp"

namespace hsms {

void StateMachine::on_tcp_connect() {
    if (state_ == State::NotConnected) {
        state_ = State::NotSelected;
    }
}

void StateMachine::on_tcp_disconnect() {
    state_ = State::NotConnected;
}

bool StateMachine::process_incoming_header(const Header& req, Header& rsp) {
    if (state_ == State::NotConnected) {
        return false;
    }

    switch (req.stype) {
        case SType::SelectReq: {
            rsp.session_id   = req.session_id;
            rsp.ptype        = 0;
            rsp.stype        = SType::SelectRsp;
            rsp.system_bytes = req.system_bytes;

            if (state_ == State::NotSelected) {
                state_ = State::Selected;
                rsp.header_byte3 = static_cast<uint8_t>(SelectStatus::Success);
            } else { // Already Selected
                rsp.header_byte3 = static_cast<uint8_t>(SelectStatus::AlreadyActive);
            }
            return true;
        }

        case SType::LinktestReq: {
            rsp.session_id   = req.session_id;
            rsp.header_byte2 = 0;
            rsp.header_byte3 = 0;
            rsp.ptype        = 0;
            rsp.stype        = SType::LinktestRsp;
            rsp.system_bytes = req.system_bytes;
            return true;
        }

        case SType::SeparateReq: {
            if (state_ == State::Selected) {
                state_ = State::NotSelected;
            }
            return false; // Separate.req has no response frame
        }

        default:
            break;
    }

    return false;
}

} // namespace hsms