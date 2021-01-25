
#include "CLI11.hpp"
#include "VMCPacketListener.hpp"
#include "osc/OscOutboundPacketStream.h"

#include <fstream>
#include <thread>

static void printCommand(const VMC::Marionette::Command* command)
{
    // Sample code
    const auto address = command->address();

    if (address == VMC::Marionette::Address_OK) {
        auto state = command->available();

        uint32_t loaded = state->loaded();
        uint32_t calibrationState = state->calibrationState();

        std::cout << "/VMC/Ext/OK " << loaded << " " << calibrationState << std::endl;
    } else if (address == VMC::Marionette::Address_Bend_Apply) {
        std::cout << "/VMC/Ext/Blend/Apply ";
        const auto values = command->values();
        const auto size = values->size();

        for (flatbuffers::uoffset_t i = 0; i < size; i++) {
            auto index = values->Get(i)->index();
            std::cout << (uint32_t)index << " ";
        }

        std::cout << std::endl;
    } else if (address == VMC::Marionette::Address_Bone_Pos) {
        auto bones = command->bones();
        auto boneCount = bones->size();

       std::cout << "/VMC/Ext/Bone/Pos: Total " << boneCount << " bones" << std::endl;
    }
}

int main(int argc, char* argv[])
{
    CLI::App app { "vmcrec: Record & replay VMC motions" };

    std::uint16_t port = 39539;
    app.add_option<std::uint16_t>("-p,--port", port, "port number to bind (39539 by default)");

	std::uint8_t fps = 100;
	app.add_option("-s,--fps", fps, "frame per second");

    std::string output;
    app.add_option("-o,--output", output, "output file name");

    bool replay = false;
    app.add_flag("-r,--replay", replay, "replay recording");

    std::string input;
    app.add_option("-i,--input", input, "input file name (must use with --replay)");

    CLI11_PARSE(app, argc, argv);

    if (replay) {
        if (input.empty()) {
            std::cout << "[ERROR] --input [input filename] is required in order to replay" << std::endl;
            return 1;
        }

        std::ifstream fin(input, std::ios::binary | std::ios::in);

        uint32_t count = 0;
        float lasttime = 0;
        while (true) {
            uint32_t bufferSize = 0;
            fin.read(reinterpret_cast<char*>(&bufferSize), 4);

            if (fin.eof() || bufferSize == 0)
                break;

            count++;

            uint8_t* buffer = (uint8_t*)malloc(bufferSize);
            fin.read(reinterpret_cast<char*>(buffer), bufferSize);
            auto verifier = flatbuffers::Verifier(buffer, bufferSize);
            if (!VMC::Marionette::VerifyCommandBuffer(verifier)) {
                std::cout << "[ERROR] Wrong buffer at " << fin.tellg() << ". exiting." << std::endl;
                free(buffer);
                break;
            }
            const VMC::Marionette::Command* command = VMC::Marionette::GetCommand(buffer);

            const auto localtime = command->localtime();
            const auto dt = static_cast<uint32_t>((localtime - lasttime) * 1000);
            lasttime = localtime;

            printCommand(command);

            free(buffer);
        }

        std::cout << "[INFO] Total: " << count << " records" << std::endl;

    } else {
        if (output.empty()) {
            std::cout << "[ERROR] --output [output filename] is required in order to record motions" << std::endl;
            return 1;
        }

        VmcPacketListener listener(output, fps);
        UdpListeningReceiveSocket s(
            IpEndpointName(IpEndpointName::ANY_ADDRESS, port),
            &listener);

        std::cout << "[INFO] Listening for input on port " << port << "..." << std::endl;
        std::cout << "[INFO] Type Ctrl-C to end recording and generate dump file" << std::endl;

        s.RunUntilSigInt();

        std::cout << "[INFO] Generating " << output << "..." << std::endl;

        listener.Finish();
    }

    return 0;
}