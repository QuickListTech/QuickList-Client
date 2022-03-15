// SPDX-FileCopyrightText: 2022 <copyright holder> <email>
// SPDX-License-Identifier: Apache-2.0

#include "quicklistclient.h"
#include "websocketsession.h"
#include "unixdomainserver.h"
#include <iostream>

QuicklistClient::QuicklistClient()
{
    qlHost_ = config.as_object()["client"].as_object()["host"].as_string().c_str();
    qlPort_ = config.as_object()["client"].as_object()["port"].as_int64();
    qlReconnect_ = config.as_object()["client"].as_object()["reconnect"].as_int64();
    qlCert_ = config.as_object()["client"].as_object()["certificate"].as_string().c_str();
    qlKey_ = config.as_object()["client"].as_object()["privatekey"].as_string().c_str();
    udsFile_ = config.as_object()["UDS"].as_object()["file"].as_string().c_str();
    enableQueue_ = config.as_object()["UDS"].as_object()["queue"].as_bool();
}

void QuicklistClient::run()
{
     clientT_.reset( new std::thread ( [&] {
          net::io_context ioc;
          net::signal_set signals ( ioc, SIGINT, SIGTERM );
          signals.async_wait ( [&ioc] ( boost::system::error_code const&, int )
          {
               ioc.stop();
          } );

          ssl::context ctx{ssl::context::tlsv12_client};

          ctx.use_certificate_file ( qlCert_, ssl::context::file_format::pem );
          ctx.use_private_key_file ( qlKey_, ssl::context::file_format::pem );

          // This holds the root certificate used for verification
          ctx.set_verify_mode ( ssl::verify_peer | ssl::verify_fail_if_no_peer_cert );
          ctx.add_verify_path ( "/etc/ssl/certs" );

          std::shared_ptr<WebsocketSession> ws = std::make_shared<WebsocketSession> ( ioc, ctx, qlHost_, qlPort_);
          ws->connect ( );

          {
               std::lock_guard<std::mutex> lock ( mtx_ );
               websocket_ = ws->weak_from_this();
          }

          ioc.run();
     } ) );

     serverT_.reset( new std::thread  ( [&] {
          net::io_context ioc;
          net::signal_set signals ( ioc, SIGINT, SIGTERM );
          signals.async_wait ( [&ioc] ( boost::system::error_code const&, int )
          {
               ioc.stop();
          } );

          UnixDomainServer serv ( ioc, udsFile_, this);

          ioc.run();
     } ) );


     clientT_->join();
     serverT_->join();

}
