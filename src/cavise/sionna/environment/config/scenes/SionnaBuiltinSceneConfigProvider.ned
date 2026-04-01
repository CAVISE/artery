package cavise.sionna.environment.config.scenes;

import cavise.sionna.environment.config.IBootstrapSceneConfigProvider;

simple SionnaBuiltinSceneConfigProvider like IBootstrapSceneConfigProvider
{
    parameters:
        @class(SionnaBuiltinSceneConfigProvider);

        string scene = default("munich");
}
