package cavise.capi;

simple CAPICore {
    parameters:
        @signal[capi.init](type=bool);
        @signal[capi.step](type=simtime_t);
        @signal[capi.close](type=bool);
        double updateInterval @unit(s) = default(1s);
}
