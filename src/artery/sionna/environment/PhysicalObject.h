#pragma once

#include <nanobind/nanobind.h>

#include <artery/sionna/bridge/Fwd.h>
#include <artery/sionna/bridge/Helpers.h>
#include <artery/sionna/bridge/SionnaBridge.h>
#include <artery/sionna/environment/Material.h>

#include <inet/common/geometry/base/ShapeBase.h>
#include <inet/environment/contract/IPhysicalObject.h>

#include <mitsuba/core/object.h>

#include <optional>
#include <string>

NAMESPACE_BEGIN(artery)
NAMESPACE_BEGIN(sionna)

/**
 * @brief Adapter that exposes a Sionna SceneObject as an INET IPhysicalObject.
 *
 * Geometry access is limited in the current bridge, so we provide a lightweight
 * dummy shape by default. Consumers that need a richer shape can pass one at
 * construction time.
 */
MI_VARIANT class PhysicalObject : public inet::physicalenvironment::IPhysicalObject {
public:
    SIONNA_IMPORT_CORE_TYPES(Point3f, Vector3f)
    SIONNA_IMPORT_RENDER_TYPES(Mesh)
    SIONNA_IMPORT_BRIDGE_TYPES(RadioMaterial, Compat)

    explicit PhysicalObject(nanobind::object obj);
    explicit PhysicalObject(mitsuba::ref<Mesh> mesh);

    PhysicalObject(const std::string& fname, const std::string& name, mitsuba::ref<RadioMaterial> material);

    // inet::physicalenvironment::IPhysicalObject implementation.
    virtual const inet::Coord& getPosition() const override;
    virtual const inet::EulerAngles& getOrientation() const override;

    virtual const inet::ShapeBase *getShape() const override;
    virtual const inet::physicalenvironment::IMaterial *getMaterial() const override;

    virtual double getLineWidth() const override;
    virtual const cFigure::Color& getLineColor() const override;
    virtual const cFigure::Color& getFillColor() const override;
    virtual double getOpacity() const override;

    virtual const char *getTexture() const override;
    virtual const char *getTags() const override;

private:
    py::SceneObject<Float, Spectrum> py_;

    mutable inet::Coord position_;
    mutable inet::EulerAngles orientation_;

    mutable cFigure::Color color_;
    mutable std::optional<artery::sionna::RadioMaterial<Float, Spectrum>> material_;
    mutable std::shared_ptr<inet::ShapeBase> shape_;
};

SIONNA_EXTERN_CLASS(PhysicalObject)

NAMESPACE_END(sionna)
NAMESPACE_END(artery)
