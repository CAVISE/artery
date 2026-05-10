package replay;

import cavise.application.CosimService;

simple Replay extends CosimService
{
    parameters:
        @namespace(cavise);
        @class(Replay);

        string replayStrategy @enum("first", "repeat") = default("first");
        int replayCount = default(3);
}
