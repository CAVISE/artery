package masquerade;

import cavise.application.CosimService;

simple Masquerade extends CosimService
{
    parameters:
        @namespace(cavise);
        @class(Masquerade);

        string masqueradeStrategy @enum("first", "swap", "random") = default("first");
}
