#include "hsms/tcp_server.hpp"
#include "hsms/byte_order.hpp"

#include <algorithm> // Required for std::copy_n on GCC/Linux
#include <cstring>   // Required for std::memcpy on GCC/Linux
#include <iostream>
#include <vector>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "Ws2_32.lib")
    using socket_t = SOCKET;
    using socklen_t_type = int;
    constexpr socket_t INVALID_SOCK = INVALID_SOCKET;
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <unistd.h>
    using socket_t = int;
    using socklen_t_type = socklen_t;
    constexpr socket_t INVALID_SOCK = -1;
#endif

namespace hsms {

TcpServer::TcpServer(uint16_t port) : port_(port) {
#ifdef _WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif
}

TcpServer::~TcpServer() {
    stop();
#ifdef _WIN32
    WSACleanup();
#endif
}

bool TcpServer::start() {
    if (is_running_) return true;

    server_socket_ = static_cast<uint64_t>(socket(AF_INET, SOCK_STREAM, IPPROTO_TCP));
    if (static_cast<socket_t>(server_socket_) == INVALID_SOCK) {
        return false;
    }

    int opt = 1;
    setsockopt(static_cast<socket_t>(server_socket_), SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&opt), sizeof(opt));

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port_);

    if (bind(static_cast<socket_t>(server_socket_), reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr)) < 0) {
        return false;
    }

    if (listen(static_cast<socket_t>(server_socket_), 5) < 0) {
        return false;
    }

    is_running_ = true;
    listener_thread_ = std::thread(&TcpServer::accept_loop, this);
    return true;
}

void TcpServer::stop() {
    if (!is_running_) return;

    is_running_ = false;
    tx_queue_.stop();

    if (static_cast<socket_t>(server_socket_) != INVALID_SOCK) {
#ifdef _WIN32
        closesocket(static_cast<socket_t>(server_socket_));
#else
        close(static_cast<socket_t>(server_socket_));
#endif
        server_socket_ = static_cast<uint64_t>(INVALID_SOCK);
    }

    if (listener_thread_.joinable()) {
        listener_thread_.join();
    }
}

void TcpServer::send_secs_message(const SecsMessage& msg) {
    tx_queue_.push(msg);
}

void TcpServer::accept_loop() {
    while (is_running_) {
        sockaddr_in client_addr{};
        socklen_t_type addr_len = sizeof(client_addr);
        socket_t client_sock = accept(static_cast<socket_t>(server_socket_), reinterpret_cast<sockaddr*>(&client_addr), &addr_len);

        if (client_sock == INVALID_SOCK) {
            if (!is_running_) break;
            continue;
        }

        // Spawn a detached worker thread for each client connection
        std::thread(&TcpServer::client_handler_loop, this, static_cast<uint64_t>(client_sock)).detach();
    }
}

void TcpServer::client_handler_loop(uint64_t socket_fd) {
    auto client_sock = static_cast<socket_t>(socket_fd);
    state_machine_.on_tcp_connect();

    while (is_running_) {
        uint32_t net_len = 0;
        int bytes_read = recv(client_sock, reinterpret_cast<char*>(&net_len), 4, 0);

        if (bytes_read <= 0) break; // Client disconnected

        uint32_t msg_len = network_to_host32(net_len);
        if (msg_len < HSMS_HEADER_SIZE) break;

        std::vector<uint8_t> buffer(msg_len);
        int payload_read = recv(client_sock, reinterpret_cast<char*>(buffer.data()), static_cast<int>(msg_len), 0);

        if (payload_read < static_cast<int>(HSMS_HEADER_SIZE)) break;

        std::array<uint8_t, HSMS_HEADER_SIZE> hdr_bytes{};
        std::copy_n(buffer.begin(), HSMS_HEADER_SIZE, hdr_bytes.begin());

        Header req_hdr = Header::deserialize(hdr_bytes);
        Header rsp_hdr{};

        if (state_machine_.process_incoming_header(req_hdr, rsp_hdr)) {
            auto rsp_bytes = rsp_hdr.serialize();
            uint32_t rsp_len = host_to_network32(static_cast<uint32_t>(HSMS_HEADER_SIZE));

            std::vector<uint8_t> tx_buf(4 + HSMS_HEADER_SIZE);
            std::memcpy(tx_buf.data(), &rsp_len, 4);
            std::memcpy(tx_buf.data() + 4, rsp_bytes.data(), HSMS_HEADER_SIZE);

            send(client_sock, reinterpret_cast<const char*>(tx_buf.data()), static_cast<int>(tx_buf.size()), 0);
        }
    }

    state_machine_.on_tcp_disconnect();
#ifdef _WIN32
    closesocket(client_sock);
#else
    close(client_sock);
#endif
}

} // namespace hsms