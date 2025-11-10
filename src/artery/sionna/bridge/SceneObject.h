#pragma once

#include <mitsuba/core/fwd.h>
#include <mitsuba/render/mesh.h>
#include <pybind11/pybind11.h>

#include "Helpers.h"
#include "Material.h"
#include "Constants.h"

namespace artery {

    namespace sionna {

        // Meshes, exported by Sionna.

        // SceneObject is a wrapper for sionna-rt's objects, that might be
        // physically preset in scene. Scene objects are bound to mitsuba meshes,
        // that are used to support INET's scene objects' configuration.
        // defined meshes might be found here: https://github.com/NVlabs/sionna-rt/blob/main/src/sionna/rt/scene.py
        MI_VARIANT class SceneObject : pybind11::object {
        public:
            using Point3d = typename mitsuba::CoreAliases<Float>::Point3d;
            using Point3f = typename mitsuba::CoreAliases<Float>::Point3f;
            using Vector3f = typename mitsuba::CoreAliases<Float>::Vector3f;
            using Mesh = typename mitsuba::Mesh<Float, Spectrum>;

            static SceneObject createFromMesh(const std::string& fname, const RadioMaterialBase<Float, Spectrum>& material, const std::string& name = "");
            static SceneObject createFromMesh(const Mesh& mesh, const RadioMaterialBase<Float, Spectrum>& material, const std::string& name = "");

            // Setters.
            void setOrientation(const Point3f& angles);
            void setPosition(const Point3d& pos);
            void setScaling(const Vector3f& scale);
            void setRadioMaterial(const RadioMaterialBase<Float, Spectrum>& material);
    
            // Getters.
            const Point3f& orientation() const;
            const Point3d& position() const;
            const Vector3f& scaling() const;
            const RadioMaterialBase<Float, Spectrum>& radioMaterial() const;
            
            const std::string& name() const;
            std::uint32_t objectId() const;
            
            void lookAt(const Point3f& target);

        };

    }  // namespace sionna

}  // namespace artery

MI_VARIANT SceneObject<Float, Spectrum>::SceneObject(pybind11::object&& obj)
    : pybind11::object{std::move(obj)}
{}

MI_VARIANT SceneObject<Float, Spectrum> SceneObject<Float, Spectrum>::createFromMesh(const std::string& fname, const RadioMaterialBase& material, const std::string& name) {
    using namespace pybind11::literals;

    pybind11::object sceneObject = sionna::sionnaRt().attr("SceneObject");
    return sceneObject(
        "fname"_a = fname,
        "material"_a = material,
        "name"_a = name
    );
}

MI_VARIANT const std::string& SceneObject<Float, Spectrum>::name() const {
    return sionna::access<std::string>(this, "name");
}

MI_VARIANT std::uint32_t SceneObject<Float, Spectrum>::objectId() const {
    return sionna::access<std::uint32_t>(this, "object_id");
}

MI_VARIANT void SceneObject<Float, Spectrum>::setPosition(const Point3d& pos) {
    sionna::set(this, "position", pos);
}

MI_VARIANT const typename SceneObject<Float, Spectrum>::Point3d& SceneObject<Float, Spectrum>::position() const {

}
