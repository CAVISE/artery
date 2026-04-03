package cavise.sionna.environment.config.scenes;

import cavise.sionna.environment.config.IBootstrapSceneConfigProvider;

simple FileSceneConfigProvider like IBootstrapSceneConfigProvider
{
    parameters:
        @class(FileSceneConfigProvider);

        string scenePath;
        bool mergeMeshes = default(true);
        bool mergeEquivalent = default(true);
}
