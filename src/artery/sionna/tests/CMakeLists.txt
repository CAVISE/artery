# Compatibility test for current Mitsuba distribution. If this compiles and runs,
# that means that nanobind object registry was shared and Sionna bridge module can operate.

nanobind_add_module(mitsuba_ext
    NB_DOMAIN ${MI_NB_DOMAIN}
    NB_SHARED
    ${MI_DIST_FLAGS}
    main.cpp
    ${CORE_PY_SRC}
    ${RENDER_PY_SRC}
)