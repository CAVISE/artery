#pragma once

#include <nanobind/nanobind.h>

#include <cavise/sionna/bridge/bindings/Modules.h>
#include <cavise/sionna/bridge/bindings/Paths.h>
#include <cavise/sionna/bridge/bindings/Scene.h>
#include <cavise/sionna/bridge/capabilities/Calling.h>

namespace artery::sionna::py {

    class SIONNA_BRIDGE_API PathSolver
        : public SionnaRtModule
        , public InitPythonClassCapability {
    public:
        const char* className() const override;

        PathSolver();
        explicit PathSolver(nanobind::object obj);

        std::string loopMode() const;
        void setLoopMode(const std::string& mode);

        Paths solve(
            const SionnaScene& scene,
            int maxDepth = 3,
            int maxNumPathsPerSrc = 1000000,
            int samplesPerSrc = 1000000,
            bool syntheticArray = true,
            bool los = true,
            bool specularReflection = true,
            bool diffuseReflection = false,
            bool refraction = true,
            bool diffraction = false,
            bool edgeDiffraction = false,
            bool diffractionLitRegion = true,
            int seed = 42) const;
    };

} // namespace artery::sionna::py
