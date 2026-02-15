package cavise;

module CAPIManager
{
    parameters:
        typename HandlerType = default(cavise.ZmqCAPIConnectionHandler);

        // pass-through params for core
        string address = default("tcp://127.0.0.1:5555");
        bool bind = default(true);
        double period = default(1s);

        // pass-through params for handler
        string handlerAddress = default("tcp://127.0.0.1");
        int handlerPort = default(5555);
        bool handlerBind = default(false);
        int handlerReceiveTimeout = default(200);

        @display("bgb=600,220");

    submodules:
        core: ManagerCore {
            parameters:
                address = address;
                bind    = bind;
                period  = period;
                @display("p=160,110");
        }
        handler: HandlerType like ICAPIConnectionHandler {
            parameters:
                address        = handlerAddress;
                port           = handlerPort;
                bind           = handlerBind;
                receiveTimeout = handlerReceiveTimeout;
                @display("p=440,110");
        }
}