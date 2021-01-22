#include "VMCPacketListener.hpp"

using namespace VMC::Marionette;

VmcPacketListener::VmcPacketListener(std::string& output)
    : OscPacketListener()
    , output(output)
    , online(true)
    , builder(1024) /* bytes */
    , blendshapes()
    , lasttime(std::chrono::steady_clock::now())
    , uptime(0)
    , calibrated(false)
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
        const auto name = (arg++)->AsStringUnchecked();

        const auto px = (arg++)->AsFloatUnchecked();
        const auto py = (arg++)->AsFloatUnchecked();
        const auto pz = (arg++)->AsFloatUnchecked();

        const auto qx = (arg++)->AsFloatUnchecked();
        const auto qy = (arg++)->AsFloatUnchecked();
        const auto qz = (arg++)->AsFloatUnchecked();
        const auto qw = (arg++)->AsFloatUnchecked();

        const auto sx = (arg++)->AsFloatUnchecked();
        const auto sy = (arg++)->AsFloatUnchecked();
        const auto sz = (arg++)->AsFloatUnchecked();

        const auto ox = (arg++)->AsFloatUnchecked();
        const auto oy = (arg++)->AsFloatUnchecked();
        const auto oz = (arg++)->AsFloatUnchecked();

        const auto p = Vec3(px, py, pz);
        const auto q = Vec4(qx, qy, qz, qw);
        const auto s = Vec3(sx, sy, sz);
        const auto o = Vec3(ox, oy, oz);

        auto fbname = builder.CreateString(name);

        CommandBuilder command(builder);
        command.add_address(Address::Address_Root_Pos);
        command.add_localtime(uptime);
        command.add_p(&p);
        command.add_q(&q);
        command.add_s(&s);
        command.add_o(&o);
        command.add_name(fbname);
        builder.Finish(command.Finish());
        Save();
    } else if (calibrated && std::strcmp(address, "/VMC/Ext/Bone/Pos") == 0) {

        const auto px = (arg++)->AsFloatUnchecked();
        const auto py = (arg++)->AsFloatUnchecked();
        const auto pz = (arg++)->AsFloatUnchecked();

        const auto qx = (arg++)->AsFloatUnchecked();
        const auto qy = (arg++)->AsFloatUnchecked();
        const auto qz = (arg++)->AsFloatUnchecked();
        const auto qw = (arg++)->AsFloatUnchecked();

        const auto p = Vec3(px, py, pz);
        const auto q = Vec4(qx, qy, qz, qw);

        CommandBuilder command(builder);
        command.add_address(Address::Address_Bone_Pos);
        command.add_localtime(uptime);
        command.add_p(&p);
        command.add_q(&q);
        builder.Finish(command.Finish());
        Save();
    } else if (calibrated && std::strcmp(address, "/VMC/Ext/Blend/Val") == 0) {
        const auto name = (arg++)->AsStringUnchecked();
        const auto value = (arg++)->AsFloatUnchecked();
        blendshapes.emplace(name, value);
    } else if (calibrated && std::strcmp(address, "/VMC/Ext/Blend/Apply") == 0 && blendshapes.size() > 0) {

        std::vector<flatbuffers::Offset<Value>> values;
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
