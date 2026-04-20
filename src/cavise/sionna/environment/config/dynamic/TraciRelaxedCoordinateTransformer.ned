package cavise.sionna.environment.config.dynamic;

simple TraciRelaxedCoordinateTransformer extends TraciCoordinateTransformer
{
    parameters:
        @class(TraciRelaxedCoordinateTransformer);

        string roadMeshName = default("");
        double baseOffsetX @unit(m) = default(0m);
        double baseOffsetY @unit(m) = default(0m);
        double baseOffsetZ @unit(m) = default(0m);
}
