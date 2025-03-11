#include <iostream>
#include <cstring>
#include "RakPeerInterface.h"
#include "RakSleep.h"
#include "NatPunchthroughServer.h"

constexpr int SERVER_PORT = 61666;
constexpr int MAX_CLIENTS = 8096;
constexpr int TIMEOUT_MS = 5000;
constexpr int SLEEP_INTERVAL_MS = 30;

int main() {
    // Initialize RakNet Peer Interface
    RakNet::RakPeerInterface* peer = RakNet::RakPeerInterface::GetInstance();
    if (!peer) {
        std::cerr << "Failed to initialize RakNet peer interface." << std::endl;
        return EXIT_FAILURE;
    }

    // List all available network interfaces
    std::cout << "Available network interfaces:" << std::endl;
    for (int i = 0; i < MAXIMUM_NUMBER_OF_INTERNAL_IDS; i++) {
        RakNet::SystemAddress addr = peer->GetLocalIP(i);
        if (addr != RakNet::UNASSIGNED_SYSTEM_ADDRESS) {
            std::cout << "  " << addr.ToString(true) << std::endl;
        }
    }

    // Bind server to all network interfaces
    RakNet::SocketDescriptor socketDescriptor(SERVER_PORT, "0.0.0.0");
    if (peer->Startup(MAX_CLIENTS, &socketDescriptor, 1) != RakNet::RAKNET_STARTED) {
        std::cerr << "Failed to start RakNet peer on port " << SERVER_PORT << std::endl;
        RakNet::RakPeerInterface::DestroyInstance(peer);
        return EXIT_FAILURE;
    }

    // Set timeout and maximum incoming connections
    peer->SetTimeoutTime(TIMEOUT_MS, RakNet::UNASSIGNED_SYSTEM_ADDRESS);
    peer->SetMaximumIncomingConnections(MAX_CLIENTS);

    std::cout << "NAT Punchthrough Server started on ALL INTERFACES (0.0.0.0:" 
              << SERVER_PORT << ")" << std::endl;

    // Attach NAT Punchthrough Server Plugin
    auto natServer = new RakNet::NatPunchthroughServer;
    peer->AttachPlugin(natServer);

    // Server main loop
    while (true) {
        for (RakNet::Packet* packet = peer->Receive(); packet; peer->DeallocatePacket(packet), packet = peer->Receive()) {
            // Process incoming packets if needed
        }
        RakSleep(SLEEP_INTERVAL_MS);
    }

    // Cleanup (unreachable, but good practice)
    delete natServer;
    RakNet::RakPeerInterface::DestroyInstance(peer);

    return EXIT_SUCCESS;
}
