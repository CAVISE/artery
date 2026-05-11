#pragma once

#include <cavise/application/CosimService.h>

#include <memory>
#include <optional>

namespace cavise {

    class IReplayStrategy {
    public:
        virtual ~IReplayStrategy() = default;

        virtual void save(const capi::Entity& message) = 0;
        virtual std::optional<capi::Entity> replay() = 0;
    };

    class Replay : public CosimService {
    public:
        void initialize() override;

    protected:
        bool process(capi::Entity& message) override;
        std::unique_ptr<capi::Entity> make(std::shared_ptr<artery::NetworkInterface>& iface) override;

    private:
        std::unique_ptr<IReplayStrategy> strategy_;
    };

} // namespace cavise
