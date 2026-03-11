simple OpenCDAMessageGenerator
{
    parameters:
        @class(OpenCDAMessageGenerator);
        @display("i=block/source");

        int minEntities = default(1);
        int maxEntities = default(1);
        string idPrefix = default("dummy");
        int minId = default(1);
        int maxId = default(1);
}
