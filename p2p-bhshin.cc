/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2009 The Boeing Company
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/config-store-module.h"
#include "ns3/point-to-point-module.h" // 170809
#include "ns3/point-to-point-layout-module.h" // 170809
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/internet-apps-module.h"
#include "ns3/internet-module.h"
#include "ns3/netanim-module.h" // netanim
#include "ns3/ipv4-static-routing-helper.h" // routing
//#include "ns3/ipv4-list-routing-helper.h" // list routing (170916)
//#include "ns3/olsr-helper.h" // OLSR routing (170916)

#ifdef NS3_MPI
#include "ns3/mpi-interface.h"
#include <mpi.h>
#endif

#include "my_config.h"
#include "my_node.h"
#include "node_id_map.h"
#include "hello.h"
#include "app_flowreq_pkt.h"
#include "string_tokenizer.h"

#include <iostream>
#include <string>
#include <vector>
#include <fstream> // reading a topology config file

using namespace ns3;
using namespace std;

NS_LOG_COMPONENT_DEFINE ("BhshinP2PExample");

std::map<int, MyNode*> myNodes;
NodeIdMap* nodeIdMap;

static void ReceiveHello (Ptr<Socket> socket)
{
	Time now = Simulator::Now();
	Address from;
	Ptr<Packet> packet = socket->RecvFrom(from);
	InetSocketAddress senderAddr = InetSocketAddress::ConvertFrom(from);

	uint8_t* dataBuffer = MyNode::getPacketDataBuffer(packet);
	std::string dataStr = ((char*)dataBuffer);
	Hello hello;
	hello.parse(dataStr);

	Ptr<Node> node = socket->GetNode();
	MyNode* myNode = myNodes[node->GetId()];
	//int ifIdx = socket->GetBoundNetDevice()->GetIfIndex(); // not used

	// MyNode stat
	uint32_t senderId = nodeIdMap->getNodeId(senderAddr.GetIpv4());
	Flow flow(senderId, (int)senderAddr.GetPort(), NODEID_BROADCAST, MyConfig::instance().getPortByName("HelloPort"), FlowType::UDP);
	PacketInfo pktInfo((long)(now.GetMilliSeconds()), flow, 0, packet->GetSize());
	myNode->handlePacketInfo(senderId, pktInfo);

	// handle message
	myNode->handleHello(node, senderAddr.GetIpv4(), hello);
	delete(dataBuffer);
}

static void ReceiveDelayMeasurement(Ptr<Socket> socket)
{
	Address from;
	Ptr<Packet> packet = socket->RecvFrom(from);
	uint8_t* dataBuffer = MyNode::getPacketDataBuffer(packet);
	std::string dataStr = ((char*)dataBuffer);
	Time now = Simulator::Now();

	// parse message
	DelayMeasurement dm;
	dm.parse(dataStr);

	Ptr<Node> node = socket->GetNode();
	MyNode* myNode = myNodes[node->GetId()];
	InetSocketAddress senderAddr = InetSocketAddress::ConvertFrom(from);
	//int ifIdx = socket->GetBoundNetDevice()->GetIfIndex();
	//Ipv4Address nodeIpv4Addr = MyNode::getNodeIpv4Addr(node, ifIdx);

	// MyNode stat
	uint32_t senderId = nodeIdMap->getNodeId(senderAddr.GetIpv4());
	Flow flow(senderId, (int)senderAddr.GetPort(), node->GetId(), MyConfig::instance().getPortByName("DelayMeasurePort"), FlowType::UDP);
	PacketInfo pktInfo((long)(now.GetMilliSeconds()), flow, 0, packet->GetSize());
	myNode->handlePacketInfo(senderId, pktInfo);

	// handle message
	myNode->handleDelayMeasurement(node, socket, senderAddr.GetIpv4(), 0, dm);
	delete (dataBuffer);
}

