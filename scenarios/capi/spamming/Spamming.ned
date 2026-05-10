package spamming;

import cavise.application.CosimService;

simple Spamming extends CosimService
{
    parameters:
        @namespace(cavise);
        @class(Spamming);

        int burstCount = default(5);
}
