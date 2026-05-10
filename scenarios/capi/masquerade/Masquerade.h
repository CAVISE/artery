#pragma once

#include <cavise/application/CosimService.h>

#include <cstdint>
#include <memory>
#include <optional>
#include <string>

namespace cavise {

    class IMasqueradeStrategy {
    public:
        virtual ~IMasqueradeStrategy() = default;

        virtual void newId(const std::string& newID) = 0;
        virtual std::optional<std::string> takeNewID() = 0;
    };

    class Masquerade : public CosimService {
    public:
        void initialize() override;

    protected:
        bool process(capi::Entity& message) override;
        std::unique_ptr<capi::Entity> make(std::shared_ptr<artery::NetworkInterface>& iface) override;

    private:
        std::unique_ptr<IMasqueradeStrategy> strategy_;
    };

} // namespace cavise
