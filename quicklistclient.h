// SPDX-FileCopyrightText: 2022 <copyright holder> <email>
// SPDX-License-Identifier: Apache-2.0

#ifndef QUICKLISTCLIENT_H
#define QUICKLISTCLIENT_H

#include <thread>
#include <memory>
#include <boost/beast/ssl.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/websocket/ssl.hpp>

#include "namespace.h"

class WebsocketSession;
class UnixDomainSession;


/**
 * This class starts 2 threads. One for websocket context and one for unix domain socket context.
 *
 */
class QuicklistClient
{
public:
    QuicklistClient();

    /*
     * Start threads
     */
    void run();

    std::string const & remoteHost() const
    {
        return qlHost_;
    }
    std::string const & remotePort() const
    {
        return qlPort_;
    }
    std::string const & unixDomainSocketPath() const
    {
        return udsFile_;
    }
    std::string const &clientCertificatePath() const
    {
        return qlCert_;
    }
    std::string const &clientPrivateKeyPath() const
    {
        return qlKey_;
    }

    bool isQueueEnabled() const {
        return queueEnabled_;
    }

    unsigned int reconnectTime() const {
        return reconnectTime_;
    }

    std::weak_ptr<WebsocketSession>
    createWebsocketSession(std::weak_ptr<UnixDomainSession> uds);
private:
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
    bool queueEnabled_;
    unsigned int reconnectTime_;
    ssl::context ctx_;
};

#endif // QUICKLISTCLIENT_H
