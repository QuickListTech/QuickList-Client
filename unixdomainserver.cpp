// SPDX-FileCopyrightText: 2022 <copyright holder> <email>
// SPDX-License-Identifier: Apache-2.0

#include "unixdomainserver.h"
#include "websocketsession.h"
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

void UnixDomainServer::onAccept ( Session sp, boost::system::error_code const & error )
{
     if ( !error ) {
          sp->run();
     } else {
          std::cerr << "UDServer/onAccept: " << error.what() << std::endl;
          return;
     }
     sp.reset ( new UnixDomainSession ( ioc_, client_ ) );
     acceptor_.async_accept ( sp->socket(),
                              std::bind ( &UnixDomainServer::onAccept, this, sp, _1 ) );
}

UnixDomainServer::~UnixDomainServer()
{
     std::system ( ( string ( "rm " ) + file_ ).c_str() );
}

