//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

/**
 * @file ArteryManager.h
 *
 * @brief Declaration of ArteryManager module.
 */
#ifndef ARTERYMANAGERSERVICE_H_
#define ARTERYMANAGERSERVICE_H_

#include "artery/application/ItsG5Service.h"
#include "artery/application/NetworkInterface.h"

#include "OpenCDA_message_structure.pb.h"
#include "Artery_message_structure.pb.h"

namespace artery
{

/**
 * @brief Logs a message to a file with timestamp.
 * @param filename Path to the log file.
 * @param message Message to be logged.
 */
void logToFile(const std::string& filename, const std::string& message);
/**
 * @brief ArteryManager module for handling incoming data and triggering data requests.
 */
class ArteryManager : public ItsG5Service
{
    public:
        ArteryManager();
        ~ArteryManager();
        
         /**
        * @brief Indicates incoming data and processes it.
        * @param ind Data indication received.
        * @param packet Received packet.
        * @param net Network interface.
        */
        void indicate(const vanetza::btp::DataIndication&, omnetpp::cPacket*, const artery::NetworkInterface&) override;

        void trigger() override;
        /**
        * @brief Receives signals from other modules.
        * @param source Source of the signal.
        * @param signal Signal received.
        * @param obj1 Object associated with the signal (unused).
        * @param obj2 Object associated with the signal (unused).
        */
        void receiveSignal(omnetpp::cComponent*, omnetpp::simsignal_t, omnetpp::cObject*, omnetpp::cObject*) override;
    
    protected:
        void initialize() override;
        void finish() override;
        void handleMessage(omnetpp::cMessage*) override;

    private:
        omnetpp::cMessage* m_self_msg; 
};

} // namespace artery

#endif /* ARTERYMANAGERSERVICE_H_ */
