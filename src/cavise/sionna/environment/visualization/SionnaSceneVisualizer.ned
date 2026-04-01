package cavise.sionna.environment.visualization;

simple SionnaSceneVisualizer like ISceneVisualizer
{
    parameters:
        @namespace(artery::sionna);
        @class(SionnaSceneVisualizer);

        string outputDir = default("renders");
        int spp = default(16);
        int width = default(1280);
        int height = default(720);
        double renderInterval @unit(s) = default(0s);
}
