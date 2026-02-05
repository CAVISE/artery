#pragma once

#include <artery/sionna/bridge/Fwd.h>
#include <artery/sionna/bridge/Compat.h>

#include <mitsuba/core/ray.h>
#include <mitsuba/core/bbox.h>
#include <mitsuba/render/mesh.h>
#include <mitsuba/render/scene.h>

#include <inet/common/geometry/base/ShapeBase.h>

#include <drjit/array.h>

NAMESPACE_BEGIN(artery)
NAMESPACE_BEGIN(sionna)

/**
 * @brief ShapeBase adapter that uses a Mitsuba scene to intersect a mesh; bbox
 * size is reported for metadata only.
 *
 * Intersection is performed via Mitsuba's acceleration structure
 * (ray_intersect_preliminary); if no scene is available, intersection fails.
 */
MI_VARIANT
class MitsubaShape
    : public inet::ShapeBase {
public:
    SIONNA_IMPORT_CORE_TYPES(BoundingBox3f, Point3f, Vector3f)
    SIONNA_IMPORT_RENDER_TYPES(Scene, Ray3f, Mesh, PreliminaryIntersection3f)
    SIONNA_IMPORT_BRIDGE_TYPES(Compat)

    MitsubaShape(mitsuba::ref<Mesh> mesh);

    // inet::ShapeBase
    inet::Coord computeBoundingBoxSize() const override;

    bool computeIntersection(
        const inet::LineSegment& lineSegment,
        inet::Coord& intersection1,
        inet::Coord& intersection2,
        inet::Coord& normal1,
        inet::Coord& normal2
    ) const override;

private:
    inet::Coord size_;
    mitsuba::ref<Mesh> mesh_;
};

SIONNA_EXTERN_CLASS(MitsubaShape)

NAMESPACE_END(sionna)
NAMESPACE_END(artery)