static void ReceiveRoutingMessages (Ptr<Socket> socket)
{
	Time now = Simulator::Now();
	Ptr<Node> node = socket->GetNode();
	MyNode* myNode = myNodes[node->GetId()];
	Address from;
	Ptr<Packet> packet = socket->RecvFrom(from);
	InetSocketAddress senderAddr = InetSocketAddress::ConvertFrom(from);
	//int ifIdx = socket->GetBoundNetDevice()->GetIfIndex();
	uint8_t* dataBuffer = MyNode::getPacketDataBuffer(packet);
	std::string dataStr = ((char*)dataBuffer);
	int rtMsgType = MyNode::checkRoutingMessageType(dataStr);

	//NS_LOG_UNCOND("[Node " << node->GetId() << "] received RoutingMsg from "<< senderAddr.GetIpv4() << ":" << senderAddr.GetPort() << " (ifIdx=" << ifIdx << "), t=" << now.GetMilliSeconds());
	//NS_LOG_UNCOND(" - " << dataStr);

	switch(rtMsgType){
	case ROUTE_ARREQ:
		myNode->handleARREQ(dataStr, senderAddr.GetIpv4(), 0);
		break;
	case ROUTE_ARREP:
		myNode->handleARREP(dataStr, senderAddr.GetIpv4(), 0);
		break;
	case ROUTE_ARERR:
		myNode->handleARERR(dataStr, senderAddr.GetIpv4(), 0);
		break;
	case ROUTE_SETUP:
		myNode->handleRouteSetup(dataStr, senderAddr.GetIpv4(), 0);
		break;
	case PATH_PROBE:
		myNode->handlePathProbe(dataStr, senderAddr.GetIpv4(), 0);
		break;
	case ROUTE_SRC_ROUTE_UPDATE:
		myNode->handleSourceRouteUpdate(dataStr, senderAddr.GetIpv4(), 0);
		break;
	case ROUTE_FLOWCHECK_REQUEST:
	case ROUTE_FLOWCHECK_REPLY:
		myNode->handleFlowCheck(dataStr, senderAddr.GetIpv4(), 0);
		break;
	case ROUTE_FLOW_ACCEPT_REQUEST:
		myNode->handleFlowAcceptRequest(dataStr, senderAddr.GetIpv4(), 0);
		break;
	case ROUTE_FLOW_ACCEPT_REPLY:
		myNode->handleFlowAcceptReply(dataStr, senderAddr.GetIpv4(), 0);
		break;
	default:
		break;
	}

	uint32_t senderId = nodeIdMap->getNodeId(senderAddr.GetIpv4());
	Flow flow(senderId, (int)senderAddr.GetPort(), NODEID_BROADCAST, MyConfig::instance().getPortByName("RoutingPort"), FlowType::UDP);
	PacketInfo pktInfo((long)(now.GetMilliSeconds()), flow, 0, packet->GetSize());
	myNode->handlePacketInfo(senderId, pktInfo);

	delete(dataBuffer);
}

static void ReceiveMyPacket (Ptr<Socket> socket)
{
	Time now = Simulator::Now();
	Address from;
	Ptr<Packet> packet = socket->RecvFrom(from);
	uint8_t* dataBuffer = MyNode::getPacketDataBuffer(packet);
	std::string dataStr = ((char*)dataBuffer);
	FlowType::Type type = FlowType::UDP;

	Ptr<Node> node = socket->GetNode();
	MyNode* myNode = myNodes[node->GetId()];
	InetSocketAddress senderAddr = InetSocketAddress::ConvertFrom(from);
	Ptr<MyNS3Packet> myPkt = MyNS3Packet::parse(dataStr);

	myNode->handleMyPacket(myPkt, packet->GetSize(), type, senderAddr.GetIpv4());

	// debug
	//NS_LOG_UNCOND("[Node " << node->GetId() << "] ReceiveMyPacket from " << nodeIdMap->getNodeId(senderAddr.GetIpv4()));

	// MyNode stat
	Flow flow(myPkt->getSrc(), myPkt->getAppSrcPort(), myPkt->getDst(), myPkt->getAppDstPort(), FlowType::UDP);
	PacketInfo pktInfo((long)(now.GetMilliSeconds()), flow, 0, packet->GetSize());
	myNode->handlePacketInfo(nodeIdMap->getNodeId(senderAddr.GetIpv4()), pktInfo);

	delete(dataBuffer);
}

static void writeFlowStat(Ptr<Node> node, Time flowWriteInterval){
	MyNode* myNode = myNodes[node->GetId()];
	myNode->writeFlowLog();
	myNode->getFlowTable()->updateRealTimeBandwidth();
	myNode->getControlFlowTable()->updateRealTimeBandwidth();
	myNode->writeNeighborTable();
	myNode->writeRouteTable();

	// periodically check neighbors
	Simulator::Schedule (flowWriteInterval, &writeFlowStat, node, flowWriteInterval);
}

