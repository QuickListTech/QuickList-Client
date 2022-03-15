// SPDX-FileCopyrightText: 2022 <copyright holder> <email>
// SPDX-License-Identifier: Apache-2.0

#include "unixdomainsession.h"
#include "websocketsession.h"
#include "quicklistclient.h"
#include <iostream>
#include <sstream>

using namespace std::placeholders;
using std::string;
using std::ostringstream;

using namespace boost::posix_time;


extern void fail ( boost::system::error_code, char const* );

UnixDomainSession::UnixDomainSession ( net::io_context& ioc, QuicklistClient *p )
     : socket_ ( ioc ), client_ ( p ), outQueueTimer_ ( ioc )
{
     outQueueTimer();
}

void UnixDomainSession::run()
{
     socket_.async_read_some ( boost::asio::buffer ( data_ ),
                               std::bind ( &UnixDomainSession::onRead, shared_from_this(),_1, _2 ) );
}

void UnixDomainSession::onRead ( const boost::system::error_code& error, size_t bytes_transferred )
{
     if ( error ) {
          return fail ( error, "UDSession/onRead" );
     }

     if ( bytes_transferred < data_.size() ) {
          ostringstream payload;

          if ( buffers_.size() > 0 ) {
               for ( auto buf : buffers_ ) {
                    payload << string ( buf.data(), buf.size() );
               }

               buffers_.clear();
          }

          payload << string ( data_.data(), data_.size() ).c_str();
          string pl ( payload.str() );
          data_ = {};

          auto sp = client_->websocket_.lock();

          if ( pl == "STATUS" ) {
               if ( sp && sp->isOpen() ) {
                    receive ( "UP" );
               } else {
                    receive ( "DOWN" );
               }
          } else {
               if ( sp  && sp->isOpen() ) {
                    sp->send ( std::make_shared<string> ( pl ), weak_from_this() );
               } else {
                    fallback ( pl );
               }
          }
     } else {
          buffers_.push_back ( data_ );
          data_ = {};
     }

     socket_.async_read_some ( boost::asio::buffer ( data_ ),
                               std::bind ( &UnixDomainSession::onRead, shared_from_this(),_1, _2 ) );
}

void UnixDomainSession::onWrite ( const boost::system::error_code& error )
{
     if ( error ) {
          return fail ( error, "UDSession/onWrite" );
     }

     inQueue_.pop();

     if ( !error ) {
          if ( inQueue_.size() > 0 ) {
               net::async_write ( socket_, net::buffer ( inQueue_.front() ), std::bind ( &UnixDomainSession::onWrite, shared_from_this(), _1 ) );
          }
     }
}

void UnixDomainSession::receive ( string const &msg )
{
     net::post ( socket_.get_executor(), std::bind ( &UnixDomainSession::onReceive, shared_from_this(), msg ) );
}

void UnixDomainSession::onReceive ( string const &msg )
{
     inQueue_.push ( msg );

     if ( inQueue_.size() > 1 ) {
          return;
     }

     net::async_write ( socket_, net::buffer ( inQueue_.front() ), std::bind ( &UnixDomainSession::onWrite, shared_from_this(), _1 ) );
}

void UnixDomainSession::fallback ( string const &msg )
{
     if ( client_->enableQueue_ ) {
          receive ( "QUEUE" );
          outQueue_.push ( msg );
     } else {
          receive ( "FAIL" );
     }
}

void UnixDomainSession::outQueueTimer()
{
     outQueueTimer_.expires_from_now ( seconds ( 5 ) );
     outQueueTimer_.async_wait ( [&] ( boost::system::error_code const &e ) {
          if ( e ) {
               return;
          }

          if ( outQueue_.size() > 0 ) {
               if ( auto sp = client_->websocket_.lock() ) {
                    if ( sp->isOpen() ) {
                         while ( !outQueue_.empty() ) {
                              sp->send ( std::make_shared<string> ( outQueue_.front() ), weak_from_this() );
                              outQueue_.pop();
                         }
                    }
               }
          }

          outQueueTimer();
     } );
}

UnixDomainSession::~UnixDomainSession()
{
     outQueueTimer_.cancel();
}

