#pragma once

#include <artery/sionna/bridge/Fwd.h>
#include <inet/common/geometry/common/Coord.h>
#include <mitsuba/core/bbox.h>
#include <mitsuba/core/vector.h>
#include <drjit/array.h>
#include <drjit/array_traits.h>
#include <limits>

/**
 * @file Helpers for converting Mitsuba datatypes into INET and STL friendly
 * representations.
 */

NAMESPACE_BEGIN(artery)
NAMESPACE_BEGIN(sionna)
NAMESPACE_BEGIN(compat)

/**
 * @brief Collection of conversions from Mitsuba types into INET-/STL-friendly
 * counterparts.
 */
MI_VARIANT
struct Compat {
    SIONNA_IMPORT_CORE_TYPES(BoundingBox3f)

    /**
     * @brief Convert a Mitsuba bounding box into an INET coordinate representing
     * its extents (max - min). Returns Coord::NIL for invalid boxes.
     */
    static inet::Coord bboxExtentsToCoord(const BoundingBox3f& bbox);
};

/**
 * @brief Convert Dr.Jit value to a host double, detaching AD and taking lane 0 for JIT packets.
 */
template <typename Value>
double toScalar(const Value& value) {
    if constexpr (drjit::is_diff_v<Value> || drjit::is_jit_v<Value>) {
        return static_cast<double>(drjit::slice(drjit::detach(value), 0));
    } else {
        // Some Value types provide quiet_NaN only as unqualified (ADL), avoid qualified lookup issues.
        return static_cast<double>(value);
    }
}

SIONNA_EXTERN_CLASS(Compat)

NAMESPACE_END(compat)
NAMESPACE_END(sionna)
NAMESPACE_END(artery)