static void parseNodeIdAndPort(string str, uint32_t* nodeId, int* port){
	vector<string> tokens;
	tokenizeString(str, tokens, ":");
	*nodeId = atoi(tokens[0].c_str());
	*port = atoi(tokens[1].c_str());
}

static vector<AppFlowReqPkt> readFlowSettingsFromFile(string filepath) {
	cout << "[MyFlows] reading flows file: " << filepath << endl;

	ifstream inStream(filepath, ios::in);
	string temp;
	vector<AppFlowReqPkt> appFlowReqPkts;

	while(!inStream.eof()){
		getline(inStream, temp);
		ltrim(temp);
		rtrim(temp);

		// Skip comments
		if(temp.substr(0, 1) == "#" || temp.substr(0, 2) == "//"){
			continue;
		}
		// skip blank lines
		if(temp == "" || temp.size() == 0){
			continue;
		}

		// debug
		cout << temp << endl;

		vector<string> tokens;
		tokenizeString(temp, tokens, "\t");
		if(tokens.size() != 11){
			continue;
		}

		// extract information from the string
		int initNodeId = atoi(tokens[0].c_str());
		uint32_t src;
		int sport = 0;
		uint32_t dst;
		int dport = 0;
		parseNodeIdAndPort(tokens[1], &src, &sport);
		parseNodeIdAndPort(tokens[2], &dst, &dport);
		FlowType::Type type = checkType(tokens[3]);
		int pktSize = atoi(tokens[4].c_str());
		int sendingRate = atoi(tokens[5].c_str());
		Time startTime = Seconds(atof(tokens[6].c_str()));
		Time duration = Seconds(atof(tokens[7].c_str()));
		double bwReq = (pktSize*8)*sendingRate;
		double delayReq = atof(tokens[8].c_str());
		double jitterReq = atof(tokens[9].c_str());
		double lossRateReq = atof(tokens[10].c_str());

		Flow flow(src, sport, dst, dport, type);
		string data(pktSize, 'a');
		Ptr<MyNS3Packet> myPkt = CreateObject<MyNS3Packet>(flow.getSrc(), flow.getSrcPort(), flow.getDst(), flow.getDstPort(), data);
		QoSRequirement qosReq(bwReq, delayReq, jitterReq, lossRateReq);
		FlowRequest flowReq(flow, qosReq, pktSize, sendingRate, startTime, duration);

		AppFlowReqPkt appFlowReqPkt(initNodeId, flowReq, myPkt);
		appFlowReqPkts.push_back(appFlowReqPkt);
	}

	return appFlowReqPkts;
}

/*
 * Creates a point-to-point link between two nodes in a NodeContainer.
 *
 * @param allNodes NodeContainer for making connection
 * @param i the index i of allNodes
 * @param j the index j of allNodes
 * @param ipv4Base the base Ipv4 address for auto assignment (e.g. "10.1.1.0")
 */
static Ipv4InterfaceContainer makePointToPoint(PointToPointHelper pointToPoint, NodeContainer allNodes, uint32_t i, uint32_t j, Ipv4Address ipv4Base)
{
	Ipv4InterfaceContainer p2pInterfaces;

	// Check for error.
	if(i >= allNodes.GetN() || j >= allNodes.GetN()){
		NS_LOG_UNCOND("[makePointToPoint] Error: node ID out of bound: " << i << ", " << j);
		return p2pInterfaces; // return an empty container
	}

	NodeContainer p2pNodes;
	p2pNodes.Add(allNodes.Get(i));
	p2pNodes.Add(allNodes.Get(j));

	NetDeviceContainer p2pDevices;
	p2pDevices = pointToPoint.Install (p2pNodes);
	Ipv4AddressHelper address;
	address.SetBase (ipv4Base, "255.255.255.0"); // e.g. "10.1.2.0"
	p2pInterfaces = address.Assign (p2pDevices);

	myNodes[i]->addIpv4Address(p2pInterfaces.Get(0).first->GetAddress(p2pInterfaces.Get(0).second, 0).GetLocal());
	myNodes[j]->addIpv4Address(p2pInterfaces.Get(1).first->GetAddress(p2pInterfaces.Get(1).second, 0).GetLocal());
	nodeIdMap->addMapping(p2pInterfaces.Get(0).first->GetAddress(p2pInterfaces.Get(0).second, 0).GetLocal(), (uint32_t)i);
	nodeIdMap->addMapping(p2pInterfaces.Get(1).first->GetAddress(p2pInterfaces.Get(1).second, 0).GetLocal(), (uint32_t)j);

	return p2pInterfaces;
}

