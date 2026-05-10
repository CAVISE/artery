package spamming;

import cavise.application.CosimService;

simple Spamming extends CosimService
{
    parameters:
        @class(::cavise::Spamming);

        int burstCount = default(5);
}
