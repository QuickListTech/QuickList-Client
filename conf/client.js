{
    "remote" : {
        "host" : "api.quicklist.tech",
        "port" : 5000,
        "privatekey" : "tls/clientB.key.pem",
        "certificate" : "tls/clientB.cert.pem"
    },
    "UDS": {
        "daemonize" : false,
        "rundir" : "./",
        "pidfile" : "/var/run/quicklist-client.pid",
        "user" : "petr",
        "group" : "petr",
        "log" : "/var/log/quicklist-client.log",
        "loglevel" : "info"
    }

}