/*
 * Reads textual configurations of point-to-point network topology.
 * The config file must be like this:
 *
 * 0,1,5Mbps,2ms
 * 1,2,10Mbps,1ms
 * 2,3,5Mbps,10ms
 * ...
 *
 * @param filepath The path of a topology configuration file.
 * @param pointToPoint PointToPointHelper parameter
 * @param allNodes NodeContainer for making connection
 * @return a vector of all created interfaces as Ipv4InterfaceContainer
 */
static std::vector<Ipv4InterfaceContainer> loadTopologyConfig(string filepath, PointToPointHelper pointToPoint, NodeContainer nodes)
{
	std::vector<Ipv4InterfaceContainer> containers;

	std::ifstream inStream(filepath, std::ios::in);
	std::string temp;
	int count = 0;

	while(!inStream.eof()){
		getline(inStream, temp);
		ltrim(temp);
		rtrim(temp);

		std::vector<std::string> tokens;
		tokenizeString(temp, tokens, ",");

		if(tokens.size() >= 4){
			uint32_t i = atoi(tokens[0].c_str());
			uint32_t j = atoi(tokens[1].c_str());
			std::string bandwidth = tokens[2];
			std::string delay = tokens[3];

			std::stringstream ss;
			ss << "10.1." << count++ << ".0";
			Ipv4Address ipv4;
			ipv4.Set(ss.str().c_str());

			Ipv4InterfaceContainer container = makePointToPoint(pointToPoint, nodes, i, j, ipv4);
			containers.push_back(container);
		}
	}

	inStream.close();
	return containers;
}

static uint32_t getMaxNumOfNodes(string topologyConfigFile)
{
	std::ifstream inStream(topologyConfigFile, std::ios::in);
	std::string temp;
	uint32_t maxNodeId = 0;

	while(!inStream.eof()){
		getline(inStream, temp);
		ltrim(temp);
		rtrim(temp);

		std::vector<std::string> tokens;
		tokenizeString(temp, tokens, ",");

		if(tokens.size() >= 4){
			uint32_t i = atoi(tokens[0].c_str());
			uint32_t j = atoi(tokens[1].c_str());

			// Find the maximum nodeID number.
			if(i > maxNodeId){
				maxNodeId = i;
			}
			if(j > maxNodeId){
				maxNodeId = j;
			}
		}
	}

	inStream.close();
	return ++maxNodeId;
}

int main (int argc, char *argv[])
{
	MyConfig::instance().readConfigFromFile("sina.config");

	double totalTime = 39.0;
	uint32_t packetSize = 1000; // bytes
	uint32_t numPackets = 5;
#ifdef NS3_MPI
	bool nullmsg = false; // MPI interface, 171114
#endif
	bool verbose = true;
	uint32_t numOfNodes = 0;
	int scheme = BASELINE; // default: existing work.
	string appsConfigFile("2apps_10pkts"); // default: read "2apps_10pkts.txt".
	string topologyFile("topology.config"); // default: read "topology.config".
	double helloIntervalN = atof(MyConfig::instance().getValue("HelloInterval").c_str()); // seconds
	double dmIntervalN = atof(MyConfig::instance().getValue("DelayMeasurementInterval").c_str()); // seconds
	double flowIntervalN = 1.0;
	double flowLogIntervalN = 1.0; // seconds
	Time helloInterval = Seconds (helloIntervalN);
	Time dmInterval = Seconds (dmIntervalN);
	Time flowInterval = Seconds(flowIntervalN);
	Time flowLogInterval = Seconds (flowLogIntervalN);

	if (verbose)
	{
		LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_INFO);
		LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_INFO);
	}

	CommandLine cmd;
	cmd.AddValue ("packetSize", "size of application packet sent", packetSize);
	cmd.AddValue ("numPackets", "number of packets generated", numPackets);
	cmd.AddValue ("verbose", "turn on all WifiNetDevice log components", verbose);
	cmd.AddValue ("apps", "input file for application flows (by Byoungheon Shin)", appsConfigFile);
	cmd.AddValue ("topology", "input file for network topology (by Byoungheon Shin)", topologyFile);
	cmd.AddValue ("scheme", "for research, which scheme is selected, 0 or 1 (by Byoungheon Shin)", scheme);
