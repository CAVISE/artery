#pragma once

#include <nanobind/nanobind.h>

#include <cavise/sionna/bridge/SionnaBridge.h>
#include <cavise/sionna/environment/Material.h>
#include <cavise/sionna/environment/MitsubaShape.h>

#include <inet/common/geometry/base/ShapeBase.h>
#include <inet/environment/contract/IPhysicalObject.h>

#include <mitsuba/core/object.h>

#include <string>

namespace artery::sionna {

    /**
     * @brief Adapter that exposes a Sionna SceneObject as an INET IPhysicalObject.
     *
     * Geometry access is limited in the current bridge, so we provide a lightweight
     * dummy shape by default. Consumers that need a richer shape can pass one at
     * construction time.
     */
    class PhysicalObject
        : public inet::physicalenvironment::IPhysicalObject {
    public:
        explicit PhysicalObject(nanobind::object obj);
        explicit PhysicalObject(mitsuba::ref<mitsuba::Resolve::Mesh> mesh);

        PhysicalObject(const std::string& fname, const std::string& name, py::RadioMaterial material);

        // inet::physicalenvironment::IPhysicalObject implementation.
        const inet::Coord& getPosition() const override;
        const inet::EulerAngles& getOrientation() const override;

        const inet::ShapeBase* getShape() const override;
        const inet::physicalenvironment::IMaterial* getMaterial() const override;

        double getLineWidth() const override;
        const cFigure::Color& getLineColor() const override;
        const cFigure::Color& getFillColor() const override;
        double getOpacity() const override;

        const char* getTexture() const override;
        const char* getTags() const override;

        virtual ~PhysicalObject();

    private:
        py::SceneObject py_;

        mutable inet::Coord position_;
        mutable inet::EulerAngles orientation_;

        mutable cFigure::Color color_;
        mutable std::optional<artery::sionna::RadioMaterial> material_;
        mutable std::shared_ptr<inet::ShapeBase> shape_;
    };

} // namespace artery::sionna
