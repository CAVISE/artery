#pragma once

#include <pybind11/pybind11.h>

namespace artery {

    namespace sionna {

        // SceneObject is a wrapper for sionna-rt's objects, that might be
        // physically preset in scene. Scene objects are bound to mitsuba meshes,
        // that are used to support INET's scene objects' configuration.
        // defined meshes might be found here: https://github.com/NVlabs/sionna-rt/blob/main/src/sionna/rt/scene.py
        class SceneObject {};

        inline static

    }  // namespace sionna

}  // namespace artery