#ifdef NS3_MPI
	cmd.AddValue ("nullmsg", "Enable the use of null-message synchronization", nullmsg);
#endif
	cmd.Parse (argc, argv);

#ifdef NS3_MPI
	// Distributed simulation setup; by default use granted time window algorithm.
	if(nullmsg){
		GlobalValue::Bind ("SimulatorImplementationType",
				StringValue ("ns3::NullMessageSimulatorImpl"));
	} else {
		GlobalValue::Bind ("SimulatorImplementationType",
				StringValue ("ns3::DistributedSimulatorImpl"));
	}

	// Enable parallel simulator with the command line arguments
	MpiInterface::Enable (&argc, &argv);

	uint32_t systemId = MpiInterface::GetSystemId ();
	uint32_t systemCount = MpiInterface::GetSize ();

	//debug
	NS_LOG_UNCOND("[main] System ID: " << systemId << " / System Count: " << systemCount);
#endif

	/*
	 * Create nodes and MyNodes (bhshin).
	 */
	NodeContainer allNodes;
	numOfNodes = getMaxNumOfNodes(topologyFile);

#ifdef NS3_MPI
	// MPI system ID allocation
	for(uint32_t i=0; i<systemCount; i++){
		int partialCount = (int)(numOfNodes/systemCount);

		if(i+1 == systemCount){
			partialCount += (int)(numOfNodes % systemCount);
		}

		// debug
		NS_LOG_UNCOND("[main] creating " << partialCount << " nodes for system ID " << i);

		NodeContainer partialNodes;
		partialNodes.Create(partialCount, i);
		allNodes.Add(partialNodes);
	}
#else
	allNodes.Create(numOfNodes);
#endif

	// debug
	NS_LOG_UNCOND("[main] created total " << allNodes.GetN() << " nodes from " << topologyFile);

	for(uint32_t i=0; i<allNodes.GetN(); i++){
		MyNode* myNode = new MyNode(i, allNodes.Get(i), scheme);
		myNodes[i] = myNode;
	}
	nodeIdMap = new NodeIdMap();

	// Routing protocol for all nodes
