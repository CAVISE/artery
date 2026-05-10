package flooding;

import cavise.application.CosimService;

simple Flooding extends CosimService
{
    parameters:
        @class(::cavise::Flooding);

        string floodingStrategy @enum("random", "periods") = default("random");
        double dropProbability = default(0.25);
        double clearPeriodMean @unit(s) = default(1s);
        double floodedPeriodMean @unit(s) = default(0.2s);
}
