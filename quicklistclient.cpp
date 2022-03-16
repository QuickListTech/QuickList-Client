// SPDX-FileCopyrightText: 2022 <copyright holder> <email>
// SPDX-License-Identifier: Apache-2.0

#include "quicklistclient.h"
#include "websocketsession.h"
#include "unixdomainserver.h"
#include <iostream>
#include <sstream>

using std::ostringstream;

QuicklistClient::QuicklistClient() : ctx_ ( ssl::context::tlsv12_client )
{
     qlHost_ = config.as_object() ["client"].as_object() ["host"].as_string().c_str();
     ostringstream os;
     os <<  config.as_object() ["client"].as_object() ["port"].as_int64();
     qlPort_ = os.str();
     qlReconnect_ = config.as_object() ["client"].as_object() ["reconnect"].as_int64();
     qlCert_ = config.as_object() ["client"].as_object() ["certificate"].as_string().c_str();
     qlKey_ = config.as_object() ["client"].as_object() ["privatekey"].as_string().c_str();
     udsFile_ = config.as_object() ["UDS"].as_object() ["file"].as_string().c_str();
     enableQueue_ = config.as_object() ["UDS"].as_object() ["queue"].as_bool();

     ctx_.use_certificate_file ( qlCert_, ssl::context::file_format::pem );
     ctx_.use_private_key_file ( qlKey_, ssl::context::file_format::pem );

     // This holds the root certificate used for verification
     ctx_.set_verify_mode ( ssl::verify_peer | ssl::verify_fail_if_no_peer_cert );
     ctx_.add_verify_path ( "/etc/ssl/certs" );
}

void QuicklistClient::run()
{
     clientT_.reset ( new std::thread ( [&] {
          net::signal_set signals ( clientIOC_, SIGINT, SIGTERM );
          signals.async_wait ( [&] ( boost::system::error_code const&, int )
          {
               clientIOC_.stop();
          } );

          clientIOC_.run();
     } ) );

     serverT_.reset ( new std::thread ( [&] {
          net::signal_set signals ( serverIOC_, SIGINT, SIGTERM );
          signals.async_wait ( [&] ( boost::system::error_code const&, int )
          {
               serverIOC_.stop();
          } );

          UnixDomainServer serv ( serverIOC_, udsFile_, this );

          serverIOC_.run();
     } ) );


     clientT_->join();
     serverT_->join();

}

std::weak_ptr<WebsocketSession>
QuicklistClient::createWebsocketSession(std::weak_ptr<UnixDomainSession> uds)
{
     auto socket = std::make_shared<WebsocketSession>(clientIOC_, ctx_, this, uds);
     socket->connect();

     return socket->weak_from_this();
}
