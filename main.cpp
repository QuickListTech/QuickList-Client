// SPDX-FileCopyrightText: 2022 Petr Janda admin@quicklist.tech
// SPDX-License-Identifier: Apache-2.0

#include <cstdlib>
#include <functional>
#include <iostream>
#include <fstream>
#include <boost/json/src.hpp>
#include "quicklistclient.h"
#include <pwd.h>
#include <grp.h>

#include "log.h"

using std::string;
using std::ifstream;
using std::ofstream;
using std::getline;

Log logger;
static string confFile;
json::value config = {
     {
          "remote", {
               { "host", "api.quicklist.tech"},
               { "port", 5000 },
               { "reconnect", 2 }
          }
     },

     {
          "UDS", {
               { "file", "/tmp/quicklist.sock" },
               { "queue" , true },
               { "daemonize" , false },
               { "rundir" , "/"},
               { "pidfile", "/var/run/quicklist-client.pid"},
               { "log", "quicklist.log"}
          }
     }
};

void customConfig ( json::value const &custom, json::value &actual )
{
     auto &obj = custom.as_object();

     for ( auto it = obj.begin(); it < obj.end(); ++it ) {
          auto &o = actual.as_object();
          auto v = o.find ( it->key() );

          if ( v == o.end() ) {
               o[it->key()] = it->value();
          } else {
               if ( it->value().is_object() ) {
                    customConfig ( it->value(), v->value() );
               } else {
                    if ( v->value() != it->value() ) {
                         o[it->key()] = it->value();
                    }
               }
          }
     }
}

void configure()
{
     ifstream inputFile ( confFile,  ifstream::in );
     string line;
     json::stream_parser p;
     json::error_code ec;

     while ( getline ( inputFile, line ) ) {
          p.write ( line, ec );

          if ( ec ) {
               std::cerr << "Configuration file could not parse at line: " << line << std::endl;
               exit(EXIT_FAILURE);
          }
     }

     p.finish();
     customConfig ( p.release(), config );
}

void reload(int sig)
{
     logger.close();
     logger.open();

     configure();

     logger.warn() << "SIGNAL(" << sig << ") received. Reloading QuickList-Client daemon..." << std::endl;
}

void skeleton_daemon()
{
     json::value server = config.as_object()["UDS"];
     string pidFile = server.as_object()[ "pidfile" ].as_string().c_str();
     json::string *usr = server.as_object()["user"].if_string();
     json::string *grp = server.as_object()["group"].if_string();

     string user;
     string group;

     if (usr) {
          user = usr->c_str();
     }

     if (grp) {
          group = grp->c_str();
     }

     ifstream checkPid ( pidFile );
     if ( checkPid.good() ) {
          int pid;
          checkPid >> pid;

          int ret = kill ( pid, 0 );

          if ( ret == 0 || ( ret == -1 && errno == EPERM ) ) {
               std::cerr << "QuickList Server already running" << std::endl;
               exit ( EXIT_FAILURE );
          } else {
               std::cout << "Removing stale pid file" << std::endl;
               std::system ( ( string ( "rm " ) + pidFile ).c_str() );
          }
     }

     checkPid.close();

     struct passwd *susr;
     struct group *sgrp;
     susr = getpwnam ( user.c_str() );
     sgrp = getgrnam ( group.c_str() );

     if ( !susr ) {
          std::cerr << "User doesn't exist" << std::endl;
          exit(EXIT_FAILURE);
     }

     if ( !sgrp ) {
          std::cerr << "Group doesn't exist" << std::endl;
          exit(EXIT_FAILURE);
     }

     pid_t pid;

     pid = fork();

     if ( pid < 0 ) {
          exit ( EXIT_FAILURE );
     }

     if ( pid > 0 ) {
          exit ( EXIT_SUCCESS );
     }

     if ( setsid() < 0 ) {
          exit ( EXIT_FAILURE );
     }

     signal ( SIGCHLD, SIG_IGN );
     signal ( SIGHUP, reload );

     pid = fork();

     if ( pid < 0 ) {
          exit ( EXIT_FAILURE );
     }

     if ( pid > 0 ) {
          exit ( EXIT_SUCCESS );
     }

     umask ( 0 );
     chdir ( server.as_object()["rundir"].as_string().c_str() );

     ofstream writePid ( pidFile, std::ios::out );
     writePid << getpid();
     writePid.close();

     setuid ( susr->pw_uid );
     setgid ( sgrp->gr_gid );

     int x;
     for ( x = sysconf ( _SC_OPEN_MAX ); x>=0; x-- ) {
          close ( x );
     }
}


int main ( int argc, char** argv )
{
     if ( argc != 3 || ( argc == 3 && strcmp ( argv[1],"-c" ) != 0 ) ) {
          std::cerr << "Usage: quicklist-client -c client.js" << std::endl;
          return EXIT_FAILURE;
     }

     confFile = argv[2];
     configure();

     json::value remote = config.as_object() ["remote"];
     json::value cert = remote.get_object() ["certificate"];
     json::value pKey = remote.get_object() ["privatekey"];

     if ( cert == nullptr || pKey == nullptr ) {
          std::cerr << "client.certificate or client.privatekey is missing" << std::endl;
          return EXIT_FAILURE;
     }

     json::value server = config.as_object()["UDS"];

     if (server.as_object()["daemonize"].as_bool())
     {
          skeleton_daemon();
     }

     string logFile = server.as_object()["log"].as_string().c_str();

     logger.file(logFile);
     logger.open();

     logger.info() << "Starting QuickList-Client daemon." << std::endl;

     QuicklistClient qlc;

     qlc.run();

     return EXIT_SUCCESS;
}

