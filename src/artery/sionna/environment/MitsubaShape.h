#pragma once

#include <artery/sionna/bridge/Fwd.h>
#include <artery/sionna/bridge/Compat.h>

#include <mitsuba/core/ray.h>
#include <mitsuba/core/bbox.h>
#include <mitsuba/render/mesh.h>
#include <mitsuba/render/scene.h>

#include <inet/common/geometry/base/ShapeBase.h>

#include <drjit/array.h>

namespace artery {

    namespace sionna {

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
            SIONNA_BRIDGE_IMPORT_BRIDGE_TYPES()

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

    }
}

SIONNA_BRIDGE_EXTERN_CLASS(artery::sionna::MitsubaShape)
