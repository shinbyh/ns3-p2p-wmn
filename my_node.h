/*
 * my_node.h
 *
 *  Created on: Dec 13, 2016
 *      Author: bhshin
 */

#ifndef SCRATCH_BHSHIN_MY_NODE_H_
#define SCRATCH_BHSHIN_MY_NODE_H_

#include "ns3/core-module.h"
#include "ns3/applications-module.h"
#include "ns3/internet-apps-module.h"
#include "ns3/internet-module.h"

#include <fstream>
#include <vector>

#include "hello.h"
#include "delay_measurement.h"
#include "neighbor_entry.h"
#include "neighbor_table.h"
#include "flow_table.h"
#include "ctrl_flowtable.h"
#include "route_table.h"
#include "route_arreqtable.h"
#include "route_arreptable.h"
#include "my_ns3_packet.h"
#include "flow_request.h"
#include "route_arreq.h"
#include "route_arrep.h"
#include "route_arerr.h"
#include "route_pathprobe.h"
#include "route_rtsetup.h"
#include "route_pathprobe.h"
#include "route_src_rt_update.h"


using namespace std;

class MyNode {
private:
	uint32_t nodeId;
	ns3::Ptr<ns3::Node> node;
	vector<ns3::Ipv4Address> ipv4List;
	NeighborTable* ncTable;
	FlowTable* flowTable;
	ControlFlowTable* ctrlFlowTable;
	RouteTable* routeTable;
	ARREPRecvTable* arrepRecvTable;
	ARREQSentTable* arreqSentTable;
	map<Flow, vector<uint32_t>> srcRtMap; // source route map for source nodes

	ofstream nodeOut;
	ofstream flowOut;
	ofstream flowInfoOut;
	ofstream ctrlOut;
	ofstream ctrlInfoOut;

	int helloSeqNo;
	int scheme;
	int numOfFlows;
	ns3::Ptr<ns3::UniformRandomVariable> rngDelay; // delay random number generator
	ns3::Ptr<ns3::UniformRandomVariable> rngMyPkt; // mypacket random number generator
	ns3::Ptr<ns3::UniformRandomVariable> rngRtPkt; // routingPkt random number generator

	void sendPacketWithJitter(ns3::Time jitter, ns3::Ptr<ns3::Socket> socket, ns3::Ptr<ns3::Packet> pkt);
	ns3::Time getDelayDifference(ns3::Ipv4Address ipAddr, ns3::Time current);
	double getMyAvailableBandwidth(NeighborEntry* entry);
	void accumulateLinkQuality(LinkQuality* lq, NeighborEntry* entry);
	bool isControlPort(int port);

public:
	MyNode(uint32_t nodeId, ns3::Ptr<ns3::Node> node, int scheme);
	virtual ~MyNode();
	static void sendPacket(MyNode* myNode, ns3::Ptr<ns3::Socket> socket, ns3::Ptr<ns3::Packet> pkt);
	void _sendPacket(ns3::Ptr<ns3::Socket> socket, ns3::Ptr<ns3::Packet> pkt);
	void sendRoutingPacket(ns3::Ipv4Address target, string rtMsg);

	const int getScheme() const;
	void addIpv4Address(ns3::Ipv4Address addr);
	vector<ns3::Ipv4Address> getIpv4List();
	NeighborTable* getNcTable();
	FlowTable* getFlowTable();
	ControlFlowTable* getControlFlowTable();
	void writeFlowLog();
	void writeNeighborTable();
	void writeMyBWStat();
	void writeRouteTable();

	// periodically called functions
	static void checkFlowQoS(MyNode* myNode, ns3::Time interval);
	void _checkFlowQoS(ns3::Time interval);
	static void checkUnicastDelayMeasurement(MyNode* myNode, ns3::Time interval);
	void _checkUnicastDelayMeasurement(ns3::Time interval);
	static void checkBroadcastHello(MyNode* myNode, ns3::Time interval);
	void _checkBroadcastHello(ns3::Time interval);
	Hello* generateHello();
	void sendDelayMeasurement(ns3::Ipv4Address target, DelayMeasurement dm, int pktSize);
	static void setupRoute(MyNode* myNode, Flow flow);
	void _setupRoute(Flow flow);
	static void schedulePacketsFromFlowRequest(MyNode* myNode, FlowRequest flowReq, string msg);
	void _schedulePacketsFromFlowRequest(FlowRequest flowReq, string msg);
	void sendMyPacket(ns3::Ipv4Address target, ns3::Ptr<MyNS3Packet> myPkt, FlowType::Type type, int pktSize);

	// functions for routing
	void broadcastARREQ(Flow flow, int seqNo, FlowRequest flowReq);
	ARREP getOptimalARREP(vector<ARREP> arrepList);

	// packet handling functions
	static void doRouting(MyNode* myNode, ns3::Ptr<MyNS3Packet> myPkt, FlowRequest flowReq);
	void _doRouting(ns3::Ptr<MyNS3Packet> myPkt, FlowRequest flowReq);
	void handlePacketInfo(PacketInfo pktInfo);
	void handleHello(ns3::Ptr<ns3::Node> node, ns3::Ipv4Address ipAddr, Hello hello);
	void handleDelayMeasurement(ns3::Ptr<ns3::Node> node, ns3::Ptr<ns3::Socket> socket, ns3::Ipv4Address ipAddr, int ifIdx, DelayMeasurement dm);
	void handleARREQ(string str, ns3::Ipv4Address clientIP, int ifIdx);
	void handleARREP(string str, ns3::Ipv4Address clientIP, int ifIdx);
	void handleARERR(string str, ns3::Ipv4Address clientIP, int ifIdx);
	void handleRouteSetup(string str, ns3::Ipv4Address clientIP, int ifIdx);
	void handlePathProbe(string str, ns3::Ipv4Address clientIP, int ifIdx);
	void handleSourceRouteUpdate(string str, ns3::Ipv4Address clientIP, int ifIdx);
	void handleMyPacket(ns3::Ptr<MyNS3Packet> myPkt, int pktSize, FlowType::Type type, ns3::Ipv4Address ipAddr);
	const bool checkDstOfMyPacket(const MyNS3Packet* myPkt) const;
	int getAndIncrementHelloSeqNo();
	const double getAvgResidualBW();

	// static utility functions
	static const bool isMyAddress(ns3::Ptr<ns3::Node> node, const ns3::Ipv4Address addr);
	static ns3::Ptr<ns3::Packet> createPacket(string data, int pktSize);
	static uint8_t* getPacketDataBuffer(ns3::Ptr<ns3::Packet> packet);
	static const int checkRoutingMessageType(const string msg);
};

#endif /* SCRATCH_BHSHIN_MY_NODE_H_ */
