#pragma once

#include <memory>

#include "artery/sionna/bridge/Fwd.h"

namespace artery {
    namespace sionna {

        MI_VARIANT class SionnaScene 
            : std::enable_shared_from_this<SionnaScene<Float, Spectrum>> {
        public:
            SIONNA_IMPORT_RENDER_TYPES(Scene)

            static std::shared_ptr<SionnaScene> instance();

            Scene& scene();
        };

    }  // namespace sionna
}  // namespace artery