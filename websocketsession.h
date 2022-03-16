// SPDX-FileCopyrightText: 2022 <copyright holder> <email>
// SPDX-License-Identifier: Apache-2.0

#ifndef WEBSOCKETSESSION_H
#define WEBSOCKETSESSION_H

#include <boost/beast/core.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/websocket/ssl.hpp>
#include "namespace.h"
#include <queue>

class UnixDomainSession;
class QuicklistClient;

/**
 * @todo write docs
 */
class WebsocketSession : public std::enable_shared_from_this<WebsocketSession>
{

public:
    explicit WebsocketSession ( net::io_context& ioc, ssl::context& ctx, QuicklistClient *cp, std::weak_ptr<UnixDomainSession> uds);
    ~WebsocketSession();

    void connect ();
    void send(std::shared_ptr<const std::string> msg);
    bool isOpen() const { return socket_->is_open(); }
private:
    void onResolve ( beast::error_code ec, tcp::resolver::results_type results );
    void onConnect ( beast::error_code ec, tcp::resolver::results_type::endpoint_type );
    void onSSLHandshake ( beast::error_code ec );
    void onHandshake ( beast::error_code ec );
    void onWrite ( beast::error_code ec, std::size_t bytes_transferred);
    void onRead ( beast::error_code ec, std::size_t bytes_transferred);
    void onSend(std::shared_ptr<const std::string> msg);
    void reconnectTimer();

    typedef websocket::stream<beast::ssl_stream<beast::tcp_stream>> StreamType;

    net::io_context& ioc_;
    ssl::context& ctx_;
    tcp::resolver resolver_;
    std::shared_ptr<StreamType> socket_;
    beast::flat_buffer buffer_;
    std::queue<std::string> queue_;
    net::deadline_timer rTimer_;
    QuicklistClient* client_;
    std::weak_ptr<UnixDomainSession> uds_;
};

#endif // WEBSOCKETSESSION_H
