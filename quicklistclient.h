// SPDX-FileCopyrightText: 2022 <copyright holder> <email>
// SPDX-License-Identifier: Apache-2.0

#ifndef QUICKLISTCLIENT_H
#define QUICKLISTCLIENT_H

#include <thread>
#include <memory>
#include <unordered_map>
#include <boost/beast/ssl.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/websocket/ssl.hpp>

#include "namespace.h"

class WebsocketSession;
class UnixDomainSession;


/**
 * @todo write docs
 */
class QuicklistClient
{
    friend WebsocketSession;
    friend UnixDomainSession;
public:
    QuicklistClient();

    void run();
    std::weak_ptr<WebsocketSession> createWebsocketSession(std::weak_ptr<UnixDomainSession> uds);
private:
    std::mutex mtx_;
    std::unique_ptr<std::thread> serverT_;
    std::unique_ptr<std::thread> clientT_;
    net::io_context serverIOC_;
    net::io_context clientIOC_;

    std::string qlHost_;
    std::string qlPort_;
    int qlReconnect_;
    std::string udsFile_;
    std::string qlCert_;
    std::string qlKey_;
    bool enableQueue_;
    ssl::context ctx_;
};

#endif // QUICKLISTCLIENT_H
