#include "VMCPacketListener.hpp"

#define CGLTF_IMPLEMENTATION
#define CGLTF_VRM_v0_0
#define CGLTF_VRM_v0_0_IMPLEMENTATION
#include "cgltf.h"

#define ROOT_BONE_ID 255

using namespace VMC::Marionette;

VmcPacketListener::VmcPacketListener(std::string& output, uint8_t fps)
    : OscPacketListener()
    , output(output)
    , online(true)
    , builder(1024) /* bytes */
    , blendshapes()
    , lasttime(std::chrono::steady_clock::now())
    , uptime(0)
    , calibrated(false)
    , blendshapesChanged(false)
    , interval(std::chrono::milliseconds((1000 / fps)))
{
    fout.open(output.c_str(), std::ios::out | std::ios::app | std::ios::binary);
}

VmcPacketListener::~VmcPacketListener()
{
}

void VmcPacketListener::ProcessMessage(const osc::ReceivedMessage& m,
    const IpEndpointName& remoteEndpoint)
{
    (void)remoteEndpoint; // UNUSED

    const auto now = std::chrono::steady_clock::now();
    const auto delta = std::chrono::duration_cast<std::chrono::milliseconds>(now - lasttime);
    uptime += delta.count() / 1000.0f;
    lasttime = now;

    auto arg = m.ArgumentsBegin();
    const auto address = m.AddressPattern();
    if (!calibrated && std::strcmp(address, "/VMC/Ext/OK") == 0) {

        const auto loaded = (arg++)->AsInt32Unchecked();
        const auto calibrationState = (arg++)->AsInt32Unchecked();
        const auto calibrationMode = (arg++)->AsInt32Unchecked();
        const auto trackingState = (arg++)->AsInt32Unchecked();

        // Start recording only after alibration is done
        if (calibrationState == 3) {
            calibrated = true;

            auto available = Available((uint8_t)loaded, (uint8_t)calibrationState,
                (uint8_t)calibrationMode, (uint8_t)trackingState);

            CommandBuilder command(builder);
            command.add_address(Address::Address_OK);
            command.add_localtime(uptime);
            command.add_available(&available);
            builder.Finish(command.Finish());
            Save();
        }
    } else if (calibrated && std::strcmp(address, "/VMC/Ext/Root/Pos") == 0) {
		arg++; // ignore root bone name

        const auto px = (arg++)->AsFloatUnchecked();
        const auto py = (arg++)->AsFloatUnchecked();
        const auto pz = (arg++)->AsFloatUnchecked();

        const auto qx = (arg++)->AsFloatUnchecked();
        const auto qy = (arg++)->AsFloatUnchecked();
        const auto qz = (arg++)->AsFloatUnchecked();
        const auto qw = (arg++)->AsFloatUnchecked();

        root = { { px, py, pz }, { qx, qy, qz, qw } };
    } else if (calibrated && std::strcmp(address, "/VMC/Ext/Bone/Pos") == 0) {
        std::string name = (arg++)->AsStringUnchecked();

        const auto px = (arg++)->AsFloatUnchecked();
        const auto py = (arg++)->AsFloatUnchecked();
        const auto pz = (arg++)->AsFloatUnchecked();

        const auto qx = (arg++)->AsFloatUnchecked();
        const auto qy = (arg++)->AsFloatUnchecked();
        const auto qz = (arg++)->AsFloatUnchecked();
        const auto qw = (arg++)->AsFloatUnchecked();

        if (!name.empty()) {
            name[0] = (char)std::tolower(name[0]);
            cgltf_vrm_humanoid_bone_bone_v0_0 bone;
            if (select_cgltf_vrm_humanoid_bone_bone_v0_0(name.c_str(), &bone)) {
                BonePos value = { { px, py, pz }, { qx, qy, qz, qw } };
                bones[bone] = value;
            }        
        }
    } else if (calibrated && std::strcmp(address, "/VMC/Ext/Blend/Val") == 0) {
        const auto name = (arg++)->AsStringUnchecked();
        const auto value = (arg++)->AsFloatUnchecked();
        blendshapes[name] = value;
    } else if (calibrated && std::strcmp(address, "/VMC/Ext/Blend/Apply") == 0 && blendshapes.size() > 0) {
        blendshapesChanged = true;
    }

    if (delta > interval) {
        // Bone transform (including root)
        if (bones.size()) {
            std::vector<flatbuffers::Offset<VMC::Marionette::Bone>> fbbones;

            const auto rootp = Vec3(root.p[0], root.p[1], root.p[2]);
            const auto rootq = Vec4(root.q[0], root.q[1], root.q[2], root.q[3]);
            fbbones.push_back(CreateBone(builder, (uint8_t)ROOT_BONE_ID, &rootp, &rootq));

            for (const auto &bone : bones) {
                const auto pos = bone.second;
                const auto p = Vec3(pos.p[0], pos.p[1], pos.p[2]);
                const auto q = Vec4(pos.q[0], pos.q[1], pos.q[2], pos.q[3]);
                fbbones.push_back(CreateBone(builder, (int8_t)bone.first, &p, &q));
            }
            auto fbvec = builder.CreateVector(fbbones);

            CommandBuilder command(builder);
            command.add_address(Address::Address_Bone_Pos);
            command.add_localtime(uptime);
            command.add_bones(fbvec);
            builder.Finish(command.Finish());
            Save();
        }

        // Blendshape
        if (blendshapesChanged) {
            std::vector<flatbuffers::Offset<VMC::Marionette::Value>> values;
            for (auto v : blendshapes) {
                values.push_back(CreateValueDirect(builder, v.first.c_str(), v.second));
            }
            auto fbvec = builder.CreateVector(values);

            CommandBuilder command(builder);
            command.add_address(Address::Address_Bend_Apply);
            command.add_localtime(uptime);
            command.add_values(fbvec);
            builder.Finish(command.Finish());
            Save();

            blendshapes.clear();
            blendshapesChanged = false;
        }
    }
}

void VmcPacketListener::Save()
{
    const uint8_t* buffer = builder.GetBufferPointer();
    const uint32_t size = builder.GetSize();

    if (size > 0) {
        fout.write(reinterpret_cast<const char*>(&size), 4);
        fout.write(reinterpret_cast<const char*>(buffer), size);

        builder.Clear();
    }
}

void VmcPacketListener::Finish()
{
    fout.close();
}
