// SPDX-FileCopyrightText: 2022 <copyright holder> <email>
// SPDX-License-Identifier: Apache-2.0

#include "unixdomainserver.h"
#include "websocketsession.h"
#include "log.h"
#include <iostream>

using std::string;
using std::shared_ptr;
using namespace std::placeholders;

UnixDomainServer::UnixDomainServer ( net::io_context& ioc, string const & file, QuicklistClient *p )
     : ioc_ ( ioc ), acceptor_ ( ioc, net::local::stream_protocol::endpoint ( file ) ), file_ ( file ), client_ ( p )
{
     Session sp ( new UnixDomainSession ( ioc_, client_ ) );
     acceptor_.async_accept ( sp->socket(),
                              std::bind ( &UnixDomainServer::onAccept, this, sp, _1 ) );
}

void UnixDomainServer::onAccept ( Session sp, boost::system::error_code const & ec )
{
     if ( !ec ) {
          sp->run();
     } else {
          fail ( ec, "UDServer/onAccept: ");
          return;
     }
     sp.reset ( new UnixDomainSession ( ioc_, client_ ) );
     acceptor_.async_accept ( sp->socket(),
                              std::bind ( &UnixDomainServer::onAccept, this, sp, _1 ) );
}

void UnixDomainServer::removeSockFile(string const &file)
{
     std::system ( ( string ( "rm " ) + file ).c_str() );
}

UnixDomainServer::~UnixDomainServer()
{
     removeSockFile(file_);
}

