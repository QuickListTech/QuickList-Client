QuickList-Client* Connects to QuickList websocket * Forwards JSON api calls* Automatically reconnects if connection goes down* By default queues messages while connection is down, returning QUEUE message via Unix domain socket* Check for connection by sending STATUS to the domain socket (returns either UP or DOWN) 