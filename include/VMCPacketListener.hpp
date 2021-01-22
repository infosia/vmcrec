#pragma once

#include <chrono>
#include <string>
#include <unordered_map>
#include <fstream>

#include "flatbuffers/flatbuffers.h"
#include "ip/UdpSocket.h"
#include "osc/OscPacketListener.h"
#include "osc/OscReceivedElements.h"
#include "VMC.Marionette_generated.h"

using namespace osc;

class VmcPacketListener : public OscPacketListener {
public:
    VmcPacketListener(std::string& output);
    virtual ~VmcPacketListener();
    virtual void ProcessMessage(const osc::ReceivedMessage& m,
        const IpEndpointName& remoteEndpoint);

    void Save();
    void Finish();

private:
    std::ofstream fout;
    std::string output;
    bool online;
    std::unordered_map<std::string, float> blendshapes;
    flatbuffers::FlatBufferBuilder builder;
    std::chrono::steady_clock::time_point lasttime;
    float uptime;
    bool calibrated;
};
