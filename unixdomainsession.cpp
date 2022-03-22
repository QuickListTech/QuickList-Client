// SPDX-FileCopyrightText: 2022 <copyright holder> <email>
// SPDX-License-Identifier: Apache-2.0

#include "unixdomainsession.h"
#include "websocketsession.h"
#include "quicklistclient.h"
#include "log.h"
#include <iostream>
#include <sstream>

using namespace std::placeholders;
using std::string;
using std::ostringstream;

using namespace boost::posix_time;

UnixDomainSession::UnixDomainSession ( net::io_context& ioc, QuicklistClient *p )
     : socket_ ( ioc ), client_ ( p ), outQueueTimer_ ( ioc )
{
}

void UnixDomainSession::run()
{
     startOutQueueTimer();
     ws_ = client_->createWebsocketSession ( weak_from_this() );

     net::async_read_until ( socket_, data_, '\n',
                             std::bind ( &UnixDomainSession::onRead, shared_from_this(),_1, _2 ) );
}

void UnixDomainSession::onRead ( boost::system::error_code const & ec, size_t bytes_transferred )
{
     // close socket on error
     if ( ec ) {
          closeSocket();
          return fail ( ec, "UDSession/onRead" );
     }

     /*
      * Read buffer
      */
     auto sp = ws_.lock();
     string payload = bufferToString();

     // Empty buffer
     data_.consume(bytes_transferred);

     // Respond to basic STATUS requests
     if ( payload == "STATUS\n" ) {
          if ( sp && sp->isOpen() ) {
               receive ( "UP" );
          } else {
               receive ( "DOWN" );
          }
     } else {
          if ( sp  && sp->isOpen() ) {
               sp->send ( std::make_shared<string> ( payload ) );
          } else {
               // Queue message
               fallback ( payload );
          }
     }

     net::async_read_until ( socket_, data_, '\n',
                             std::bind ( &UnixDomainSession::onRead, shared_from_this(),_1, _2 ) );
}

void UnixDomainSession::onWrite ( const boost::system::error_code& ec )
{
     if ( ec ) {
          closeSocket();
          return fail ( ec, "UDSession/onWrite" );
     }

     inQueue_.pop();

     if ( inQueue_.size() > 0 ) {
          net::async_write ( socket_,
                             net::buffer ( inQueue_.front() ),
                             std::bind ( &UnixDomainSession::onWrite, shared_from_this(), _1 ) );
     }
}

void UnixDomainSession::receive ( string const &msg )
{
     net::post ( socket_.get_executor(),
                 std::bind ( &UnixDomainSession::onReceive, shared_from_this(), msg + "\n" ) );
}

void UnixDomainSession::onReceive ( string const &msg )
{
     inQueue_.push ( msg );

     if ( inQueue_.size() > 1 ) {
          return;
     }

     net::async_write ( socket_,
                         net::buffer ( inQueue_.front() ), std::bind ( &UnixDomainSession::onWrite, shared_from_this(), _1 ) );
}

void UnixDomainSession::fallback ( string const &msg )
{
     if ( client_->isQueueEnabled() ) {
          receive ( "QUEUE" );
          outQueue_.push ( msg );
     } else {
          receive ( "FAIL" );
     }
}

void UnixDomainSession::startOutQueueTimer()
{
     outQueueTimer_.expires_from_now ( seconds ( 5 ) );
     outQueueTimer_.async_wait ( std::bind ( &UnixDomainSession::onOutQueueTimer, shared_from_this(), _1 ) );
}

void UnixDomainSession::onOutQueueTimer ( boost::system::error_code const &ec )
{
     if ( ec || !socket_.is_open() ) {
          return;
     }

     if ( outQueue_.size() > 0 ) {
          if ( auto sp = ws_.lock() ) {
               if ( sp->isOpen() ) {
                    while ( !outQueue_.empty() ) {
                         sp->send ( std::make_shared<string> ( outQueue_.front() ) );
                         outQueue_.pop();
                    }
               }
          }
     }

     startOutQueueTimer();
}
UnixDomainSession::~UnixDomainSession()
{
     if ( auto sp = ws_.lock() ) {
          sp->close();
     }
}

void UnixDomainSession::closeSocket()
{
     socket_.close();
     BOOST_LOG_TRIVIAL(info) << "Unix Domain Socket closed";
}

string UnixDomainSession::bufferToString() const
{
     using boost::asio::buffers_begin;

     string result ( buffers_begin ( data_.data() ), buffers_begin ( data_.data() ) + data_.size() );

     return result;
}
