#include "PhysicalObject.h"

#include <cavise/sionna/environment/Compat.h>

#include <algorithm>
#include <cstdint>

using namespace artery::sionna;

namespace {

    cFigure::Color toColor(const py::RadioMaterialBase::TColor& rgb) {
        return cFigure::Color(
            static_cast<uint8_t>(std::clamp(std::get<0>(rgb), 0.0F, 1.0F) * 255.0F),
            static_cast<uint8_t>(std::clamp(std::get<1>(rgb), 0.0F, 1.0F) * 255.0F),
            static_cast<uint8_t>(std::clamp(std::get<2>(rgb), 0.0F, 1.0F) * 255.0F));
    }

} // namespace

PhysicalObject::PhysicalObject(nanobind::object obj)
    : py_(std::move(obj)) {
}

PhysicalObject::PhysicalObject(mitsuba::ref<mitsuba::Resolve::Mesh> mesh)
    : py_(std::move(mesh), py::RadioMaterial("physical_object_material")) {
}

PhysicalObject::PhysicalObject(const std::string& fname, const std::string& name, py::RadioMaterial material)
    : py_(fname, name, material) {
}

const inet::Coord& PhysicalObject::getPosition() const {
    position_ = convert<inet::Coord>(py_.position());
    return position_;
}

const inet::EulerAngles& PhysicalObject::getOrientation() const {
    orientation_ = convert<inet::EulerAngles>(py_.orientation());
    return orientation_;
}

const inet::ShapeBase* PhysicalObject::getShape() const {
    if (!shape_) {
        shape_ = std::make_shared<MitsubaShape>(py_.mesh());
    }
    return shape_.get();
}

const inet::physicalenvironment::IMaterial* PhysicalObject::getMaterial() const {
    if (!material_) {
        material_.emplace(py_.material());
    }
    return &material_.value();
}

double PhysicalObject::getLineWidth() const {
    return 1.0;
}

const cFigure::Color& PhysicalObject::getLineColor() const {
    return getFillColor();
}

const cFigure::Color& PhysicalObject::getFillColor() const {
    color_ = toColor(py_.material().color());
    return color_;
}

double PhysicalObject::getOpacity() const {
    return 1.0;
}

const char* PhysicalObject::getTexture() const {
    return "";
}

const char* PhysicalObject::getTags() const {
    return "";
}

PhysicalObject::~PhysicalObject() = default;
