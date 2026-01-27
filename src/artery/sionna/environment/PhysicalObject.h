#pragma once

#include <artery/sionna/bridge/Fwd.h>
#include <artery/sionna/bridge/Helpers.h>
#include <artery/sionna/bridge/bindings/SceneObject.h>
#include <inet/common/geometry/common/Coord.h>
#include <inet/common/geometry/common/EulerAngles.h>
#include <inet/common/geometry/object/LineSegment.h>
#include <inet/common/geometry/base/ShapeBase.h>
#include <inet/environment/contract/IPhysicalObject.h>

#include <mitsuba/core/object.h>

#include <string>
#include <tuple>
#include <algorithm>
#include <cmath>
#include <nanobind/nanobind.h>

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
    SIONNA_IMPORT_BRIDGE_TYPES(RadioMaterial)

    explicit PhysicalObject(nanobind::object obj);

    PhysicalObject(const Mesh& mesh);
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
    cFigure::Color tupleToColor(const std::tuple<float, float, float>& t);

private:
    py::SceneObject<Float, Spectrum> py_;

    mutable inet::Coord position_;
    mutable inet::EulerAngles orientation_;

    mutable cFigure::Color color_;
    mutable RadioMaterial material_;
};

NAMESPACE_END(sionna)
NAMESPACE_END(artery)

MI_VARIANT
cFigure::Color artery::sionna::PhysicalObject<Float, Spectrum>::tupleToColor(const std::tuple<float, float, float>& t) {
    auto toChannel = [](float v) -> unsigned char {
        float clamped = std::clamp(v, 0.0f, 1.0f);
        return static_cast<unsigned char>(std::lround(clamped * 255.0f));
    };

    return cFigure::Color(
        toChannel(std::get<0>(t)),
        toChannel(std::get<1>(t)),
        toChannel(std::get<2>(t)));
}

MI_VARIANT
const inet::Coord& artery::sionna::PhysicalObject<Float, Spectrum>::getPosition() const {
    const Point3f p = py_.position();
    position_ = inet::Coord(static_cast<double>(p.x()), static_cast<double>(p.y()), static_cast<double>(p.z()));
    return position_;
}

MI_VARIANT
const inet::EulerAngles& artery::sionna::PhysicalObject<Float, Spectrum>::getOrientation() const {
    const Vector3f o = py_.orientation();
    orientation_ = inet::EulerAngles(static_cast<double>(o.x()), static_cast<double>(o.y()), static_cast<double>(o.z()));
    return orientation_;
}

MI_VARIANT
const inet::ShapeBase* artery::sionna::PhysicalObject<Float, Spectrum>::getShape() const {
    return nullptr;
}

MI_VARIANT
const inet::physicalenvironment::IMaterial* artery::sionna::PhysicalObject<Float, Spectrum>::getMaterial() const {
    material_ = RadioMaterial(py_.material());
    return &material_;
}

MI_VARIANT
double artery::sionna::PhysicalObject<Float, Spectrum>::getLineWidth() const {
    return 1.0;
}

MI_VARIANT
const cFigure::Color& artery::sionna::PhysicalObject<Float, Spectrum>::getLineColor() const {
    return getFillColor();
}

MI_VARIANT
const cFigure::Color& artery::sionna::PhysicalObject<Float, Spectrum>::getFillColor() const {
    color_ = tupleToColor(py_.material().color());
    return color_;
}

MI_VARIANT
double artery::sionna::PhysicalObject<Float, Spectrum>::getOpacity() const {
    return 1.0;
}

MI_VARIANT
const char* artery::sionna::PhysicalObject<Float, Spectrum>::getTexture() const {
    return "";
}

MI_VARIANT
const char* artery::sionna::PhysicalObject<Float, Spectrum>::getTags() const {
    return "";
}
