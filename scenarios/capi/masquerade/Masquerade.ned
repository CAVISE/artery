package masquerade;

import cavise.application.CosimService;

simple Masquerade extends CosimService
{
    parameters:
        @class(::cavise::Masquerade);

        string masqueradeStrategy @enum("first", "swap", "random") = default("first");
}
