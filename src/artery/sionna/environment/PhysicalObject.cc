#include "PhysicalObject.h"
#include "MitsubaShape.h"

NAMESPACE_BEGIN(artery)
NAMESPACE_BEGIN(sionna)

MI_VARIANT
PhysicalObject<Float, Spectrum>::PhysicalObject(nanobind::object obj)
    : py_(std::move(obj)) {}

MI_VARIANT
PhysicalObject<Float, Spectrum>::PhysicalObject(const Mesh& mesh)
    : shape_(std::make_shared<mitsuba::TriangleMesh>(mesh)) {}

MI_VARIANT
PhysicalObject<Float, Spectrum>::PhysicalObject(const std::string& fname, const std::string& name, mitsuba::ref<RadioMaterial> material) {
    shape_ = std::make_shared<mitsuba::TriangleMesh>(fname, name, material);
}

MI_VARIANT
cFigure::Color PhysicalObject<Float, Spectrum>::tupleToColor(const std::tuple<float, float, float>& t) {
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
const inet::Coord& PhysicalObject<Float, Spectrum>::getPosition() const {
    const Point3f p = py_.position();
    position_ = inet::Coord(compat::toScalar(p.x()), compat::toScalar(p.y()), compat::toScalar(p.z()));
    return position_;
}

MI_VARIANT
const inet::EulerAngles& PhysicalObject<Float, Spectrum>::getOrientation() const {
    const Vector3f o = py_.orientation();
    orientation_ = inet::EulerAngles(compat::toScalar(o.x()), compat::toScalar(o.y()), compat::toScalar(o.z()));
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
    color_ = tupleToColor(py_.material().color());
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

SIONNA_INSTANTIATE_CLASS(PhysicalObject)

NAMESPACE_END(sionna)
NAMESPACE_END(artery)
