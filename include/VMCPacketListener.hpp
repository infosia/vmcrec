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

#define CGLTF_VRM_v0_0
#include "cgltf.h"

using namespace osc;

struct BonePos {
    float p[3];
    float q[4];
};

class VmcPacketListener : public OscPacketListener {
public:
    VmcPacketListener(std::string& output, uint8_t fps);
    virtual ~VmcPacketListener();
    virtual void ProcessMessage(const osc::ReceivedMessage& m,
        const IpEndpointName& remoteEndpoint);

    void Save();
    void Finish();

private:
    BonePos root;
    std::unordered_map<uint8_t, float> blendshapes;
    std::unordered_map<std::string, uint8_t> blendnames;
    std::unordered_map<cgltf_vrm_humanoid_bone_bone_v0_0, BonePos> bones;
    flatbuffers::FlatBufferBuilder builder;
    std::chrono::steady_clock::time_point lasttime;
    std::chrono::milliseconds interval;
    std::ofstream fout;
    std::string output;
    uint8_t blendCount;

    bool online;
    bool calibrated;
    bool blendshapesChanged;
    float uptime;
};
