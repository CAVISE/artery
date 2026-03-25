package artery.sionna.environment.providers.scenefile;

import artery.sionna.environment.providers.ISceneConfigProvider;

//
// Static scene provider backed by an explicit scene file on disk.
//
simple SceneFileConfigProvider like ISceneConfigProvider
{
    parameters:
        @namespace(artery::sionna);
        @class(SceneFileConfigProvider);
        string sceneFile = default("");
}
