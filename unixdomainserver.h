// SPDX-FileCopyrightText: 2022 <copyright holder> <email>
// SPDX-License-Identifier: Apache-2.0

#ifndef UNIXDOMAINSERVER_H
#define UNIXDOMAINSERVER_H

#include "namespace.h"
#include <boost/asio/local/stream_protocol.hpp>

/**
 * UnixDomainServer handles new Unix Domain socket connections asynchronously
 */
class QuicklistClient;
class UnixDomainSession;

class UnixDomainServer
{
    typedef std::shared_ptr<UnixDomainSession> Session;
public:
    /*
     *  Helper function to remove stale sock file
     */
    static void removeSockFile(std::string const &file);

    UnixDomainServer ( net::io_context& ioc, std::string const & file, QuicklistClient *p );
    ~UnixDomainServer();

private:
    void onAccept ( Session sp, boost::system::error_code const & ec );

    net::io_context& ioc_;
    net::local::stream_protocol::acceptor acceptor_;
    std::string file_;
    QuicklistClient* client_;


};

#endif // UNIXDOMAINSERVER_H
