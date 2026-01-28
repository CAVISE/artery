#include "PhysicalObject.h"

NAMESPACE_BEGIN(artery)
NAMESPACE_BEGIN(sionna)

SIONNA_INSTANTIATE_CLASS(PhysicalObject)

MI_VARIANT
PhysicalObject<Float, Spectrum>::PhysicalObject(nanobind::object obj)
    : py_(std::move(obj))
{}

MI_VARIANT
PhysicalObject<Float, Spectrum>::PhysicalObject(const Mesh& mesh)
    : py_(mesh)
{}

MI_VARIANT
PhysicalObject<Float, Spectrum>::PhysicalObject(const std::string& fname, const std::string& name, mitsuba::ref<RadioMaterial> material)
    : py_(fname, name, material)
{}

MI_VARIANT
const inet::Coord& PhysicalObject<Float, Spectrum>::getPosition() const {
    position_ = Compat::convert(py_.position());
    return position_;
}

MI_VARIANT
const inet::EulerAngles& PhysicalObject<Float, Spectrum>::getOrientation() const {
    orientation_ = Compat::convert(py_.orientation());
    return orientation_;
}

MI_VARIANT
const inet::ShapeBase* PhysicalObject<Float, Spectrum>::getShape() const {
    return shape_.get();
}

MI_VARIANT
const inet::physicalenvironment::IMaterial* PhysicalObject<Float, Spectrum>::getMaterial() const {
    material_ = RadioMaterial(py_.material());
    return &material_;
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
    color_ = Compat::convert(py_.material().color());
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

NAMESPACE_END(sionna)
NAMESPACE_END(artery)
