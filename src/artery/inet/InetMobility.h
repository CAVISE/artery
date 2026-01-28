#ifndef ARTERY_INETMOBILITY_H_SKZPGILS
#define ARTERY_INETMOBILITY_H_SKZPGILS

#include "artery/traci/MobilityBase.h"
#include "artery/traci/PersonMobility.h"
#include "artery/traci/VehicleMobility.h"
<<<<<<< current

#include <inet/mobility/contract/IMobility.h>
#include <omnetpp/csimplemodule.h>

namespace inet
{
class CanvasProjection;
}
=======
#include <inet/mobility/contract/IMobility.h>
#include <omnetpp/csimplemodule.h>

namespace inet { class CanvasProjection; }
>>>>>>> incoming

namespace artery
{

class InetMobility : public inet::IMobility, public virtual MobilityBase, public omnetpp::cSimpleModule
{
public:
    // inet::IMobility interface
    double getMaxSpeed() const override;
    inet::Coord getCurrentPosition() override;
    inet::Coord getCurrentSpeed() override;
    inet::EulerAngles getCurrentAngularPosition() override;
    inet::EulerAngles getCurrentAngularSpeed() override;
    inet::Coord getConstraintAreaMax() const override;
    inet::Coord getConstraintAreaMin() const override;

    // omnetpp::cSimpleModule
    void initialize(int stage) override;
    int numInitStages() const override;

protected:
    virtual void updateVisualRepresentation();

    void initialize(const Position& pos, Angle heading, double speed) override;
    void update(const Position& pos, Angle heading, double speed) override;

private:
    inet::Coord mPosition;
    inet::Coord mSpeed;
    inet::EulerAngles mOrientation;
    double mAntennaHeight = 0.0;
    omnetpp::cModule* mVisualRepresentation = nullptr;
    const inet::CanvasProjection* mCanvasProjection = nullptr;
};

class InetVehicleMobility : public InetMobility, public VehicleMobility
{
public:
    void initialize(int stage) override;
<<<<<<< current
    const std::string& getId() override { return mVehicleId; };
=======
    const std::string& getId() override  { return mVehicleId; };
>>>>>>> incoming
};

class InetPersonMobility : public InetMobility, public PersonMobility
{
public:
    void initialize(int stage) override;
    const std::string& getId() override { return mPersonId; };
};

<<<<<<< current
}  // namespace artery
=======
} // namespace artery
>>>>>>> incoming

#endif /* ARTERY_INETMOBILITY_H_SKZPGILS */
