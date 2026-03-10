simple DummyCapiDealer
{
    parameters:
        @class(DummyCapiDealer);
        @display("i=block/source");

        string address = default("tcp://127.0.0.1:5555");
        string registryModule = default("^.registry");
        double startDelay @unit(s) = default(0.1s);
        double sendInterval @unit(s) = default(1s);

        int receiveTimeout = default(100);
        int sendTimeout = default(1000);
}
