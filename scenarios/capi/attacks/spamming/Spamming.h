#pragma once

#include <cavise/application/CosimService.h>

#include <deque>
#include <memory>

namespace cavise {

    class Spamming : public CosimService {
    public:
        void initialize() override;
        void trigger() override;

    protected:
        bool process(capi::Entity& message) override;
        std::unique_ptr<capi::Entity> make(std::shared_ptr<artery::NetworkInterface>& iface) override;

    private:
        std::deque<capi::Entity> payloads_;
        int burstCount_ = 1;
        bool replayingOldPayload_ = false;
    };

} // namespace cavise
