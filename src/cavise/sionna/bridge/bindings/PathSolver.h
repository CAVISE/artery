#pragma once

// for ref.
#include <nanobind/nanobind.h>

#include <cavise/sionna/bridge/bindings/Modules.h>
#include <cavise/sionna/bridge/bindings/Paths.h>
#include <cavise/sionna/bridge/bindings/Scene.h>
#include <cavise/sionna/bridge/capabilities/Calling.h>

namespace artery::sionna::py {

    // Sionna path solver. These solvers generate paths objects containing calculated
    // signal paths, which may be visualized on render() call.
    class SIONNA_BRIDGE_API PathSolver
        : public SionnaRtModule
        , public InitPythonClassCapability {
    public:
        // IPythonClassIdentityCapability implementation.
        const char* className() const override;

        // Default constructor.
        PathSolver();
        // Wrap existing object, assuming it's PathSolver.
        explicit PathSolver(nanobind::object obj);

        // Calculate paths
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
