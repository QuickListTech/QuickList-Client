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
 * UnixDomainSession is created for every connection. It establishes 1-to-1 WebsocketSession connection.
 * If WebsocketSession goes down, it queues all messages and returns "QUEUE" signal to client.
 *
 * An internal timer then checks when connection is back up and sends all queued messages.
 *
 * It responds to basic STATUS messages to tell client if WebsocketSession is UP or DOWN
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

    /*
     * Start async chain
     */
    void run();

    /*
     * Incoming messages from QuickList should call this to deliver responses to client
     */
    void receive(std::string const &);
private:
  void onRead ( const boost::system::error_code& ec, size_t bytes_transferred );
  void onWrite ( const boost::system::error_code& ec );
  void closeSocket();

  net::local::stream_protocol::socket socket_;
  QuicklistClient *client_;
  std::queue<std::string> inQueue_;
  std::queue<std::string> outQueue_;

  //typedef boost::array<char, 4> SingleBuffer;
  //SingleBuffer data_;
  //std::vector<SingleBuffer> buffers_;
  net::streambuf data_;
  net::deadline_timer outQueueTimer_;
  std::weak_ptr<WebsocketSession> ws_;

  void onReceive(std::string const &msg);
  void fallback(std::string const &msg);
  void startOutQueueTimer();
  void onOutQueueTimer(boost::system::error_code const &);
  std::string bufferToString() const;

};

#endif // UNIXDOMAINSESSION_H
