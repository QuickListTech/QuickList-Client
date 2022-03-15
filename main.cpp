#include <cstdlib>
#include <functional>
#include <iostream>
#include <fstream>
#include <boost/json/src.hpp>
#include "quicklistclient.h"

using std::string;
using std::ifstream;
using std::getline;

json::value config = {
     {
          "client", {
               { "host", "api.quicklist.tech"},
               { "port", 5000 },
               { "reconnect", 2 }
          }
     },

     {
          "UDS", {
               { "file", "/tmp/quicklist.sock" },
               { "queue" , true }
          }
     }
};

void configure ( json::value const &custom, json::value &actual )
{
     auto &obj = custom.as_object();

     for ( auto it = obj.begin(); it < obj.end(); ++it ) {
          auto &o = actual.as_object();
          auto v = o.find ( it->key() );

          if ( v == o.end() ) {
               o[it->key()] = it->value();
          } else {
               if ( it->value().is_object() ) {
                    configure ( it->value(), v->value() );
               } else {
                    if ( v->value() != it->value() ) {
                         o[it->key()] = it->value();
                    }
               }
          }
     }
}

int main ( int argc, char** argv )
{
     if ( argc != 3 || ( argc == 3 && strcmp ( argv[1],"-c" ) != 0 ) ) {
          std::cerr << "Usage: quicklist-client -c client.js" << std::endl;
          return EXIT_FAILURE;
     }

     ifstream inputFile ( argv[2],  ifstream::in );
     string line;
     json::stream_parser p;
     json::error_code ec;

     while ( getline ( inputFile, line ) ) {
          p.write ( line, ec );

          if ( ec ) {
               std::cerr << "Configuration file could not parse at line: " << line << std::endl;
               return EXIT_FAILURE;
          }
     }

     p.finish();
     configure ( p.release(), config );

     json::value client = config.as_object() ["client"];
     json::value cert = client.get_object() ["certificate"];
     json::value pKey = client.get_object() ["privatekey"];

     if ( cert == nullptr || pKey == nullptr ) {
          std::cerr << "client.certificate or client.privatekey is missing" << std::endl;
          return EXIT_FAILURE;
     }

     QuicklistClient qlc;
     qlc.run();

     return EXIT_SUCCESS;
}

