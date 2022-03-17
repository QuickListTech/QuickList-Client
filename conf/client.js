{
    "client" : {
        "host" : "192.168.1.242",
        "port" : 8080,
        "privatekey" : "tls/clientB.key.pem",
        "certificate" : "tls/clientB.cert.pem"
    },

    "UDS": {
        "daemonize" : true,
        "rundir" : "./",
        "pidfile" : "quicklist.pid",
        "user" : "petr",
        "group" : "petr"
    }
}
