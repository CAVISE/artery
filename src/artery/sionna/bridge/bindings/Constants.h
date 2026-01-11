#pragma once

#include <artery/sionna/bridge/Bindings.h>

NAMESPACE_BEGIN(artery)
NAMESPACE_BEGIN(sionna)

NAMESPACE_BEGIN(py)

PY_IDENTITY_TAG(SionnaRtConstants, sionna.rt.constants);
PY_IDENTITY_TAG(IntersectionTypes, IntersectionTypes);

MI_VARIANT
class Constants
{
public:
    SIONNA_IMPORT_CORE_TYPES(Float64, Int32)

    inline static auto DEFAULT_THICKNESS = makeDefaulted<Float64>(SionnaRtConstantsTag::name(), "DEFAULT_THICKNESS");
    inline static auto INTERSECTION_NONE = makeDefaulted<Int32>(SionnaRtConstantsTag::name(), "NONE", IntersectionTypesTag::name());
    inline static auto INTERSECTION_SPECULAR = makeDefaulted<Int32>(SionnaRtConstantsTag::name(), "SPECULAR", IntersectionTypesTag::name());
    inline static auto INTERSECTION_DIFFUSE = makeDefaulted<Int32>(SionnaRtConstantsTag::name(), "DIFFUSE", IntersectionTypesTag::name());
    inline static auto INTERSECTION_REFRACTION = makeDefaulted<Int32>(SionnaRtConstantsTag::name(), "REFRACTION", IntersectionTypesTag::name());
    inline static auto INTERSECTION_DIFFRACTION = makeDefaulted<Int32>(SionnaRtConstantsTag::name(), "DIFFRACTION", IntersectionTypesTag::name());
};

NAMESPACE_END(py)

NAMESPACE_END(sionna)
NAMESPACE_END(artery)
