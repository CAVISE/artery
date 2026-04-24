package artery.inet;

import cavise.sionna.environment.radio.ISionnaReceiver;
import cavise.sionna.environment.radio.ISionnaTransmitter;
import inet.physicallayer.ieee80211.packetlevel.Ieee80211ScalarRadio;

module VanetRadio extends Ieee80211ScalarRadio
{
    parameters:
        @class(VanetRadio);
        string sionnaTransmitterType = default("");
        string sionnaReceiverType = default("");

        // signal on incoming radio frame
        @signal[RadioFrame](type=cPacket);

    submodules:
        sionnaTransmitter: <sionnaTransmitterType> like ISionnaTransmitter if sionnaTransmitterType != "" {
            parameters:
                @display("p=180,80");
        }
        sionnaReceiver: <sionnaReceiverType> like ISionnaReceiver if sionnaReceiverType != "" {
            parameters:
                @display("p=180,120");
        }
}
