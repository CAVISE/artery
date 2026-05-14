#include "PathSolver.h"

#include <cavise/sionna/bridge/Helpers.h>

using namespace artery::sionna;
using namespace artery::sionna::literals;

const char* py::PathSolver::className() const {
    return "PathSolver";
}

py::PathSolver::PathSolver() {
    InitPythonClassCapability::init();
}

py::PathSolver::PathSolver(nanobind::object obj) {
    WrapPythonClassCapability::init(std::move(obj));
}

py::Paths py::PathSolver::solve(
    const SionnaScene& scene,
    const PathSolverOptions& options) const {
    return py::Paths(sionna::call<nanobind::object>(
        bound_,
        "__call__",
        "scene"_a = scene,
        "max_depth"_a = options.maxDepth,
        "max_num_paths_per_src"_a = options.maxNumPathsPerSrc,
        "samples_per_src"_a = options.samplesPerSrc,
        "synthetic_array"_a = options.syntheticArray,
        "los"_a = options.los,
        "specular_reflection"_a = options.specularReflection,
        "diffuse_reflection"_a = options.diffuseReflection,
        "refraction"_a = options.refraction,
        "diffraction"_a = options.diffraction,
        "edge_diffraction"_a = options.edgeDiffraction,
        "diffraction_lit_region"_a = options.diffractionLitRegion,
        "seed"_a = options.seed));
}
