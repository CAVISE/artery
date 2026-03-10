package cavise.capi;

module CAPIManager
{
    parameters:
        string address = default("tcp://127.0.0.1:5555");
        int receiveTimeout = default(10000);
        int sendTimeout = default(10000);
        string handlerType = default("ZmqCAPIConnectionHandler");
        double updateInterval @unit(s) = default(1s);

        @display("bgb=600,220");

    submodules:
        core: CAPICore {
            parameters:
                updateInterval = updateInterval;
                @display("p=160,110");
        }
        connection_handler: <handlerType> like ICAPIConnectionHandler {
            parameters:
                address = address;
                receiveTimeout = receiveTimeout;
                sendTimeout = sendTimeout;
                @display("p=440,110");
        }
}
