#include "artery/sionna/bridge/Fwd.h"
#include "artery/sionna/environment/MitsubaShape.h"

#include "PhysicalObject.h"

SIONNA_BRIDGE_EXTERN_CLASS(artery::sionna::PhysicalObject)

using namespace artery::sionna;

MI_VARIANT
PhysicalObject<Float, Spectrum>::PhysicalObject(nanobind::object obj)
    : py_(std::move(obj))
{}

MI_VARIANT
PhysicalObject<Float, Spectrum>::PhysicalObject(mitsuba::ref<Mesh> mesh)
    : py_(std::move(mesh))
{}

MI_VARIANT
PhysicalObject<Float, Spectrum>::PhysicalObject(const std::string& fname, const std::string& name, mitsuba::ref<RadioMaterial> material)
    : py_(fname, name, material)
{}

MI_VARIANT
const inet::Coord& PhysicalObject<Float, Spectrum>::getPosition() const {
    position_ = Compat::toCoord(py_.position());
    return position_;
}

MI_VARIANT
const inet::EulerAngles& PhysicalObject<Float, Spectrum>::getOrientation() const {
    orientation_ = Compat::toEuler(py_.orientation());
    return orientation_;
}

MI_VARIANT
const inet::ShapeBase* PhysicalObject<Float, Spectrum>::getShape() const {
    if (!shape_) {
        shape_ = std::make_shared<MitsubaShape<Float, Spectrum>>(py_.mesh());
    }
    return shape_.get();
}

MI_VARIANT
const inet::physicalenvironment::IMaterial* PhysicalObject<Float, Spectrum>::getMaterial() const {
    if (!material_) {
        material_.emplace(py_.material());
    }
    return &material_.value();
}

MI_VARIANT
double PhysicalObject<Float, Spectrum>::getLineWidth() const {
    return 1.0;
}

MI_VARIANT
const cFigure::Color& PhysicalObject<Float, Spectrum>::getLineColor() const {
    return getFillColor();
}

MI_VARIANT
const cFigure::Color& PhysicalObject<Float, Spectrum>::getFillColor() const {
    color_ = Compat::toColor(py_.material().color());
    return color_;
}

MI_VARIANT
double PhysicalObject<Float, Spectrum>::getOpacity() const {
    return 1.0;
}

MI_VARIANT
const char* PhysicalObject<Float, Spectrum>::getTexture() const {
    return "";
}

MI_VARIANT
const char* PhysicalObject<Float, Spectrum>::getTags() const {
    return "";
}
