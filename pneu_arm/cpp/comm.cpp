#include "comm.h"

Comm::Comm()
{
    ctrlMachine = {};
    target = {};
    ctrlNode = {};
    bridgeNetwork = {};

    // target <-------> controller
    udpSockCtrlSend = socket(AF_INET, SOCK_DGRAM, 0);
    udpSockCtrlRecv = socket(AF_INET, SOCK_DGRAM, 0);
    if (udpSockCtrlSend == -1 || udpSockCtrlRecv == -1)
    {
        std::cerr << "Error creating UDP socket\n";
        return;
    }

    ctrlMachine.sin_family = AF_INET;
    ctrlMachine.sin_addr.s_addr = inet_addr(CTRL_MACHINE_IP);
    ctrlMachine.sin_port = htons(CTRL_MACHINE_PORT);

    target.sin_family = AF_INET;
    target.sin_addr.s_addr = inet_addr(TARGET_IP);
    target.sin_port = htons(TARGET_PORT);

#ifdef RUN_REAL
    if (bind(udpSockCtrlRecv, (const struct sockaddr *)&ctrlMachine, sizeof(ctrlMachine)) < 0)
    {
        std::cerr << "Error binding UDP socket\n";
        close(udpSockCtrlRecv);
        return;
    }
#endif

    // controller <-------> ROS bridge
    udpSockBridgeSend = socket(AF_INET, SOCK_DGRAM, 0);
    udpSockBridgeRecv = socket(AF_INET, SOCK_DGRAM, 0);

    if (udpSockBridgeSend == -1 || udpSockBridgeRecv == -1)
    {
        std::cerr << "Error creating UDP socket for bridge communication\n";
        return;
    }

    ctrlNode.sin_family = AF_INET;
    ctrlNode.sin_addr.s_addr = inet_addr(BRIDGE_TO_CTRL_IP);
    ctrlNode.sin_port = htons(BRIDGE_TO_CTRL_PORT);

    bridgeNetwork.sin_family = AF_INET;
    bridgeNetwork.sin_addr.s_addr = inet_addr(CTRL_TO_BRIDGE_IP);
    bridgeNetwork.sin_port = htons(CTRL_TO_BRIDGE_PORT);

    if (bind(udpSockBridgeRecv, (const struct sockaddr *)&ctrlNode, sizeof(ctrlNode)) < 0)
    {
        std::cerr << "Error binding UDP socket for ROS communication\n";
        close(udpSockBridgeRecv);
        return;
    }

    std::cout << "Init Comm done" << "\n";
}

Comm::~Comm()
{
    close(udpSockCtrlSend);
    close(udpSockCtrlRecv);
    close(udpSockBridgeSend);
    close(udpSockBridgeRecv);
}
int Comm::InitComm()
{
    return 1;
    // target <-------> controller
    udpSockCtrlSend = socket(AF_INET, SOCK_DGRAM, 0);
    udpSockCtrlRecv = socket(AF_INET, SOCK_DGRAM, 0);
    if (udpSockCtrlSend == -1 || udpSockCtrlRecv == -1)
    {
        std::cerr << "Error creating UDP socket\n";
        return -1;
    }

    ctrlMachine.sin_family = AF_INET;
    ctrlMachine.sin_addr.s_addr = inet_addr(CTRL_MACHINE_IP);
    ctrlMachine.sin_port = htons(CTRL_MACHINE_PORT);

    target.sin_family = AF_INET;
    target.sin_addr.s_addr = inet_addr(TARGET_IP);
    target.sin_port = htons(TARGET_PORT);

#ifdef RUN_REAL
    if (bind(udpSockCtrlRecv, (const struct sockaddr *)&ctrlMachine, sizeof(ctrlMachine)) < 0)
    {
        std::cerr << "Error binding UDP socket\n";
        close(udpSockCtrlRecv);
        return -1;
    }
#endif

    // controller <-------> ROS bridge
    udpSockBridgeSend = socket(AF_INET, SOCK_DGRAM, 0);
    udpSockBridgeRecv = socket(AF_INET, SOCK_DGRAM, 0);

    if (udpSockBridgeSend == -1 || udpSockBridgeRecv == -1)
    {
        std::cerr << "Error creating UDP socket for bridge communication\n";
        return -1;
    }

    ctrlNode.sin_family = AF_INET;
    ctrlNode.sin_addr.s_addr = inet_addr(BRIDGE_TO_CTRL_IP);
    ctrlNode.sin_port = htons(BRIDGE_TO_CTRL_PORT);

    bridgeNetwork.sin_family = AF_INET;
    bridgeNetwork.sin_addr.s_addr = inet_addr(CTRL_TO_BRIDGE_IP);
    bridgeNetwork.sin_port = htons(CTRL_TO_BRIDGE_PORT);

    if (bind(udpSockBridgeRecv, (const struct sockaddr *)&ctrlNode, sizeof(ctrlNode)) < 0)
    {
        std::cerr << "Error binding UDP socket for ROS communication\n";
        close(udpSockBridgeRecv);
        return -1;
    }

    std::cout << "Init Comm done" << "\n";
    return 1;
}

void Comm::CommGetDataFromTarget(double stateBuffer[])
{
    int dimChange = 0;
#ifdef RUN_REAL
    ssize_t readBytesTarget = recv(udpSockCtrlRecv, targetInputBuffer[dimChange], sizeof(targetInputBuffer), 0);
    do
    {
        dimChange = (dimChange + 1) % 2;
        readBytesTarget = recv(udpSockCtrlRecv, targetInputBuffer[dimChange], sizeof(targetInputBuffer),
                               MSG_DONTWAIT);
    } while (readBytesTarget != -1);
    dimChange = (dimChange + 1) % 2;
#endif
    std::memcpy(stateBuffer, targetInputBuffer[dimChange], TG_IN_CHANNELS * sizeof(double));
}

void Comm::CommSendDataToTarget(double commands[])
{
    sendto(udpSockCtrlSend, commands, TG_OUT_CHANNELS * sizeof(double), 0, (const struct sockaddr *)&target, sizeof(target));
}

void Comm::CommPublishData(double states[])
{
    sendto(udpSockBridgeSend, states, TG_IN_CHANNELS * sizeof(double), 0, (const struct sockaddr *)&bridgeNetwork, sizeof(bridgeNetwork));
}

int Comm::CommRecvCommands(double inputBuffer[])
{
    // read commands from socket
    static int totalBytesRead = 0;
    static int recDataTimer = 0;
    static bool bfOverflowWarning = true;
    static bool receivingData = false;

    int totalSamples = -1;

    ssize_t readBytesROS = recv(udpSockBridgeRecv, (uint8_t *)inputBuffer + totalBytesRead, TRAJSIZE - totalBytesRead, MSG_DONTWAIT);
    if (readBytesROS != -1)
    {
        receivingData = true;
        totalBytesRead = totalBytesRead + (int)readBytesROS;
        recDataTimer = 0;
    }
    // if input buffer from ROS is full
    if (totalBytesRead == (int)TRAJSIZE && bfOverflowWarning)
    {
        std::cout << "Warning: buffer too small for requested input trajectory!" << "\n";
        bfOverflowWarning = false;
    }
    // receiving routine
    if (receivingData)
    {
        recDataTimer++;
        if (recDataTimer == WAIT_TICKS)
        { // start traj. replay
            totalSamples = totalBytesRead / ((int)(sizeof(double) * NUM_OF_DIMS));
            // reset
            totalBytesRead = 0;
            bfOverflowWarning = true;
        }
    }
    return totalSamples;
}
