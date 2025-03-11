#include <iostream>
#include <cstring>
#include <cerrno>
#include "RakPeerInterface.h"
#include "RakSleep.h"
#include "NatPunchthroughServer.h"
#include "MessageIdentifiers.h"

constexpr int SERVER_PORT = 61666;
constexpr int MAX_CLIENTS = 8096;
constexpr int TIMEOUT_MS = 5000;
constexpr int SLEEP_INTERVAL_MS = 30;

class MyNatDebugInterface : public RakNet::NatPunchthroughServerDebugInterface
{
public:
    void OnServerMessage(const char *msg) override {
        std::cout << "[NAT Debug] " << msg << std::endl;
    }
};

std::string getPacketTypeName(int packetType) {
    switch (packetType) {
        // NAT Punchthrough Plugin messages
        case ID_NAT_PUNCHTHROUGH_REQUEST: return "ID_NAT_PUNCHTHROUGH_REQUEST";
        case ID_NAT_GET_MOST_RECENT_PORT: return "ID_NAT_GET_MOST_RECENT_PORT";
        case ID_NAT_CLIENT_READY: return "ID_NAT_CLIENT_READY";
        case ID_NAT_TARGET_NOT_CONNECTED: return "ID_NAT_TARGET_NOT_CONNECTED";
        case ID_NAT_TARGET_UNRESPONSIVE: return "ID_NAT_TARGET_UNRESPONSIVE";
        case ID_NAT_CONNECTION_TO_TARGET_LOST: return "ID_NAT_CONNECTION_TO_TARGET_LOST";
        case ID_NAT_ALREADY_IN_PROGRESS: return "ID_NAT_ALREADY_IN_PROGRESS";
        case ID_NAT_PUNCHTHROUGH_FAILED: return "ID_NAT_PUNCHTHROUGH_FAILED";
        case ID_NAT_PUNCHTHROUGH_SUCCEEDED: return "ID_NAT_PUNCHTHROUGH_SUCCEEDED";
        case ID_NAT_PING: return "ID_NAT_PING";
        case ID_NAT_PONG: return "ID_NAT_PONG";
        case ID_NAT_REQUEST_BOUND_ADDRESSES: return "ID_NAT_REQUEST_BOUND_ADDRESSES";
        case ID_NAT_RESPOND_BOUND_ADDRESSES: return "ID_NAT_RESPOND_BOUND_ADDRESSES";

        // RakNet default messages
        case ID_CONNECTED_PING: return "ID_CONNECTED_PING";
        case ID_UNCONNECTED_PING: return "ID_UNCONNECTED_PING";
        case ID_CONNECTED_PONG: return "ID_CONNECTED_PONG";
        case ID_DETECT_LOST_CONNECTIONS: return "ID_DETECT_LOST_CONNECTIONS";
        case ID_CONNECTION_REQUEST: return "ID_CONNECTION_REQUEST";
        case ID_NEW_INCOMING_CONNECTION: return "ID_NEW_INCOMING_CONNECTION";
        case ID_CONNECTION_REQUEST_ACCEPTED: return "ID_CONNECTION_REQUEST_ACCEPTED";
        case ID_CONNECTION_ATTEMPT_FAILED: return "ID_CONNECTION_ATTEMPT_FAILED";
        case ID_ALREADY_CONNECTED: return "ID_ALREADY_CONNECTED";
        case ID_NO_FREE_INCOMING_CONNECTIONS: return "ID_NO_FREE_INCOMING_CONNECTIONS";
        case ID_DISCONNECTION_NOTIFICATION: return "ID_DISCONNECTION_NOTIFICATION";
        case ID_CONNECTION_LOST: return "ID_CONNECTION_LOST";
        case ID_INCOMPATIBLE_PROTOCOL_VERSION: return "ID_INCOMPATIBLE_PROTOCOL_VERSION";

        // Out-of-band (OOB) & router messages
        case ID_OUT_OF_BAND_INTERNAL: return "ID_OUT_OF_BAND_INTERNAL";
        case ID_ROUTER_2_FORWARDING_NO_PATH: return "ID_ROUTER_2_FORWARDING_NO_PATH";
        case ID_ROUTER_2_FORWARDING_ESTABLISHED: return "ID_ROUTER_2_FORWARDING_ESTABLISHED";
        case ID_ROUTER_2_REROUTED: return "ID_ROUTER_2_REROUTED";

        // NAT Type Detection messages
        case ID_NAT_TYPE_DETECTION_REQUEST: return "ID_NAT_TYPE_DETECTION_REQUEST";
        case ID_NAT_TYPE_DETECTION_RESULT: return "ID_NAT_TYPE_DETECTION_RESULT";

        // Other important messages
        case ID_TIMESTAMP: return "ID_TIMESTAMP";
        case ID_UNCONNECTED_PONG: return "ID_UNCONNECTED_PONG";
        case ID_ADVERTISE_SYSTEM: return "ID_ADVERTISE_SYSTEM";

        // Unknown packet type
        default: return "UNKNOWN_PACKET_TYPE";
    }
}

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

    // NAT Punchthrough Debug Interface
    auto natDebug = new MyNatDebugInterface;

    // Attach NAT Punchthrough Server Plugin
    auto natServer = new RakNet::NatPunchthroughServer;
    natServer->SetDebugInterface(natDebug);
    peer->AttachPlugin(natServer);

    // Server main loop
    while (true) {
        for (RakNet::Packet* packet = peer->Receive(); packet; peer->DeallocatePacket(packet), packet = peer->Receive()) {
            if (packet->length > 0)
            {
                std::cout << "Received packet type: " << getPacketTypeName((int)packet->data[0])
                          << " (packet length: " << packet->length << ") "
                          << " from " << packet->systemAddress.ToString(true) << std::endl;
            }
            if (errno != 0)
            {
                std::cerr << "Command failed with error: " << strerror(errno) 
                          << " (errno: " << errno << ")"  
                          << " packet length: " << packet->length
                          << std::endl;
                errno = 0;
            }
        }
        RakSleep(SLEEP_INTERVAL_MS);
    }

    // Cleanup (unreachable, but good practice)
    delete natDebug;
    delete natServer;
    RakNet::RakPeerInterface::DestroyInstance(peer);

    return EXIT_SUCCESS;
}
