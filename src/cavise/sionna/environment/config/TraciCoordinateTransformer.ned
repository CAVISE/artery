package cavise.sionna.environment.config;

simple TraciCoordinateTransformer like ITraciCoordinateTransformer
{
    parameters:
        @namespace(artery::sionna);
        @class(TraciCoordinateTransformer);

        // 2D rotation matrix:
        // [r00 r01]
        // [r10 r11]
        double r00 = default(1.0);
        double r01 = default(0.0);
        double r10 = default(0.0);
        double r11 = default(1.0);

        // Translation applied after rotation.
        double translationX @unit(m) = default(0m);
        double translationY @unit(m) = default(0m);
        double translationZ @unit(m) = default(0m);
}
