package cavise;

moduleinterface ICAPIConnectionHandler
{
    parameters:
        @display("i=block/socket");

        // URL for OpenCDA server
        string address;
}

simple ZmqCAPIConnectionHandler like ICAPIConnectionHandler
{
    parameters:
        @class(cavise::ZmqCAPIConnectionHandler);

        string address = default("tcp://127.0.0.1:5555");
        // Timeout for receive call, Artery will wait
        // up that number of milliseconds for OpenCDA.
        // Leave this zero to wait forever.
        int receiveTimeout = default(10000);
        // Same as above, but for send.
        int sendTimeout = default(10000);
}
