package attacks.masquerade;

import cavise.application.CosimService;

simple Masquerade extends CosimService
{
    parameters:
        @class(Masquerade);

        string masqueradeStrategy @enum("first", "swap", "random") = default("first");
}
