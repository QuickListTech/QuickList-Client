# QuickList-Client* Makes a QuickList connection per Unix socket connection* Forwards JSON API calls and passes responses to Unix domain socket clients* Automatically reconnects if connection goes down* By default queues messages while connection is down, returning QUEUE message* Check for connection by sending STATUS to the domain socket (returns either UP or DOWN) * Can be run either as a daemon or terminal application* Developed with Boost 1.78* Boost Log levels used are: info, warning, error* Licensed under Apache 2.0# Sample Configuration File```{    "remote" : {        "host" : "api.quicklist.tech",        "port" : 5000,	"reconnect" : 2,        "privatekey" : "/etc/quicklist/tls/client.key.pem",        "certificate" : "/etc/quicklist/tls/client.cert.pem"    },    "UDS": {        "daemonize" : true,        "rundir" : "/",        "pidfile" : "/var/run/quicklist.pid",        "user" : "quicklist",        "group" : "quicklist",	"file" : "/tmp/quicklist.sock",	"queue" : true,	"log" : "/var/log/quicklist/quicklist.log",	"loglevel" : "info"    }}```