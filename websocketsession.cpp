// SPDX-FileCopyrightText: 2022 <copyright holder> <email>
// SPDX-License-Identifier: Apache-2.0

#include <boost/json.hpp>

#include "websocketsession.h"
#include "unixdomainsession.h"
#include "quicklistclient.h"
#include "log.h"

using std::string;
using std::shared_ptr;
using std::ostringstream;

using namespace boost::posix_time;


WebsocketSession::WebsocketSession ( net::io_context& ioc, ssl::context& ctx, QuicklistClient *cp, std::weak_ptr<UnixDomainSession> uds) :
ioc_(ioc), ctx_(ctx), resolver_ ( ioc ),  rTimer_(ioc), client_(cp), uds_(uds)
{
}

void WebsocketSession::reconnectTimer()
{
     if (auto sp = uds_.lock()) {
          sp->receive("DOWN");
     }

     rTimer_.expires_from_now ( seconds ( client_->reconnectTime() ) );
     rTimer_.async_wait (std::bind ( &WebsocketSession::connect, shared_from_this() ) );
}

void WebsocketSession::connect ()
{
     if (uds_.expired()) {
          return;
     }

     socket_.reset(new StreamType{ ioc_ , ctx_ });
     buffer_.clear();

     resolver_.async_resolve ( client_->remoteHost(),
                               client_->remotePort(),
                               beast::bind_front_handler ( &WebsocketSession::onResolve, shared_from_this() ) );
}

void WebsocketSession::onResolve ( beast::error_code ec, tcp::resolver::results_type results )
{
     if ( ec ) {
          reconnectTimer();
          return fail ( ec, "WS/onResolve" );
     }

     beast::get_lowest_layer ( *socket_ ).expires_after ( std::chrono::seconds ( 30 ) );
     beast::get_lowest_layer ( *socket_ ).async_connect (results,
                                                    beast::bind_front_handler (&WebsocketSession::onConnect, shared_from_this() ) );
}

void WebsocketSession::onConnect ( beast::error_code ec, tcp::resolver::results_type::endpoint_type )
{
     if ( ec ) {
          reconnectTimer();
          return fail ( ec, "WS/onConnect" );
     }

     beast::get_lowest_layer ( *socket_ ).expires_after ( std::chrono::seconds ( 30 ) );
     socket_->next_layer().async_handshake (ssl::stream_base::client,
                                       beast::bind_front_handler ( &WebsocketSession::onSSLHandshake, shared_from_this() ) );
}

void WebsocketSession::onSSLHandshake ( beast::error_code ec )
{
     if ( ec ) {
          reconnectTimer();
          return fail ( ec, "WS/onSSLHandshake" );

     }

     beast::get_lowest_layer ( *socket_ ).expires_never();
     socket_->set_option (websocket::stream_base::timeout::suggested (beast::role_type::client ) );
     socket_->set_option ( websocket::stream_base::decorator ( [] ( websocket::request_type& req ) {
          req.set ( http::field::user_agent, string ( BOOST_BEAST_VERSION_STRING ) + " quicklist-client" );
     } ) );

     websocket::permessage_deflate opt;
     opt.client_enable = true;
     socket_->set_option ( opt );

     socket_->async_handshake ( client_->remoteHost(),
                                "/",
                                beast::bind_front_handler (&WebsocketSession::onHandshake, shared_from_this() ) );
}

void WebsocketSession::onHandshake ( beast::error_code ec )
{
     if ( ec ) {
          reconnectTimer();
          return fail ( ec, "WS/onHandshake" );

     } else {
          BOOST_LOG_TRIVIAL(warning) << "QuickList server connected: " << client_->remoteHost() << ":" << client_->remotePort();

          if (auto sp = uds_.lock()) {
               sp->receive("UP");
          }
     }

     // Read a message
     socket_->async_read ( buffer_,
                      beast::bind_front_handler ( &WebsocketSession::onRead, shared_from_this() ) );
}

void WebsocketSession::onWrite ( beast::error_code ec, std::size_t bytes_transferred )
{
     boost::ignore_unused ( bytes_transferred );

     if ( ec ) {
          reconnectTimer();
          return fail ( ec, "WS/onWrite" );
     }

     queue_.pop();

     if ( ! queue_.empty() ) {
          socket_->async_write ( net::buffer ( queue_.front() ),
                                  beast::bind_front_handler (&WebsocketSession::onWrite,shared_from_this() ) );
     }
}

void WebsocketSession::onRead (beast::error_code ec, std::size_t bytes_transferred)
{
     boost::ignore_unused ( bytes_transferred );

     if ( ec ) {
          reconnectTimer();
          return fail ( ec, "WS/onRead" );
     }

     if (auto sp = uds_.lock()) {
          sp->receive(beast::buffers_to_string ( buffer_.data() ));
     }

     buffer_.consume(bytes_transferred);

     socket_->async_read (buffer_,
                          beast::bind_front_handler ( &WebsocketSession::onRead, shared_from_this() ) );
}

void WebsocketSession::send(shared_ptr<const string> msg)
{
     if (isOpen()) {
          net::post ( socket_->get_executor(), beast::bind_front_handler ( &WebsocketSession::onSend, shared_from_this(), msg) );
     }
}

void WebsocketSession::onSend ( shared_ptr<const string> msg)
{
     queue_.push( *msg );

     if ( queue_.size() > 1 ) {
          return;
     }

     socket_->async_write ( net::buffer ( queue_.front() ),
                            beast::bind_front_handler ( &WebsocketSession::onWrite, shared_from_this()) );
}

void WebsocketSession::close()
{
     if (socket_->is_open()) {
          socket_->async_close(beast::websocket::close_reason(), beast::bind_front_handler(&WebsocketSession::onClose, shared_from_this() ) );
     }
}

void WebsocketSession::onClose(beast::error_code ec)
{
     if (ec) {
          return fail ( ec, "WS/onClose" );
     }

     BOOST_LOG_TRIVIAL(info) << "QuickList server connection closed";
}

WebsocketSession::~WebsocketSession()
{

}
