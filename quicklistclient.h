// SPDX-FileCopyrightText: 2022 <copyright holder> <email>
// SPDX-License-Identifier: Apache-2.0

#ifndef QUICKLISTCLIENT_H
#define QUICKLISTCLIENT_H

#include <thread>
#include <mutex>
#include <memory>

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
private:
    std::mutex mtx_;
    std::unique_ptr<std::thread> serverT_;
    std::unique_ptr<std::thread> clientT_;

    std::string qlHost_;
    int qlPort_;
    int qlReconnect_;
    std::string udsFile_;
    std::string qlCert_;
    std::string qlKey_;
    bool enableQueue_;

    std::weak_ptr<WebsocketSession> websocket_;
};

#endif // QUICKLISTCLIENT_H
