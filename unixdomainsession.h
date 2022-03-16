// SPDX-FileCopyrightText: 2022 <copyright holder> <email>
// SPDX-License-Identifier: Apache-2.0

#ifndef UNIXDOMAINSESSION_H
#define UNIXDOMAINSESSION_H

#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <queue>

#include "namespace.h"

class QuicklistClient;
class WebsocketSession;

/**
 * @todo write docs
 */
class UnixDomainSession : public std::enable_shared_from_this<UnixDomainSession>
{
public:
    UnixDomainSession ( net::io_context& ioc, QuicklistClient *p);
    ~UnixDomainSession();

    net::local::stream_protocol::socket& socket()
    {
        return socket_;
    }

    void run();
    void onRead ( const boost::system::error_code& error, size_t bytes_transferred );
    void onWrite ( const boost::system::error_code& error );
    void receive(std::string const &);
private:
  net::local::stream_protocol::socket socket_;
  QuicklistClient *client_;
  std::queue<std::string> inQueue_;
  std::queue<std::string> outQueue_;

  typedef boost::array<char, 1024> SingleBuffer;
  SingleBuffer data_;
  std::vector<SingleBuffer> buffers_;
  net::deadline_timer outQueueTimer_;
  std::weak_ptr<WebsocketSession> ws_;

  void onReceive(std::string const &msg);
  void fallback(std::string const &msg);
  void outQueueTimer();

};

#endif // UNIXDOMAINSESSION_H
