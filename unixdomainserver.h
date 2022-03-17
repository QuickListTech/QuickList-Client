// SPDX-FileCopyrightText: 2022 <copyright holder> <email>
// SPDX-License-Identifier: Apache-2.0

#ifndef UNIXDOMAINSERVER_H
#define UNIXDOMAINSERVER_H

#include <boost/array.hpp>
#include <boost/asio.hpp>

#include "unixdomainsession.h"

/**
 * @todo write docs
 */
class QuicklistClient;

class UnixDomainServer
{
    typedef std::shared_ptr<UnixDomainSession>  Session;
public:
    UnixDomainServer ( net::io_context& ioc, std::string const & file, QuicklistClient *p );
    ~UnixDomainServer();

    void onAccept ( Session sp, boost::system::error_code const & ec );
    static void removeSockFile(std::string const &file);
private:
    net::io_context& ioc_;
    net::local::stream_protocol::acceptor acceptor_;
    std::string file_;
    QuicklistClient* client_;


};

#endif // UNIXDOMAINSERVER_H
