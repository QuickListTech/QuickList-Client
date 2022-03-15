#ifndef NAMESPACE_H
#define NAMESPACE_H

#include <boost/beast.hpp>
#include <boost/json.hpp>

namespace beast = boost::beast;
namespace http = beast::http;
namespace websocket = beast::websocket;
namespace net = boost::asio;
namespace ssl = boost::asio::ssl;
namespace json = boost::json;
using tcp = boost::asio::ip::tcp;

extern json::value config;

void fail ( beast::error_code ec, char const* what );

#endif
