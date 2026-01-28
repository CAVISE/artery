#include "Compat.h"

#include <drjit/array.h>
#include <drjit/array_traits.h>

NAMESPACE_BEGIN(artery)
NAMESPACE_BEGIN(sionna)
NAMESPACE_BEGIN(compat)

MI_VARIANT
inet::Coord Compat<Float, Spectrum>::bboxExtentsToCoord(const BoundingBox3f& bbox) {
    if (!drjit::all_nested(bbox.valid())) {
        return inet::Coord::NIL;
    }

    const auto size = bbox.extents();
    return inet::Coord(
        toScalar(size.x()),
        toScalar(size.y()),
        toScalar(size.z())
    );
}

SIONNA_INSTANTIATE_CLASS(Compat)

NAMESPACE_END(compat)
NAMESPACE_END(sionna)
NAMESPACE_END(artery)
