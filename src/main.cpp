#define CGLTF_IMPLEMENTATION
#define CGLTF_VRM_v0_0
#define CGLTF_VRM_v0_0_IMPLEMENTATION
#include "cgltf.h"

#include "osc/OscReceivedElements.h"
#include "osc/OscPacketListener.h"
#include "ip/UdpSocket.h"
#include "CLI11.hpp"

int main(int argc, char* argv[])
{
	CLI::App app{ "vmcrec: Record & replay VMC motions" };

	std::uint16_t port = 39539;
	app.add_option<std::uint16_t>("-p,--port", port, "port number to bind");

	CLI11_PARSE(app, argc, argv);

	return 0;
}