//	OlsrHelper olsr;
//	Ipv4StaticRoutingHelper staticRouting;
//	Ipv4ListRoutingHelper list;
//	list.Add(staticRouting, 0);
//	list.Add(olsr, 10);


	// Every node needs the Internet stack only once.
	InternetStackHelper internet;
	//internet.SetRoutingHelper(list);
	internet.Install (allNodes);

	// Common p-t-p connection settings
	PointToPointHelper pointToPoint;
	pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
	pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));

	/*
	 * Make point-to-point connections.
	 */
	vector<Ipv4InterfaceContainer> p2pIfcs = loadTopologyConfig(topologyFile, pointToPoint, allNodes);

	/*
	 * Common sockets for all nodes
	 */
	TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
	for(uint32_t i=0; i<allNodes.GetN(); i++){
		// socket for receiving Hello
		Ptr<Socket> helloRecvSocket = Socket::CreateSocket(allNodes.Get(i), tid);
		InetSocketAddress recvHelloAddr = InetSocketAddress (Ipv4Address::GetAny(), MyConfig::instance().getPortByName("HelloPort"));
		helloRecvSocket->Bind(recvHelloAddr);
		helloRecvSocket->SetRecvCallback(MakeCallback (&ReceiveHello));

		// socket for receiving DM
		Ptr<Socket> dmRecvSocket = Socket::CreateSocket(allNodes.Get(i), tid);
		InetSocketAddress dmAddr = InetSocketAddress (Ipv4Address::GetAny(), MyConfig::instance().getPortByName("DelayMeasurePort"));
		dmRecvSocket->Bind(dmAddr);
		dmRecvSocket->SetRecvCallback(MakeCallback (&ReceiveDelayMeasurement));

		// socket for receiving routing messages
		Ptr<Socket> rtSocket = Socket::CreateSocket(allNodes.Get(i), tid);
		InetSocketAddress rtAddr = InetSocketAddress (Ipv4Address::GetAny(), MyConfig::instance().getPortByName("RoutingPort"));
		rtSocket->Bind(rtAddr);
		rtSocket->SetRecvCallback(MakeCallback (&ReceiveRoutingMessages));

		// socket for receiving MyPackets
		Ptr<Socket> mpSocket = Socket::CreateSocket(allNodes.Get(i), tid);
		InetSocketAddress mpAddr = InetSocketAddress (Ipv4Address::GetAny(), MyConfig::instance().getPortByName("MyDataPort"));
		mpSocket->Bind(mpAddr);
		mpSocket->SetRecvCallback(MakeCallback (&ReceiveMyPacket));
	}

	for(uint32_t i=0; i<allNodes.GetN(); i++){
		// scheduling for broadcasting Hello
		Simulator::ScheduleWithContext (
				allNodes.Get(i)->GetId(),
				Seconds(0.1),
				&MyNode::checkBroadcastHello,
				myNodes[allNodes.Get(i)->GetId()],
				helloInterval);

		// scheduling for unicasting DM
		Simulator::ScheduleWithContext (
				allNodes.Get(i)->GetId(),
				Seconds(2.0),
				&MyNode::checkUnicastDelayMeasurement,
				myNodes[allNodes.Get(i)->GetId()],
				dmInterval);

		// flow statistics
		Simulator::ScheduleWithContext (
				allNodes.Get(i)->GetId(),
				Seconds(1.0),
				&writeFlowStat,
				allNodes.Get(i),
				flowLogInterval);

		// flow verification (QoS check)
		Simulator::ScheduleWithContext (
				allNodes.Get(i)->GetId(),
				Seconds(2.0),
				&MyNode::checkFlowQoS,
				myNodes[allNodes.Get(i)->GetId()],
				flowInterval);

		Simulator::ScheduleWithContext(
				allNodes.Get(i)->GetId(),
				Seconds(2.0),
				&MyNode::checkFlowRandomness,
				myNodes[allNodes.Get(i)->GetId()],
				flowInterval);
	}

	/*
	 * Application flows Configuration from a file
	 */
	Ptr<UniformRandomVariable> x = CreateObject<UniformRandomVariable> ();
	uint32_t jitter = x->GetInteger(1, 10000);

	stringstream ss;
	vector<AppFlowReqPkt> appFlowReqPkts = readFlowSettingsFromFile(MyConfig::instance().getValue("AppsConfigDirectory") + "/" + appsConfigFile + ".txt");
	for(AppFlowReqPkt appFlowReqPkt : appFlowReqPkts){
		// debug
		cout << "*AppFlowReqPkt\n  -nodeId = " << appFlowReqPkt.getInitNodeId() << endl;
		cout << "  -flowReq.flow = " << appFlowReqPkt.getFlowReq().getFlow().toString() << endl;
		cout << "  -flowReq.startTime = " << appFlowReqPkt.getFlowReq().getStartTime().GetSeconds() << endl;

		int nodeId = appFlowReqPkt.getInitNodeId();
		jitter = x->GetInteger(100, 100000);
		Simulator::ScheduleWithContext (nodeId,
				MicroSeconds(appFlowReqPkt.getFlowReq().getStartTime().GetMicroSeconds() + jitter), &MyNode::doRouting,
				myNodes[nodeId], appFlowReqPkt.getMyPkt(), appFlowReqPkt.getFlowReq());
	}

	/*
	UdpEchoServerHelper echoServer (9);
	ApplicationContainer serverApps = echoServer.Install (allNodes);
	serverApps.Start (Seconds (1.0));
	serverApps.Stop (Seconds (10.0));

	UdpEchoClientHelper echoClient (p2pIfcs[0].GetAddress(0), 9);
	echoClient.SetAttribute ("MaxPackets", UintegerValue (10));
	echoClient.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
	echoClient.SetAttribute ("PacketSize", UintegerValue (1024));

	ApplicationContainer clientApps = echoClient.Install (allNodes.Get(3));
	clientApps.Start (Seconds (3.0));
	clientApps.Stop (Seconds (10.0));
	*/

	/*
	 * Net-Anim settings
	 */
	//AnimationInterface anim("bhshin-animation.xml");
	//anim.SetMobilityPollInterval(Seconds(0.1));
	//anim.EnableIpv4RouteTracking ("bhshin-rt.xml", Seconds (0), Seconds (5), Seconds (0.25));

	/*
	 * Simulator start
	 */
	Simulator::Stop(Seconds(totalTime));
	Simulator::Run ();
	Simulator::Destroy ();

	/*
	 * bhshin, delete dynamic memory allocations
	 */
	for(uint32_t i=0; i<allNodes.GetN(); i++){
		delete myNodes[i];
	}
	delete nodeIdMap;

	return 0;
}
