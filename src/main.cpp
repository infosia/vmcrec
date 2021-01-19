
#include "CLI11.hpp"
#include "VMCPacketListener.hpp"

int main(int argc, char* argv[])
{
    CLI::App app { "vmcrec: Record & replay VMC motions" };

    std::uint16_t port = 39539;
    app.add_option<std::uint16_t>("-p,--port", port, "port number to bind (39539 by default)");

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


    } else {
        if (output.empty()) {
            std::cout << "[ERROR] --output [output filename] is required in order to record motions" << std::endl;
            return 1;        
        }
        VmcPacketListener listener(output);
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