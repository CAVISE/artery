package attacks.spamming;

import cavise.application.CosimService;

simple Spamming extends CosimService
{
    parameters:
        @class(Spamming);

        int burstCount = default(5);
}
