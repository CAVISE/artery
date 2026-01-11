#pragma once

#include "artery/sionna/bridge/Fwd.h"

#include <artery/sionna/bridge/Bindings.h>
#include <artery/sionna/bridge/Helpers.h>

#include <artery/sionna/bridge/bindings/Material.h>
#include <mitsuba/render/mesh.h>

#include <cstdint>
#include <string>
#include <variant>

NAMESPACE_BEGIN(artery)
NAMESPACE_BEGIN(sionna)

NAMESPACE_BEGIN(py)

// You forgot this one in the snippet; needed for Class<SionnaRtTag, ...>
PY_IDENTITY_TAG(SionnaRt, sionna.rt);
PY_IDENTITY_TAG(SceneObject, SceneObject);

// You already have material wrappers; use them as the typed surface.
PY_IDENTITY_TAG(RadioMaterial, RadioMaterial);
PY_IDENTITY_TAG(RadioMaterialBase, RadioMaterialBase);

/**
 * Minimal, typed wrapper around sionna.rt.SceneObject.
 *
 * Public API:
 *  - ctors (mesh / file)
 *  - name(), objectId()
 *  - radioMaterial(), setRadioMaterial(material), setRadioMaterial(name)
 *  - clone(...)
 *
 * No nb::object in the public API.
 */
MI_VARIANT
class SceneObject : public Class<SionnaRtTag, SceneObjectTag>
{
public:
    using Base = Class<SionnaRtTag, SceneObjectTag>;
    using Base::Base; // allow wrapping an existing python object internally

    SIONNA_IMPORT_CORE_TYPES(Point3f, Vector3f)
    SIONNA_IMPORT_RENDER_TYPES(Mesh)

    // Result of clone(): either another SceneObject or a Mitsuba Mesh.
    using CloneResult = std::variant<SceneObject, Mesh>;

public:
    /**
     * Construct from an existing Mitsuba mesh.
     *
     * Python: SceneObject(mi_mesh=..., name=..., radio_material=..., remove_duplicate_vertices=...)
     */
    SceneObject(
        Mesh mesh,
        const std::string& name,
        nb::object radioMaterial = nb::none(),
        bool removeDuplicateVertices = false)
        : Base{ctor(
            "mi_mesh"_a = std::move(mesh),
            "name"_a = name,
            "radio_material"_a = std::move(radioMaterial),
            "remove_duplicate_vertices"_a = removeDuplicateVertices
        )}
    {}

    /**
     * Construct by loading a mesh from file (.ply/.obj).
     *
     * Python: SceneObject(fname=..., name=..., radio_material=..., remove_duplicate_vertices=...)
     */
    SceneObject(
        const std::string& fname,
        const std::string& name,
        RadioMaterial<Float, Spectrum> radioMaterial,
        bool removeDuplicateVertices = false)
        : Base{ctor(
            "fname"_a = fname,
            "name"_a = name,
            "radio_material"_a = std::move(radioMaterial),
            "remove_duplicate_vertices"_a = removeDuplicateVertices
        )}
    {}

    std::string name() const
    {
        return sionna::access<std::string>(*bound_, "name");
    }

    std::uint32_t objectId() const
    {
        // Python returns int; in Sionna it is derived from the Mi mesh pointer (fits into uint32 in practice).
        return sionna::access<std::uint32_t>(*bound_, "object_id");
    }

    /**
     * Get radio material as typed wrapper.
     *
     * Note: The underlying python property is typically a Mitsuba BSDF / Sionna RadioMaterial object.
     * We wrap whatever object is there using the RadioMaterialBase wrapper.
     */
    RadioMaterialBase<Float, Spectrum> radioMaterial() const
    {
        return RadioMaterialBase<Float, Spectrum>(bound_->attr("radio_material"));
    }

    /**
     * Set by material object (typed wrapper).
     */
    void setRadioMaterial(const RadioMaterialBase<Float, Spectrum>& mat)
    {
        sionna::set(*bound_, "radio_material", mat);
    }

    /**
     * Set by material name (string) - only works if object already belongs to a Scene on python side.
     */
    void setRadioMaterial(const std::string& matName)
    {
        sionna::set(*bound_, "radio_material", matName);
    }

    /**
     * Clone object.
     *
     * Python: clone(name=None, as_mesh=False, props=None)
     *
     * We expose the same surface but return a typed variant:
     *  - asMesh=false -> SceneObject
     *  - asMesh=true  -> Mesh
     */
    CloneResult clone(const std::string* newName = nullptr, bool asMesh = false) const
    {
        nb::object nameArg = newName ? nb::cast(*newName) : nb::none();

        nb::object res = bound_->attr("clone")(
            "name"_a = std::move(nameArg),
            "as_mesh"_a = asMesh,
            "props"_a = nb::none()
        );

        if (asMesh) {
            // Expect Mitsuba Mesh
            return nb::cast<Mesh>(res);
        } else {
            // Expect SceneObject
            return SceneObject(std::move(res));
        }
    }
};

NAMESPACE_END(py)

NAMESPACE_END(sionna)
NAMESPACE_END(artery)
