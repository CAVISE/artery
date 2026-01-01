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

    inline static Defaulted<Float64> DEFAULT_THICKNESS = {SionnaRtConstantsTag::name(), "DEFAULT_THICKNESS"};
    inline static Defaulted<Int32> INTERSECTION_NONE = {SionnaRtConstantsTag::name(), "NONE", IntersectionTypesTag::name()};
    inline static Defaulted<Int32> INTERSECTION_SPECULAR = {SionnaRtConstantsTag::name(), "SPECULAR", IntersectionTypesTag::name()};
    inline static Defaulted<Int32> INTERSECTION_DIFFUSE = {SionnaRtConstantsTag::name(), "DIFFUSE", IntersectionTypesTag::name()};
    inline static Defaulted<Int32> INTERSECTION_REFRACTION = {SionnaRtConstantsTag::name(), "REFRACTION", IntersectionTypesTag::name()};
    inline static Defaulted<Int32> INTERSECTION_DIFFRACTION = {SionnaRtConstantsTag::name(), "DIFFRACTION", IntersectionTypesTag::name()};
};

NAMESPACE_END(py)

NAMESPACE_END(sionna)
NAMESPACE_END(artery)
