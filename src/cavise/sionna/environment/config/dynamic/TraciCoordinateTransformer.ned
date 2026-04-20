package cavise.sionna.environment.config.dynamic;

//
// Provider for SUMO<->scene coordinate transforms.
//
moduleinterface ITraciCoordinateTransformer
{
}

simple TraciCoordinateTransformer like ITraciCoordinateTransformer
{
    parameters:
        @namespace(artery::sionna);
        @class(TraciCoordinateTransformer);

        xml remapMatrix = default(xmldoc("transforms.xml", "/transforms/identity"));
        xml rotationMatrix = default(xmldoc("transforms.xml", "/transforms/identity"));
        xml translationVector = default(xmldoc("transforms.xml", "/transforms/zero"));
}
