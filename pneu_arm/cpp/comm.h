#ifndef COMM_H
#define COMM_H

#define RUN_REAL

#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>

// controller
#define CTRL_MACHINE_IP "192.168.2.2"
#define CTRL_MACHINE_PORT 8001
// target
#define TARGET_IP "192.168.2.1"
#define TARGET_PORT 26000
// ROS bridge to controller commanded trajectories
#define BRIDGE_TO_CTRL_IP "127.0.0.1"
#define BRIDGE_TO_CTRL_PORT 26300
// controller to ROS data stream
#define CTRL_TO_BRIDGE_IP "127.0.0.1"
#define CTRL_TO_BRIDGE_PORT 26400

//--- IO DATA
// measurement from target
#define TG_IN_CHANNELS 9 // number of channels
// command signals
#define TG_OUT_CHANNELS 4

// IO commands from ROS network
#define NUM_OF_DIMS 3     // hard coded!
#define NUM_OF_SECONDS 80 // hard coded!
#define SAMPLING_RATE 500 // hard coded!
#define NUM_OF_SAMPLES SAMPLING_RATE *NUM_OF_SECONDS
// main buffer for commanded signals
#define TRAJSIZE (NUM_OF_SAMPLES * NUM_OF_DIMS * sizeof(double))

// start traj. replay after WAIT_TIMER
#define WAIT_TIMER 0.1
#define WAIT_TICKS (int)(WAIT_TIMER * SAMPLING_RATE)

static int udpSockCtrlSend;
static int udpSockCtrlRecv;
static int udpSockBridgeSend;
static int udpSockBridgeRecv;
static sockaddr_in ctrlMachine, target, ctrlNode, bridgeNetwork;

class Comm
{
public:
    Comm();
    ~Comm();
    int InitComm();
    void CommGetDataFromTarget(double stateBuffer[]);
    void CommSendDataToTarget(double commands[]);
    int CommRecvCommands(double inputBuffer[]);
    void CommPublishData(double states[]);

    double targetInputBuffer[2][TG_IN_CHANNELS]; // main buffer to receive data from target computer
};

#endif // COMM_H
