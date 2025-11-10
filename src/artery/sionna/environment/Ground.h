#pragma once

#include <memory>

#include <mitsuba/core/ray.h>
#include <inet/environment/contract/IGround.h>

#include "artery/sionna/bridge/Scene.h"

namespace artery {

    class UnevenTerrain
        : public inet::physicalenvironment::IGround {

        UnevenTerrain(std::shared_ptr<sionna::Scene> scene);

        // inet::physicalenvironment::IGround implementation
        virtual double getElevation(const inet::Coord& position) const = 0;

        private:
            std::shared_ptr<sionna::Scene> scene_; 
    };

}

