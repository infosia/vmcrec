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
    if (std::strcmp(address, "/VMC/Ext/OK") == 0) {

        auto available = Available((int8_t)(arg++)->AsInt32Unchecked(), (int8_t)(arg++)->AsInt32Unchecked(),
            (int8_t)(arg++)->AsInt32Unchecked(), (int8_t)(arg++)->AsInt32Unchecked());

        CommandBuilder command(builder);
        command.add_address(Address::Address_OK);
        command.add_localtime(uptime);
        command.add_available(&available);
        builder.Finish(command.Finish());
        Save();
    } else if (std::strcmp(address, "/VMC/Ext/Root/Pos") == 0) {
        const auto name = (arg++)->AsStringUnchecked();

        auto p = Vec3((arg++)->AsFloatUnchecked(), (arg++)->AsFloatUnchecked(), (arg++)->AsFloatUnchecked());
        auto q = Vec4((arg++)->AsFloatUnchecked(), (arg++)->AsFloatUnchecked(), (arg++)->AsFloatUnchecked(), (arg++)->AsFloatUnchecked());
        auto s = Vec3((arg++)->AsFloatUnchecked(), (arg++)->AsFloatUnchecked(), (arg++)->AsFloatUnchecked());
        auto o = Vec3((arg++)->AsFloatUnchecked(), (arg++)->AsFloatUnchecked(), (arg++)->AsFloatUnchecked());

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
    } else if (std::strcmp(address, "/VMC/Ext/Bone/Pos") == 0) {
        auto p = Vec3((arg++)->AsFloatUnchecked(), (arg++)->AsFloatUnchecked(), (arg++)->AsFloatUnchecked());
        auto q = Vec4((arg++)->AsFloatUnchecked(), (arg++)->AsFloatUnchecked(), (arg++)->AsFloatUnchecked(), (arg++)->AsFloatUnchecked());
        CommandBuilder command(builder);
        command.add_address(Address::Address_Bone_Pos);
        command.add_localtime(uptime);
        command.add_p(&p);
        command.add_q(&q);
        builder.Finish(command.Finish());
        Save();
    } else if (std::strcmp(address, "/VMC/Ext/Blend/Val") == 0) {
        blendshapes.emplace((arg++)->AsStringUnchecked(), (arg++)->AsFloatUnchecked());
    } else if (std::strcmp(address, "/VMC/Ext/Blend/Apply") == 0 && blendshapes.size() > 0) {

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
