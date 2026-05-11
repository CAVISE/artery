#pragma once

#include <cavise/application/CosimService.h>

#include <memory>

namespace cavise {

    class IFloodingStrategy {
    public:
        virtual ~IFloodingStrategy() = default;

        virtual bool shouldDrop() = 0;
    };

    class Flooding : public CosimService {
    public:
        void initialize() override;

    protected:
        bool process(capi::Entity& message) override;

    private:
        std::unique_ptr<IFloodingStrategy> strategy_;
    };

} // namespace cavise
