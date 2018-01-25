/*
 * my_node.cc
 *
 *  Created on: Dec 13, 2016
 *      Author: bhshin
 */

#include "my_node.h"
#include "my_config.h"
#include "string_tokenizer.h"
#include "ns3/nstime.h"
#include "ns3/core-module.h"
#include <sstream>
#include <boost/foreach.hpp>

#include "delay_measurement.h"
#include "node_id_map.h"

#define DEBUG_PRINT
#define DEBUG_NODE_OUT
#define DEBUG_FLOW_OUT
#define JITTERED_PKT_TX 0

using namespace ns3;

extern NodeIdMap* nodeIdMap;

MyNode::MyNode(uint32_t nodeId, Ptr<Node> node, int scheme)
{
	this->nodeId = nodeId;
	this->node = node;
	this->scheme = scheme;
	this->helloSeqNo = 0;
	this->flowCheckSeqNo = 0;
	this->numOfFlows = 0;

	// tables
	this->ncTable = new NeighborTable(this);
	this->flowTable = new FlowTable();
	this->ctrlFlowTable = new ControlFlowTable();
	this->routeTable = new RouteTable();
	this->arreqSentTable = new ARREQSentTable();
	this->arrepRecvTable = new ARREPRecvTable();

	// file output stream variables
#ifdef DEBUG_NODE_OUT
	stringstream ssn;
	ssn << "nodelog_" << node->GetId() << ".txt";
	this->nodeOut.open(ssn.str().c_str(), ofstream::out);
#endif

#ifdef DEBUG_FLOW_OUT
	stringstream ssf;
	ssf << "flowlog_" << node->GetId() << ".txt";
	this->flowOut.open(ssf.str().c_str(), ofstream::out);

	stringstream ssfi;
	ssfi << "flowinfo_" << node->GetId() << ".txt";
	this->flowInfoOut.open(ssfi.str().c_str(), ofstream::out);
	this->flowInfoOut.close();

	stringstream ssc;
	ssc << "ovlog_" << node->GetId() << ".txt";
	this->ctrlOut.open(ssc.str().c_str(), ofstream::out);

	stringstream ssci;
	ssci << "ovinfo_" << node->GetId() << ".txt";
	this->ctrlInfoOut.open(ssci.str().c_str(), ofstream::out);
#endif

	// random number generators
	this->rngDelay = CreateObject<UniformRandomVariable> ();
	this->rngDelay->SetAttribute ("Min", DoubleValue (0.0));
	this->rngDelay->SetAttribute ("Max", DoubleValue (500.0));
	this->rngMyPkt = CreateObject<UniformRandomVariable> ();
	this->rngMyPkt->SetAttribute ("Min", DoubleValue (0.0));
	this->rngMyPkt->SetAttribute ("Max", DoubleValue (500.0));
	this->rngRtPkt = CreateObject<UniformRandomVariable> ();
	this->rngRtPkt->SetAttribute ("Min", DoubleValue (0.0));
	this->rngRtPkt->SetAttribute ("Max", DoubleValue (1000.0));
}

MyNode::~MyNode() {
	// close file pointers
#ifdef DEBUG_FLOW_OUT
	this->flowOut.close();
	this->ctrlOut.close();
	this->ctrlInfoOut.close();
#endif

#ifdef DEBUG_NODE_OUT
	this->nodeOut.close();
#endif

	// delete tables
	delete ncTable;
	delete flowTable;
	delete ctrlFlowTable;
	delete routeTable;
	delete arreqSentTable;
	delete arrepRecvTable;
}



Time MyNode::getDelayDifference(Ipv4Address ipAddr, Time current){
	NeighborEntry* entry = this->ncTable->get(ipAddr);
	Time prevDstDelayTime = entry->getPrevDstDelayMsrTime();
	Time diff;
	if(prevDstDelayTime != Seconds(0)){
		diff = current - prevDstDelayTime;
	} else {
		diff = Seconds(atof(MyConfig::instance().getValue("DelayMeasurementInterval").c_str()));
	}

	return diff;
}

double MyNode::getMyAvailableBandwidth(NeighborEntry* entry) {
	double totalAllocBW = 0.0;
	for(Flow flow : entry->getFlowList()){
		FlowEntry* flowEntry = this->flowTable->getFlowEntry(flow);
		totalAllocBW += flowEntry->getAllocatedBandwidth(entry->getNodeId());
	}

	double maxBW = MyConfig::instance().getMaxBandwidth();
	return maxBW - totalAllocBW;
}

void MyNode::accumulateLinkQuality(LinkQuality* lq, NeighborEntry* entry) {
	double availableBW = getMyAvailableBandwidth(entry);

	if(lq->getBandwidth() == 0.0 || lq->getBandwidth() > availableBW){
		lq->setBandwidth(availableBW);
	}
	lq->setDelay(lq->getDelay() + entry->getAverageDelay());
	lq->setJitter(lq->getJitter() + entry->getAverageJitter());
	double lossRate = lq->getLossRate();
	lossRate *= 1.0 - ((1.0 - lq->getLossRate()) * (1.0 - entry->getLossRate()));
	lq->setLossRate(lossRate);
}

bool MyNode::isControlPort(int port){
	vector<int> ctrlPorts = MyConfig::instance().getControlPorts();
	for(int ctrlPort : ctrlPorts){
		if(port == ctrlPort){
			return true;
		}
	}
	return false;
}

void MyNode::sendPacketWithJitter(Time jitter, Ptr<Socket> socket, Ptr<Packet> pkt){
	Simulator::Schedule(jitter, &MyNode::sendPacket, this, socket, pkt);
}

void MyNode::sendPacket(MyNode* myNode, Ptr<Socket> socket, Ptr<Packet> pkt){
	myNode->_sendPacket(socket, pkt);
}

void MyNode::_sendPacket(Ptr<Socket> socket, Ptr<Packet> pkt){
	socket->Send(pkt);
	socket->Close();
}

void MyNode::sendRoutingPacket(Ipv4Address target, string rtMsg){
	TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
	int port = MyConfig::instance().getPortByName("RoutingPort");
	InetSocketAddress rtMsgAddr = InetSocketAddress (target, port);
	int pktSize = rtMsg.size() + 20;
	Ptr<Socket> socket = Socket::CreateSocket(this->node, tid);
	socket->SetAllowBroadcast(true);
	socket->Connect(rtMsgAddr);

	// debug
#ifdef DEBUG_PRINT
	//NS_LOG_UNCOND("[Node " << this->nodeId << "] send RoutingMsg to " << target << ":" << port << " (ifIdx=" << ifIdx << ") t=" << Simulator::Now().GetMilliSeconds());
	//NS_LOG_UNCOND(" -pkt: " << rtMsg);
#endif

	// send DM reply packet to the DM sender.
	Ptr<Packet> pkt = createPacket(rtMsg, pktSize);
	uint32_t jitter = this->rngRtPkt->GetInteger();
	sendPacketWithJitter(MicroSeconds(jitter + this->nodeId), socket, pkt);
}

const int MyNode::getScheme() const {
	return this->scheme;
}

void MyNode::addIpv4Address(ns3::Ipv4Address addr){
	this->ipv4List.push_back(addr);
}

vector<ns3::Ipv4Address> MyNode::getIpv4List(){
	return this->ipv4List;
}

NeighborTable* MyNode::getNcTable() {
	return this->ncTable;
}

FlowTable* MyNode::getFlowTable() {
	return this->flowTable;
}

ControlFlowTable* MyNode::getControlFlowTable() {
	return this->ctrlFlowTable;
}

void MyNode::writeFlowLog() {
#ifdef DEBUG_FLOW_OUT
	stringstream ss;
	ss << Simulator::Now().GetSeconds();
	string outline = this->flowTable->getFormattedFlowOutput(ss.str());
	if(outline != "") this->flowOut << outline << "\n";

	string ctrlOutline = ctrlFlowTable->getControlOutput(ss.str());
	if(ctrlOutline != "") this->ctrlOut << ctrlOutline << "\n";

	// flowInfo
	if(this->numOfFlows < this->flowTable->getNumOfFlows()){
		stringstream ssfi;
		ssfi << "flowinfo_" << node->GetId() << ".txt";
		this->flowInfoOut.open(ssfi.str().c_str(), ofstream::out);
		this->flowInfoOut << this->flowTable->getAllFlowInfo() << "\n";
		this->flowInfoOut.close();
		this->numOfFlows = this->flowTable->getNumOfFlows();
	}

	//this->flowInfoOut << ss.str() << "\t" << flowTable->getAllFlowInfo() << "\n";
	this->ctrlInfoOut << ss.str() << "\t" << ctrlFlowTable->getControlFlowInfo() << "\n";
#endif
}

void MyNode::writeNeighborTable() {
#ifdef DEBUG_NODE_OUT
	this->nodeOut << this->ncTable->printNeighborTable(this->node) << "\n";
	this->nodeOut << "\n";
	this->nodeOut << this->flowTable->printFlowTable(this->nodeId) << "\n";
#endif
}

void MyNode::writeRouteTable() {
#ifdef DEBUG_NODE_OUT
	this->nodeOut << this->routeTable->toString() << "\n";
#endif
}

void MyNode::checkFlowQoS(MyNode* myNode, Time interval){
	myNode->_checkFlowQoS(interval);
}

/*
 * Handle all flows whose source is myself.
 */
void MyNode::_checkFlowQoS(Time interval){
	vector<FlowEntry*> flowEntries = flowTable->getAllFlowEntries();
	for(FlowEntry* flowEntry : flowEntries) {
		if( !flowEntry->isActive() || flowEntry->isRouteSearching() ) {
			continue;
		}

		Time now = Simulator::Now();
		if(this->nodeId == flowEntry->getFlow().getSrc()){
			// the flow starts from myself.
			uint32_t nextHop = routeTable->getNextHop(flowEntry->getFlow());
			if(nextHop == NODEID_NOT_FOUND){
#ifdef DEBUG_PRINT
				NS_LOG_UNCOND(" - no nextHop!! Route Re-discovery needed!");
#endif
#ifdef DEBUG_NODE_OUT
				this->nodeOut << " - no nextHop!! Route Re-discovery needed!" << "\n";
#endif
				continue;
			}

			// find nextHop entry to obtain QoS metrics
			NeighborEntry* nextHopEntry = this->ncTable->get(nextHop);
			if(!nextHopEntry) continue;

			// accumulate link quality of the src~nextHop link.
			LinkQuality lq;
			accumulateLinkQuality(&lq, nextHopEntry);
			double availableBW = MyConfig::instance().getMaxBandwidth() - flowEntry->getAvgRealTimeBandwidth();
			lq.setBandwidth(availableBW);

#ifdef DEBUG_NODE_OUT
			// debug
			//this->nodeOut << "  - avg real-time BW = " << flowEntry->getAvgRealTimeBandwidth() << "\n";
			//this->nodeOut << "  - allocated BW = " << flowEntry->getAllocatedBandwidth(entry->getNodeId()) << "\n";
#endif

			//TODO: check flow unsatisfactory & threshold
			// compare current link quality with the hop requirement.
			/*if(flowEntry->getHopQosReq().isSatisfactory(&lq)){
				NS_LOG_UNCOND("[Node " << this->nodeId << "][checkFlowHopQoS] hop QoS is satisfactory:");
#ifdef DEBUG_NODE_OUT
				this->nodeOut << "[Node " << this->nodeId << "][checkFlowHopQoS] hop QoS is satisfactory:\n";
#endif
			} else {
				NS_LOG_UNCOND("[Node " << this->nodeId << "][checkFlowHopQoS] hop QoS is NOT satisfactory:");
#ifdef DEBUG_NODE_OUT
				this->nodeOut << "[Node " << this->nodeId << "][checkFlowHopQoS] hop QoS is NOT satisfactory:\n";
#endif
				// TODO: how to solve?

			}

			NS_LOG_UNCOND(" - flow hop-by-hop QoS: " << flowEntry->getHopQosReq().serialize());
#ifdef DEBUG_NODE_OUT
			this->nodeOut << " - flow hop-by-hop QoS: " << flowEntry->getHopQosReq().serialize() << "\n";
#endif
			// debug
			NS_LOG_UNCOND(" - sending PathProbe to " << nextHop << ", nodeId=" << nextHopEntry->getNodeId());
#ifdef DEBUG_NODE_OUT
			this->nodeOut << " - sending PathProbe to " << nextHop << ", nodeId=" << nextHopEntry->getNodeId() << "\n";
#endif
			*/

			// send PathProbe to the nextHop
			Route* route = this->routeTable->getRoute(flowEntry->getFlow());
			PathProbe probe(flowEntry->getFlow(), route->getHopCount(), flowEntry->getAppReqSeqNo(), flowEntry->getQosReq(), lq);
			probe.setTrace(flowEntry->getSrcRoute());
			sendRoutingPacket(nextHopEntry->getIp(), probe.serialize());
		}
	}

	// periodically check flows by scheduling the next event
	Simulator::Schedule(interval, &MyNode::checkFlowQoS, this, interval);
}

void MyNode::checkUnicastDelayMeasurement(MyNode* myNode, Time interval) {
	myNode->_checkUnicastDelayMeasurement(interval);
}

void MyNode::_checkUnicastDelayMeasurement(Time interval) {
	this->ncTable->checkSendDelayMeasurement();
	Simulator::Schedule (interval, &MyNode::checkUnicastDelayMeasurement, this, interval);
}

void MyNode::checkBroadcastHello(MyNode* myNode, Time interval) {
	myNode->_checkBroadcastHello(interval);
}

void MyNode::_checkBroadcastHello(Time interval) {
	Hello* hello = generateHello();
	Ptr<Packet> pkt = createPacket(hello->serialize(), sizeof(Hello));
	uint32_t jitter = this->rngDelay->GetInteger();

	TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
	InetSocketAddress helloAddr = InetSocketAddress (Ipv4Address::GetBroadcast(), MyConfig::instance().getPortByName("HelloPort"));
	Ptr<Socket> helloSocket = Socket::CreateSocket(this->node, tid);
	helloSocket->SetAllowBroadcast(true);
	helloSocket->Connect(helloAddr);

#ifdef DEBUG_PRINT
	// debug
	//NS_LOG_UNCOND("[Node " << node->GetId() << "] broadcast Hello, t=" << Simulator::Now().GetMilliSeconds());
	//NS_LOG_UNCOND(" - " << hello->serialize());
#endif

	sendPacketWithJitter(MicroSeconds(jitter), helloSocket, pkt);
	delete hello;

	// MyNode stat
	Flow flow(this->nodeId, 5555, NODEID_BROADCAST, MyConfig::instance().getPortByName("HelloPort"), FlowType::UDP);
	PacketInfo pktInfo((long)(Simulator::Now().GetMilliSeconds()), flow, 0, sizeof(Hello));
	handlePacketInfo(NODEID_BROADCAST, pktInfo);

	// schedule the next Hello broadcast
	Simulator::Schedule (interval, &MyNode::checkBroadcastHello, this, interval);
}

Hello* MyNode::generateHello(){
	int seqNo = getAndIncrementHelloSeqNo();
	int numFlows = 0;
	//double allocBW = this->flowTable->getAllocatedBW(ifIdx);
	//double occBW = this->flowTable->getOccupiedBW(ifIdx);
	double allocBW = 0.0;
	double occBW = 0.0;
	Hello* hello = new Hello(this->nodeId, seqNo, numFlows, occBW, allocBW, true);
	hello->setNeighbors(this->ncTable->getNeighborIDs());

	if(this->scheme == SCHEME_1){
		// average bandwidth occupation of all neighbor link
#ifdef DEBUG_PRINT
		//NS_LOG_UNCOND("[Node " << this->nodeId << "] avgResidualBW = " << this->getAvgResidualBW());
#endif
#ifdef DEBUG_NODE_OUT
		//this->nodeOut << "[Node " << this->nodeId << "] avgResidualBW = " << this->getAvgResidualBW() << "\n";
#endif
		hello->setAvgOccBw(this->getAvgResidualBW());
	}

	return hello;
}

void MyNode::sendDelayMeasurement(Ipv4Address target, DelayMeasurement dm, int pktSize) {
	TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
	int port = MyConfig::instance().getPortByName("DelayMeasurePort"); // commented "+ifIdx"
	InetSocketAddress dmReqAddr = InetSocketAddress (target, port);
	Ptr<Socket> dmReqSocket = Socket::CreateSocket(this->node, tid);
	dmReqSocket->Connect(dmReqAddr);
	//dmReqSocket->BindToNetDevice(this->node->GetDevice(ifIdx)); // not for point-to-point
	Time now = Simulator::Now();
	string dmSerialized = dm.serialize();

	// send DM reply packet to the DM sender.
	Ptr<Packet> dmPkt = MyNode::createPacket(dmSerialized, pktSize);
	uint32_t jitter = this->rngDelay->GetInteger();
	Time unicastTime = now + MicroSeconds(jitter);
#ifdef DEBUG_NODE_OUT
	//this->nodeOut << "[Node " << node->GetId() << "] unicast DM(" << dm.getType() << ") to " << target << "(ifIdx " << ifIdx << "), t=" << unicastTime << "\n";
#endif
#ifdef DEBUG_PRINT
	//NS_LOG_UNCOND("[Node " << node->GetId() << "] unicast DM(" << dm.getType() << ") to " << target << "(ifIdx " << ifIdx << "), t=" << unicastTime);
	//NS_LOG_UNCOND(" - " << dmSerialized);
#endif

	sendPacketWithJitter(MicroSeconds(jitter), dmReqSocket, dmPkt);

	// MyNode stat
	Flow flow(this->nodeId, 5555, nodeIdMap->getNodeId(target), MyConfig::instance().getPortByName("DelayMeasurePort"), FlowType::UDP);
	PacketInfo pktInfo((long)(now.GetMilliSeconds()), flow, 0, pktSize);
	handlePacketInfo(nodeIdMap->getNodeId(target), pktInfo);
}

void MyNode::setupRoute(MyNode* myNode, Flow flow) {
	myNode->_setupRoute(flow);
}

void MyNode::_setupRoute(Flow flow) {
#ifdef DEBUG_PRINT
	NS_LOG_UNCOND("[Node "<< this->nodeId <<"] setupRoute, flow: " << flow.toString());
#endif
#ifdef DEBUG_NODE_OUT
	this->nodeOut << "[Node "<< this->nodeId <<"] setupRoute, flow: " << flow.toString() << "\n";
#endif
	ARREPRecvEntry* entry = this->arrepRecvTable->getEntry(flow);

	if(!entry || entry->getArrepList().size() == 0){
#ifdef DEBUG_PRINT
		NS_LOG_UNCOND(" - No available route found!!");
#endif
#ifdef DEBUG_NODE_OUT
		this->nodeOut << " - No available route found!!" << "\n";
#endif
		return;
	}

	ARREP optimalARREP = getOptimalARREP(entry->getArrepList());
	RouteSetup rs;
	rs.parseFromARREP(optimalARREP);
	uint32_t nextHop = rs.getTrace()[1];

#ifdef DEBUG_PRINT
	NS_LOG_UNCOND("[Node "<< this->nodeId <<"] optimal route: " << optimalARREP.serialize());
#endif
#ifdef DEBUG_NODE_OUT
	this->nodeOut << "[Node "<< this->nodeId <<"] optimal route: " << optimalARREP.serialize() << "\n";
#endif
	if(this->ncTable->isNeighbor(rs.getFlow().getDst())){
		// Single-hop route. Destination is a neighbor, and this route is optimal.
		// Setup itself using KernelRouteController and TrafficController.
		Route* route = new Route(rs.getFlow(), rs.getFlow().getDst(), 1);
		this->routeTable->addRoute(route);
#ifdef DEBUG_NODE_OUT
		this->nodeOut << "[RT] added route: " << route->toString() << "\n";
#endif
		this->flowTable->setQoSReq(rs.getFlow(), route->getNextHop(), rs.getQosReq());
		this->ncTable->get(rs.getFlow().getDst())->addFlow(rs.getFlow());
		//this->flowTable->setAllocatedBW(rs.getFlow(), outgoingIface, rs.getQosReq().getBandwidth());
	} else {
		// send RouteSetup message to the nextHop of a trace.
		if(rs.getTrace().size() > 1){
			Route* route = new Route(rs.getFlow(), nextHop, rs.getTrace().size()-1);
			this->routeTable->addRoute(route);
#ifdef DEBUG_NODE_OUT
			this->nodeOut << "[RT] added route: " << route->toString() << "\n";
#endif
			this->flowTable->setQoSReq(rs.getFlow(), route->getNextHop(), rs.getQosReq());
			this->ncTable->get(nextHop)->addFlow(rs.getFlow());
			//this->flowTable->setAllocatedBW(rs.getFlow(), outgoingIface, rs.getQosReq().getBandwidth());

			// 170822, set hop-by-hop QoSReq for local repair
			// calculate hop-by-hop QoSReq.
			int h = rs.getTrace().size();
			double aDelay = rs.getQosReq().getDelay() - rs.getEndToEndQuality().getDelay();
			double deltaDelay = aDelay / (double)h;
			double hopDelayReq = (double)(this->ncTable->get(nextHop)->getDelay()) + deltaDelay;

			double aJitter = rs.getQosReq().getJitter() - rs.getEndToEndQuality().getJitter();
			double deltaJitter = aJitter / (double)h;
			double hopJitterReq = (double)this->ncTable->get(nextHop)->getAverageJitter() + deltaJitter;

			QoSRequirement hopQosReq;
			hopQosReq.setBandwidth(rs.getQosReq().getBandwidth());
			hopQosReq.setDelay(hopDelayReq);
			hopQosReq.setJitter(hopJitterReq);
			hopQosReq.setLossRate(rs.getQosReq().getLossRate());

			// debug
#ifdef DEBUG_PRINT
			NS_LOG_UNCOND(" - hop-by-hop QoSReq: " << hopQosReq.serialize());
#endif
#ifdef DEBUG_NODE_OUT
			this->nodeOut << " - hop-by-hop QoSReq: " << hopQosReq.serialize() << "\n";
#endif

			this->flowTable->setHopQoSReq(rs.getFlow(), route->getNextHop(), hopQosReq);
			this->flowTable->getFlowEntry(rs.getFlow())->setSrcRoute(rs.getTrace());
			sendRoutingPacket(this->ncTable->get(nextHop)->getIp(), rs.serialize());
		}
	}

	// schedule data packets based on FlowReuqest
	ARREPRecvEntry* arrepEntry = this->arrepRecvTable->getEntry(flow);
	FlowRequest flowReq = arrepEntry->getFlowReq();
	string msg(flowReq.getPktSize(), 'a');
	_schedulePacketsFromFlowRequest(flowReq, msg);
}

void MyNode::schedulePacketsFromFlowRequest(MyNode* myNode, FlowRequest flowReq, string msg) {
	myNode->_schedulePacketsFromFlowRequest(flowReq, msg);
}

void MyNode::_schedulePacketsFromFlowRequest(FlowRequest flowReq, string msg) {
	Route* route = this->routeTable->getRoute(flowReq.getFlow());
	Time now = Simulator::Now();
#ifdef DEBUG_PRINT
	//NS_LOG_UNCOND("[Node " << this->nodeId << "] schedulePkt, t=" << now.GetMilliSeconds());
	//NS_LOG_UNCOND(" - flowReq = " << flowReq.getFlow().toString());
#endif
	/*
	 * Temp: special actions: changing packet sending rate for sudden disturbance
	 */
	if(Simulator::Now() > Seconds(30.0)){
		// make node 6 send more packets than planned
		if(this->nodeId == 6){
#ifdef DEBUG_PRINT
			//NS_LOG_UNCOND("[Node " << this->nodeId << "] @@@@ SPECIAL ACTION: send more packets than planned");
#endif
#ifdef DEBUG_NODE_OUT
			//this->nodeOut << "[Node " << this->nodeId << "] @@@@ SPECIAL ACTION: send more packets than planned" << "\n";
#endif
			flowReq.setSendingRate(360);
			//QoSRequirement tempQoSReq(5000000.0, 100.0, 100.0, 0.3);
			//QoSRequirement tempHopQoSReq(5000000.0, 25.0, 25.0, 0.3);
			//FlowEntry* flowEntry = this->flowTable->getFlowEntry(flowReq.getFlow());
			//flowEntry->setQosReq(tempQoSReq);
			//flowEntry->setHopQosReq(tempHopQoSReq);
		}
	}
	/*
	 * End Temp
	 */

	if(route){
		if(flowReq.getStartTime() + flowReq.getDuration() < now){
			// stop
#ifdef DEBUG_PRINT
			NS_LOG_UNCOND("[Node " << this->nodeId << "] stopped sending MyPkts, t=" << now.GetMilliSeconds());
#endif
#ifdef DEBUG_NODE_OUT
			this->nodeOut << "[Node " << this->nodeId << "] stopped sending MyPkts, t=" << now.GetMilliSeconds() << "\n";
#endif

			// remove related entires
			this->flowTable->setFlowInactive(flowReq.getFlow());
			return;
		}
#ifdef DEBUG_PRINT
		//NS_LOG_UNCOND(" - sending a packet to nexthop " << route->getNextHop() << ", ifIdx=" << route->getOutgoingIface());
#endif
		// send a packet
		Ptr<MyNS3Packet> myPkt = CreateObject<MyNS3Packet>(flowReq.getFlow().getSrc(), flowReq.getFlow().getSrcPort(), flowReq.getFlow().getDst(), flowReq.getFlow().getDstPort(), msg);
		sendMyPacket(this->ncTable->get(route->getNextHop())->getIp(), myPkt, flowReq.getFlow().getType(), flowReq.getPktSize());

		// schedule the next packet
		Time nextTime = Seconds(1.0/(double)flowReq.getSendingRate());
		Simulator::Schedule(nextTime, &MyNode::schedulePacketsFromFlowRequest, this, flowReq, msg);
	} else {
		// find ncTable to check if dst is my neighbor.
		NeighborEntry* entry = this->ncTable->get(flowReq.getFlow().getDst());
		if(entry){
			// one-hop. send a packet
			Ptr<MyNS3Packet> myPkt = CreateObject<MyNS3Packet>(flowReq.getFlow().getSrc(), flowReq.getFlow().getSrcPort(), flowReq.getFlow().getDst(), flowReq.getFlow().getDstPort(), msg);
			sendMyPacket(entry->getIp(), myPkt, flowReq.getFlow().getType(), flowReq.getPktSize());

			// schedule the next packet
			Time nextTime = Seconds(1.0/(double)flowReq.getSendingRate());
			Simulator::Schedule(nextTime, &MyNode::schedulePacketsFromFlowRequest, this, flowReq, msg);
		} else {
#ifdef DEBUG_PRINT
			NS_LOG_UNCOND("[Node " << this->nodeId << "] Route Re-discovery by unsatisfactory route" << flowReq.getFlow().toString());
#endif
#ifdef DEBUG_NODE_OUT
			this->nodeOut << "[Node " << this->nodeId << "] Route Re-discovery by unsatisfactory route" << flowReq.getFlow().toString() << "\n";
#endif
			int seqNo = this->arreqSentTable->getNextSeqNo(flowReq.getFlow());
			broadcastARREQ(flowReq.getFlow(), seqNo, flowReq);
			// RouteSetup will callback this function after finding a route.
		}
	}
}

void MyNode::selectNodeFromFlowCheck(MyNode* myNode, int seqNo) {
	myNode->_selectNodeFromFlowCheck(seqNo);
}

void MyNode::_selectNodeFromFlowCheck(int seqNo) {
#ifdef DEBUG_PRINT
	NS_LOG_UNCOND("[Node " << this->nodeId << "] _selectNodeFromFlowCheck (seq: " << seqNo << ")");
#endif
#ifdef DEBUG_NODE_OUT
	this->nodeOut << "[Node " << this->nodeId << "] _selectNodeFromFlowCheck (seq: " << seqNo << ")" << "\n";
#endif

	FlowCheckRecvTable* table = this->flowCheckRecvMap[seqNo];
	uint32_t optimalDetour = table->getOptimalDetourNode();
	if(optimalDetour != NODEID_NOT_FOUND){

#ifdef DEBUG_PRINT
		NS_LOG_UNCOND(" - optimal detour node: " << optimalDetour);
#endif
#ifdef DEBUG_NODE_OUT
		this->nodeOut << " - optimal detour node: " << optimalDetour << "\n";
#endif
		// Insert optimalDetour into the previous source route trace.
		// e.g. src-...- prevHopToSrc - current - prevNextHop - ... - dst
		// e.g. src-...- prevHopToSrc - current - [optimalDetour] - prevNextHop - ... - dst
		vector<uint32_t> newSrcRoute = table->getSrcRoute();
		int myPosInTrace = 0;
		for(vector<int>::size_type i=0; i< newSrcRoute.size(); i++){
			if(this->nodeId == newSrcRoute[i]) myPosInTrace = i;
		}
		newSrcRoute.insert(newSrcRoute.begin()+1+myPosInTrace, optimalDetour);

		// Perform local repair with the selected optimal nodeID.
		performLocalRepair(
				table->getPrevNextHop(),
				optimalDetour,
				table->getNextHopToSrc(),
				table->getFlow(),
				newSrcRoute,
				table->getQosReq(),
				table->getEndToEndQuality());
	} else {
		// Error: optimal node not found
#ifdef DEBUG_PRINT
		NS_LOG_UNCOND(" - optimal detour node not found!!");
#endif
#ifdef DEBUG_NODE_OUT
		this->nodeOut << " - optimal detour node not found!!" << "\n";
#endif
	}

	// Remove FlowCheck-replies which maps to the seqNo.
	delete this->flowCheckRecvMap[seqNo];
	this->flowCheckRecvMap.erase(seqNo);
}

void MyNode::sendMyPacket(Ipv4Address target, Ptr<MyNS3Packet> myPkt, FlowType::Type type, int pktSize) {
	Time now = Simulator::Now();
	TypeId tid;
	if(type == FlowType::TCP){
		tid = TypeId::LookupByName ("ns3::TcpSocketFactory");
	} else {
		tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
	}
	int port = MyConfig::instance().getPortByName("MyDataPort");
	InetSocketAddress dmReqAddr = InetSocketAddress (target, port);
	Ptr<Socket> socket = Socket::CreateSocket(this->node, tid);

	socket->Connect(dmReqAddr);
	//socket->BindToNetDevice(this->node->GetDevice(ifIdx));

#ifdef DEBUG_PRINT
	// debug
	//NS_LOG_UNCOND("[Node " << this->nodeId << "] send myPkt to " << target << ":" << port << ", ifIdx=" << ifIdx << ", t=" << now.GetMilliSeconds() << ", pktSize=" << myPkt->getPktSize());
	//NS_LOG_UNCOND(" - srcPort " << myPkt->getAppSrcPort());
	//NS_LOG_UNCOND(" - dstPort " << myPkt->getAppDstPort());
	//NS_LOG_UNCOND(" - pkt: " << myPkt->serialize());
#endif

	// send DM reply packet to the DM sender.
	Ptr<Packet> pkt = createPacket(myPkt->serialize(), myPkt->getPktSize());
#if JITTERED_PKT_TX == 1
	uint32_t jitter = this->rngMyPkt->GetInteger();
	sendPacketWithJitter(MicroSeconds(jitter), socket, pkt);
#else
	_sendPacket(socket, pkt);
#endif

	// MyNode stat
	Flow flow(myPkt->getSrc(), myPkt->getAppSrcPort(), myPkt->getDst(), myPkt->getAppDstPort(), FlowType::UDP);
	PacketInfo pktInfo((long)(now.GetMilliSeconds()), flow, 0, myPkt->getPktSize());
	handlePacketInfo(nodeIdMap->getNodeId(target), pktInfo);
}

void MyNode::broadcastARREQ(Flow flow, int seqNo, FlowRequest flowReq){
	// debug
#ifdef DEBUG_PRINT
	NS_LOG_UNCOND("[Node " << this->nodeId << "][broadcastARREQ] flow: " << flow.toString() << ", seqNo=" << seqNo);
#endif
#ifdef DEBUG_NODE_OUT
	this->nodeOut << "[Node " << this->nodeId << "][broadcastARREQ] flow: " << flow.toString() << ", seqNo=" << seqNo << "\n";
#endif

	ARREQ arreq(flow, seqNo, 10, flowReq.getQosReq());
	arreq.addTrace(this->nodeId);
	sendRoutingPacket(Ipv4Address::GetBroadcast(), arreq.serialize());

	this->arrepRecvTable->addEntry(flow, seqNo, flowReq);
	Time rtSetupInterval = Seconds(1.0);
	Simulator::Schedule(rtSetupInterval, &MyNode::setupRoute, this, flow);
}

ARREP MyNode::getOptimalARREP(const vector<ARREP> arrepList) {
	ARREP temp;

	for(size_t i=0; i<arrepList.size(); i++){
		if(i == 0){
			temp = arrepList[i];
		} else {
			if(temp.getLinkQuality().compare(arrepList[i].getLinkQuality()) < 0 ){
				temp = arrepList[i];
			}
		}
	}
	return temp;
}

void MyNode::doRouting(MyNode* myNode, Ptr<MyNS3Packet> myPkt, FlowRequest flowReq) {
	myNode->_doRouting(myPkt, flowReq);
}

void MyNode::_doRouting(Ptr<MyNS3Packet> myPkt, FlowRequest flowReq) {
	// Check my routing table to find nextHop.
	Flow flow(myPkt->getSrc(), myPkt->getAppSrcPort(), myPkt->getDst(), myPkt->getAppDstPort(), flowReq.getFlow().getType());
	Route* route = this->routeTable->getRoute(flow);

#ifdef DEBUG_PRINT
	// debug
	NS_LOG_UNCOND("[Node " << this->nodeId << "][doRouting] flow: " << flow.toString() << ", t=" << Simulator::Now().GetMilliSeconds());
	NS_LOG_UNCOND("  " << this->routeTable->toString());
#endif
#ifdef DEBUG_NODE_OUT
	this->nodeOut << "[Node " << this->nodeId << "][doRouting] flow: " << flow.toString() << ", t=" << Simulator::Now().GetMilliSeconds() << "\n";
	this->nodeOut << "  " << this->routeTable->toString() << "\n";
#endif

	if(route){
		// Route exists. Send myPkt to the next hop.
		NeighborEntry* ncEntry = this->ncTable->get(route->getNextHop());
		if(ncEntry){
#ifdef DEBUG_PRINT
			NS_LOG_UNCOND(" - nexthop exists. Send myPkt to " << route->getNextHop());
			NS_LOG_UNCOND(" - route: " << route->toString());
#endif
#ifdef DEBUG_NODE_OUT
			this->nodeOut << " - nexthop exists. Send myPkt to " << route->getNextHop() << "\n";
			this->nodeOut << " - route: " << route->toString() << "\n";
#endif
			sendMyPacket(ncEntry->getIp(), myPkt, flowReq.getFlow().getType(), flowReq.getPktSize());
		}
		_schedulePacketsFromFlowRequest(flowReq, myPkt->getMsg());
	} else {
		// There is no next hop. Check if the destination is my neighbor.
		NeighborEntry* ncEntry = this->ncTable->get(flow.getDst());
		if(ncEntry){
#ifdef DEBUG_PRINT
			NS_LOG_UNCOND(" - Dst is nextHop. Send myPkt to " << ncEntry->getIp());
#endif
#ifdef DEBUG_NODE_OUT
			this->nodeOut << " - Dst is nextHop. Send myPkt to " << ncEntry->getIp() << "\n";
#endif
			sendMyPacket(ncEntry->getIp(), myPkt, flowReq.getFlow().getType(), flowReq.getPktSize());
			_schedulePacketsFromFlowRequest(flowReq, myPkt->getMsg());
		} else {
			// Not my packet, but no nextHop: need to find a route.
			// Broadcast ARREQ and wait for ARREPs for a while.
#ifdef DEBUG_PRINT
			NS_LOG_UNCOND(" - no nextHop. Sending ARREQ.");
#endif
#ifdef DEBUG_NODE_OUT
			this->nodeOut << " - no nextHop. Sending ARREQ." << "\n";
#endif
			int seqNo = this->arreqSentTable->getNextSeqNo(flow);
			broadcastARREQ(flow, seqNo, flowReq);
		}
	}
}

void MyNode::performLocalRepair(uint32_t prevNextHop, uint32_t newNextHop,
		uint32_t nextHopToSrc, Flow flow, vector<uint32_t> srcRoute,
		QoSRequirement qosReq, LinkQuality endToEndQuality) {
#ifdef DEBUG_PRINT
	NS_LOG_UNCOND("[Node "<< this->nodeId << "] performLocalRepair");
#endif
#ifdef DEBUG_NODE_OUT
	this->nodeOut << "[Node "<< this->nodeId << "] performLocalRepair" << "\n";
#endif

	// TODO: Control message generation (for local repair request)
	// Ask the selected neighbor to add a new flow entry.
	// Modify the flow entry of the current node.
	// Modify the route entry of the current node.
	// Ask src (all reverse path nodes) to update the source route for this flow.

	RouteSetup rs;
	rs.setFlow(flow);
	rs.setQosReq(qosReq);
	rs.setSeqNo(this->flowTable->getFlowEntry(flow)->getAppReqSeqNo());
	rs.setEndToEndQuality(endToEndQuality);

	rs.setTrace(srcRoute);
	// Send RouteSetup to the new candidate.
	sendRoutingPacket(this->ncTable->get(newNextHop)->getIp(), rs.serialize());

	// Change the related route of my routing table.
	Route* route = this->routeTable->getRoute(flow);
	route->setHopCount(route->getHopCount() + 1);
	route->setNextHop(newNextHop);

	// Update flowTable with the new nextHop.
	FlowEntry* flowEntry = this->flowTable->getFlowEntry(flow);
	flowEntry->setFwdNodeId(newNextHop);
	flowEntry->setAllocatedBandwidth(prevNextHop, 0.0);
	flowEntry->addFlowStat(newNextHop);
	flowEntry->setAllocatedBandwidth(newNextHop, qosReq.getBandwidth());

	// Update ncTable for link quality change.
	this->ncTable->get(prevNextHop)->removeFlow(flow);
	this->ncTable->get(newNextHop)->addFlow(flow);

	// Send SourceRouteUpdate to the src to modify the source route.
	SourceRouteUpdate srcRtUpdate(rs.getFlow(), rs.getTrace());
	sendRoutingPacket(this->ncTable->get(nextHopToSrc)->getIp(), srcRtUpdate.serialize());

#ifdef DEBUG_PRINT
	NS_LOG_UNCOND(" - send SourceRouteUpdate to " << nextHopToSrc);
	NS_LOG_UNCOND(" - " << srcRtUpdate.serialize());
#endif
#ifdef DEBUG_NODE_OUT
	this->nodeOut << " - send SourceRouteUpdate to " << nextHopToSrc << "\n";
	this->nodeOut << " - " << srcRtUpdate.serialize() << "\n";
#endif
	// TODO: Send [    ] to the previous next-hop (prevNextHop) to cancel the previous route. (delete route, delete flow)
}

void MyNode::handlePacketInfo(int nodeId, PacketInfo pktInfo) {
	if(isControlPort(pktInfo.getFlow().getDstPort())){
		bool isTx = false;
		if(this->nodeId == pktInfo.getFlow().getSrc()){
			isTx = true;
		}
		this->ctrlFlowTable->add(isTx, &pktInfo);
	} else {
		this->flowTable->addToFlowEntry(nodeId, pktInfo);
	}
}

void MyNode::handleHello(Ptr<Node> node, Ipv4Address ipAddr, Hello hello) {
	this->ncTable->addAndUpdate(node, ipAddr, hello);
}

void MyNode::handleDelayMeasurement(Ptr<Node> node, Ptr<Socket> socket, Ipv4Address ipAddr, int ifIdx, DelayMeasurement dm) {
	Time now = Simulator::Now();

#ifdef DEBUG_PRINT
	// debug
//	NS_LOG_UNCOND("[Node "<< node->GetId() << "] handleDelayMeasurement (" << dm.getType() << ")(ifidx=" << ifIdx << "), t=" << now.GetMilliSeconds());
//	NS_LOG_UNCOND(" - from " << ipAddr);
//	NS_LOG_UNCOND(" - " << dm.serialize());
#endif
#ifdef DEBUG_NODE_OUT
//	this->nodeOut << "[Node "<< node->GetId() << "] handleDelayMeasurement (" << dm.getType() << ")(ifidx=" << ifIdx << ") from " << ipAddr << ", t=" << now.GetMilliSeconds() << "\n";
//	this->nodeOut << " - from " << ipAddr << "\n";
//	this->nodeOut <<" - " << dm.serialize() << "\n";
#endif

	if(dm.getType() == "req"){
		// prepare a reply message to send
		NeighborEntry* entry = this->ncTable->get(ipAddr);
		if(entry){
			// add reception to flow statistics
			Time difference = getDelayDifference(ipAddr, now);
			entry->setPrevDstDelayMsrTime(now);
			DelayMeasurement dmToSend("rep", dm.getSeqNo(), difference);

#ifdef DEBUG_PRINT
			// debug
			//NS_LOG_UNCOND(" -difference = " << difference);
#endif
#ifdef DEBUG_NODE_OUT
			//this->nodeOut << " -difference = " << difference << "\n";
#endif

			// reply dmToSend to the sender
			TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
			InetSocketAddress dmRepAddr = InetSocketAddress (ipAddr, MyConfig::instance().getPortByName("DelayMeasurePort")); // commented "+ifIdx"
			Ptr<Socket> dmRepSocket = Socket::CreateSocket(node, tid);
			dmRepSocket->Connect(dmRepAddr);
			//dmRepSocket->BindToNetDevice(node->GetDevice(ifIdx)); // not for point-to-point
			string dmToSendStr = dmToSend.serialize();
			Ptr<Packet> dmRepPkt = MyNode::createPacket(dmToSendStr, dmToSendStr.size() + 20);

#ifdef DEBUG_PRINT
			//NS_LOG_UNCOND("[Node " << node->GetId() << "] send DM (rep) to " << ipAddr << " (ifIdx=" << ifIdx << ")");
			//NS_LOG_UNCOND(" - " << dmToSendStr);
#endif
#ifdef DEBUG_NODE_OUT
			//this->nodeOut << "[Node " << node->GetId() << "] send DM (rep) to " << ipAddr << " (ifIdx=" << ifIdx << "), t=" << now.GetMilliSeconds() << "\n";
			//this->nodeOut << " - " << dmToSendStr << "\n";
#endif

			// send DM reply packet to the DM sender.
			uint32_t jitter = this->rngDelay->GetInteger();
			sendPacketWithJitter(MicroSeconds(jitter), dmRepSocket, dmRepPkt);

		} else {
			// no such neighbor exists. ignore the packet.
#ifdef DEBUG_PRINT
			//NS_LOG_UNCOND("  @@ Neighbor entry NULL!!! (" << ipAddr << ")");
#endif
		}

	} else if(dm.getType() == "rep"){
		// add DM info to neighbor entry and update delay (moving avg.)
		NeighborEntry* entry = this->ncTable->get(ipAddr);

		if(entry->getDmSeqNo() == dm.getSeqNo()){
			// update delay
			Time delay = now - entry->getDmSendTime();
			entry->addDelayToMovingAvg(delay.GetMilliSeconds()/2);
			entry->setDelay(delay.GetMilliSeconds()/2);

#ifdef DEBUG_PRINT
			// debug
			//NS_LOG_UNCOND("[Node " << this->nodeId << "]");
			//NS_LOG_UNCOND("  -delay(now) = " << entry->getDelay());
			//NS_LOG_UNCOND("  -delay(avg) = " << entry->getAverageDelay());
#endif
#ifdef DEBUG_NODE_OUT
			//this->nodeOut << "[Node " << this->nodeId << "] from " << ipAddr << "\n";
			//this->nodeOut << "  -delay(now) = " << entry->getDelay() << "\n";
			//this->nodeOut << "  -delay(avg) = " << entry->getAverageDelay() << "\n";
#endif

			Time prevSrcDelayMsrTime = entry->getPrevSrcDelayMsrTime();
			Time srcDifference;
			if(prevSrcDelayMsrTime != Seconds(0)){
				srcDifference = now - prevSrcDelayMsrTime;
			} else {
				srcDifference = Seconds(atof(MyConfig::instance().getValue("DelayMeasurementInterval").c_str()));
			}
			//this->ncTable->updateSrcPrevDelayMsrTime(ipAddr, now);
			entry->setPrevSrcDelayMsrTime(now);

			//parse dstDifference from DM msg.
			Time jitter = dm.getTimeDifference() - srcDifference;
			this->ncTable->updateJitter(entry->getNodeId(), jitter.GetMilliSeconds());

			// increment seqNo
			entry->incrementDmSeqNo();
		}
	}
}

void MyNode::handleARREQ(string str, Ipv4Address clientIP, int ifIdx) {
	uint32_t clientNodeId = nodeIdMap->getNodeId(clientIP);
	ARREQ arreq;
	arreq.parse(str);

#ifdef DEBUG_PRINT
	// debug
	NS_LOG_UNCOND("[Node "<< this->nodeId <<"] handleARREQ (from " << clientNodeId << ", trIdx=" << arreq.getTrace().size() << ") t=" << Simulator::Now().GetMilliSeconds());
	NS_LOG_UNCOND(" - " << str);
	NS_LOG_UNCOND(" - seqNo=" << arreq.getSeqNo());
#endif
#ifdef DEBUG_NODE_OUT
	this->nodeOut << "[Node "<< this->nodeId <<"] handleARREQ (from " << clientNodeId << ", trIdx=" << arreq.getTrace().size() << ") t=" << Simulator::Now().GetMilliSeconds() << "\n";
	this->nodeOut << " - " << str << "\n";
	this->nodeOut << " - seqNo=" << arreq.getSeqNo() << "\n";
#endif

	// if the ARREQ is already passed by myself, ignore it for preventing loop.
	for(uint32_t traceId : arreq.getTrace()){
		if(this->nodeId == traceId){
			//ignore already handled ARREQ
			return;
		}
	}

	// If ARREQ with the same seqNo arrives more than repeat threshold, discard it.
	this->arrepRecvTable->addEntry(arreq.getFlow(), arreq.getSeqNo()); // no flowReq for intermediate nodes
	ARREPRecvEntry* arrepEntry = this->arrepRecvTable->getEntry(arreq.getFlow());
	if(arrepEntry->getDupArreqCount() >= 2){
#ifdef DEBUG_PRINT
		NS_LOG_UNCOND(" -ignoring a duplicate ARREQ exceeding threshold");
#endif
#ifdef DEBUG_NODE_OUT
		this->nodeOut << " -ignoring a duplicate ARREQ exceeding threshold" << "\n";
#endif
		return;
	}

	// decrement TTL
	arreq.decrementTTL();


	// Accumulate previous hop's QoS values.
	NeighborEntry* prevHop = ncTable->get(clientIP);
	accumulateLinkQuality(arreq.getLinkQuality(), prevHop);

	QoSRequirement qosReq = arreq.getQosReq();
#ifdef DEBUG_PRINT
	NS_LOG_UNCOND("  -bw: " << qosReq.getBandwidth() << " vs " << arreq.getLinkQuality()->getBandwidth());
	NS_LOG_UNCOND("  -delay: " << qosReq.getDelay() << " vs " << arreq.getLinkQuality()->getDelay());
	NS_LOG_UNCOND("  -jiter: " << qosReq.getJitter() << " vs " << arreq.getLinkQuality()->getJitter());
	NS_LOG_UNCOND("  -loss: " << qosReq.getLossRate() << " vs " << arreq.getLinkQuality()->getLossRate());
	NS_LOG_UNCOND(this->ncTable->printNeighborTable(this->node));
#endif
#ifdef DEBUG_NODE_OUT
	this->nodeOut << "  -bw: " << qosReq.getBandwidth() << " vs " << arreq.getLinkQuality()->getBandwidth() << "\n";
	this->nodeOut << "  -delay: " << qosReq.getDelay() << " vs " << arreq.getLinkQuality()->getDelay() << "\n";
	this->nodeOut << "  -jiter: " << qosReq.getJitter() << " vs " << arreq.getLinkQuality()->getJitter() << "\n";
	this->nodeOut << "  -loss: " << qosReq.getLossRate() << " vs " << arreq.getLinkQuality()->getLossRate() << "\n";
#endif

	if(!qosReq.isSatisfactory(arreq.getLinkQuality())){
#ifdef DEBUG_PRINT
		NS_LOG_UNCOND(" -QoSReq is not satisfactory... drop it.");
#endif
#ifdef DEBUG_NODE_OUT
		this->nodeOut << " -QoSReq is not satisfactory... drop it." << "\n";
#endif
		return;
	} else {
#ifdef DEBUG_PRINT
		NS_LOG_UNCOND(" -QoSReq is satisfactory. ");
#endif
#ifdef DEBUG_NODE_OUT
		this->nodeOut << " -QoSReq is satisfactory. " << "\n";
#endif
	}

	// If destination is a neighbor, accumulate QoS values of the destination link.
	// Otherwise, relay (re-broadcast) ARREQ.
	uint32_t nextHop = routeTable->getNextHop(arreq.getFlow());
	if((nextHop != NODEID_NOT_FOUND) && nextHop == arreq.getFlow().getDst()){
		// It knows destination as a neighbor.
		// Accumulate QoS values of the destinaion link.
		NeighborEntry* nextHop = ncTable->get(arreq.getFlow().getDst());
		accumulateLinkQuality(arreq.getLinkQuality(), nextHop);

		// TODO: Determine if requirement is satisfactory.
		// - Proposed scheme,
		// - Existing work

		// If requirement is satisfactory, send ARREP back to sender.
		// generate ARREP and send back to the previous hop.
#ifdef DEBUG_PRINT
		NS_LOG_UNCOND(" - generate ARREP from neighbor.");
#endif
#ifdef DEBUG_NODE_OUT
		this->nodeOut << " - generate ARREP from neighbor." << "\n";
#endif
		ARREP arrep = ARREP::deriveFrom(arreq);
		arrep.addTrace(this->nodeId);
		arrep.addTrace(arreq.getFlow().getDst());

		// send ARREP
		sendRoutingPacket(clientIP, arrep.serialize());
	}
	else if(this->nodeId == arreq.getFlow().getDst()){
		// Destination is myself.
		// If requirement is satisfactory, send ARREP back to sender.
		// generate ARREP and send back to the previous hop.
#ifdef DEBUG_PRINT
		NS_LOG_UNCOND(" - generate ARREP from dst");
#endif
#ifdef DEBUG_NODE_OUT
		this->nodeOut << " - generate ARREP from dst" << "\n";
#endif
		arreq.addTrace(arreq.getFlow().getDst());
		ARREP arrep = ARREP::deriveFrom(arreq);

		// send ARREP
		sendRoutingPacket(clientIP, arrep.serialize());
	}
	else {
		// destination is not a neighbor
		// relay ARREQ based on TTL.
		if(arreq.getTTL() == 0) return;

#ifdef DEBUG_PRINT
		NS_LOG_UNCOND(" - relay ARREQ.");
#endif
#ifdef DEBUG_NODE_OUT
		this->nodeOut << " - relay ARREQ." << "\n";
#endif

		// broadcast ARREQ
		arreq.addTrace(this->nodeId);
		sendRoutingPacket(Ipv4Address::GetBroadcast(), arreq.serialize());
	}
}

void MyNode::handleARREP(string str, Ipv4Address clientIP, int ifIdx) {
	uint32_t clientNodeId = nodeIdMap->getNodeId(clientIP);
	ARREP arrep;
	arrep.parse(str);

#ifdef DEBUG_PRINT
	// debug
	NS_LOG_UNCOND("[Node "<< this->nodeId <<"] handleARREP (from " << clientNodeId << ") (ifIdx=" << ifIdx << ") t=" << Simulator::Now().GetMilliSeconds());
	NS_LOG_UNCOND(" - " << str);
#endif
#ifdef DEBUG_NODE_OUT
	this->nodeOut << "[Node "<< this->nodeId <<"] handleARREP (from " << clientNodeId << ") (ifIdx=" << ifIdx << ") t=" << Simulator::Now().GetMilliSeconds() << "\n";
	this->nodeOut << " - " << str << "\n";
#endif

	if(this->nodeId == arrep.getFlow().getSrc()){
		// this is a source node who initiated ARREQ.
		// check sequence number and store this.
#ifdef DEBUG_PRINT
		NS_LOG_UNCOND(" - @@@@@@@@  add ARREP to ARREPRecvTable, flow: " << arrep.getFlow().toString());
#endif
#ifdef DEBUG_NODE_OUT
		this->nodeOut << " - @@@@@@@@  add ARREP to ARREPRecvTable, flow: " << arrep.getFlow().toString() << "\n";
#endif
		this->arrepRecvTable->addARREP(arrep);

	} else {
		// this is an intermediate node.
		if(arrep.getTrace().size() > 0){
			int myPosInTrace = -1;
			for(size_t i=0; i<arrep.getTrace().size(); i++){
				if(this->nodeId == arrep.getTrace()[i]){
					myPosInTrace = i;
					break;
				}
			}

			// check src ability to talk via protocol
			if(myPosInTrace == 0){
				// store ARREP to the ARREP table.
#ifdef DEBUG_PRINT
				NS_LOG_UNCOND(" - @@ add ARREP to ARREPRecvTable, flow: " << arrep.getFlow().toString());
#endif
#ifdef DEBUG_NODE_OUT
				this->nodeOut << " - @@ add ARREP to ARREPRecvTable, flow: " << arrep.getFlow().toString() << "\n";
#endif
				this->arrepRecvTable->addARREP(arrep);
			}
			if(myPosInTrace > 0){
				uint32_t prevHop = arrep.getTrace()[myPosInTrace - 1];

				NeighborEntry* ncEntry = this->ncTable->get(prevHop);
				if(!ncEntry){
#ifdef DEBUG_PRINT
					NS_LOG_UNCOND(" -(ERROR) no prevHop for relaying ARREP! " << prevHop);
#endif
#ifdef DEBUG_NODE_OUT
					this->nodeOut << " -(ERROR) no prevHop for relaying ARREP! " << prevHop << "\n";
#endif
				}

				// add route to routeTable for the reverse route
//				Flow reverseFlow(arrep.getFlow().getDst(), arrep.getFlow().getDstPort(),
//								arrep.getFlow().getSrc(), arrep.getFlow().getSrcPort(),
//								arrep.getFlow().getType());
//				Route* route = new Route(reverseFlow, prevHop);
//				routeTable->addRoute(route);

				// send(relay) the ARREP to the previous hop.
#ifdef DEBUG_PRINT
				NS_LOG_UNCOND(" -relay ARREP to " << prevHop);
#endif
#ifdef DEBUG_NODE_OUT
				this->nodeOut << " -relay ARREP to " << prevHop << "\n";
#endif
				sendRoutingPacket(ncEntry->getIp(), arrep.serialize());
			}
		}
	}
}

void MyNode::handleARERR(string str, Ipv4Address clientIP, int ifIdx) {
	uint32_t clientNodeId = nodeIdMap->getNodeId(clientIP);
	ARERR arerr = ARERR::parse(str);

	// debug
#ifdef DEBUG_PRINT
	NS_LOG_UNCOND("[Node "<< this->nodeId <<"] handleARERR (from "<< clientNodeId << ")(ifIdx=" << ifIdx << ") t=" << Simulator::Now().GetMilliSeconds());
	NS_LOG_UNCOND(" - " << str);
#endif
#ifdef DEBUG_NODE_OUT
	this->nodeOut << "[Node "<< this->nodeId <<"] handleARERR (from "<< clientNodeId << ")(ifIdx=" << ifIdx << ") t=" << Simulator::Now().GetMilliSeconds() << "\n";
	this->nodeOut << " - " << str << "\n";
#endif

	if(this->nodeId == arerr.getFlow().getSrc()){
		// this is the source of the flow.
		// delete the current route because it is not satisfactory.
#ifdef DEBUG_PRINT
		NS_LOG_UNCOND(" - delete current route for route re-discovery...");
#endif
#ifdef DEBUG_NODE_OUT
		this->nodeOut << " - delete current route for route re-discovery..." << "\n";
#endif
		this->routeTable->deleteRoute(arerr.getFlow());

	} else {
		// intermediate of the flow.
		// relay ARERR to the source.
#ifdef DEBUG_PRINT
		NS_LOG_UNCOND(" - relay ARERR to src...");
#endif
#ifdef DEBUG_NODE_OUT
		this->nodeOut << " - relay ARERR to src..." << "\n";
#endif

		Flow reverseFlow(arerr.getFlow().getDst(), arerr.getFlow().getDstPort(),
				arerr.getFlow().getSrc(), arerr.getFlow().getSrcPort(),
				arerr.getFlow().getType());
		Route* reverseRoute = this->routeTable->getRoute(reverseFlow);

#ifdef DEBUG_PRINT
		//debug
		//NS_LOG_UNCOND(" @@@@ reverse flow = " << reverseFlow.toString());
		//NS_LOG_UNCOND(" @@@@ reverseRoute = " << reverseRoute->toString());
#endif
#ifdef DEBUG_NODE_OUT
		this->nodeOut << "@@@@ reverse flow = " << " @@@@ reverse flow = " << reverseFlow.toString() << "\n";
		this->nodeOut << " @@@@ reverseRoute = " << reverseRoute->toString() << "\n";
#endif

		NeighborEntry* ncEntry = this->ncTable->get(reverseRoute->getNextHop());
		if(ncEntry){
#ifdef DEBUG_PRINT
			//NS_LOG_UNCOND("    - src = " << nodeIdMap->getNodeId(ncEntry->getIp()));
#endif
#ifdef DEBUG_NODE_OUT
			this->nodeOut << "    - src = " << nodeIdMap->getNodeId(ncEntry->getIp()) << "\n";
#endif
			sendRoutingPacket(ncEntry->getIp(), str);
		}
	}
}

void MyNode::handleRouteSetup(string str, Ipv4Address clientIP, int ifIdx){
	uint32_t clientNodeId = nodeIdMap->getNodeId(clientIP);
	RouteSetup rs;
	rs.parse(str);

#ifdef DEBUG_PRINT
	// debug
	NS_LOG_UNCOND("[Node "<< this->nodeId <<"] handleRouteSetup (from "<< clientNodeId << ")(ifIdx=" << ifIdx << ") t=" << Simulator::Now().GetMilliSeconds());
	NS_LOG_UNCOND(" - " << str);
#endif
#ifdef DEBUG_NODE_OUT
	this->nodeOut << "[Node "<< this->nodeId <<"] handleRouteSetup (from "<< clientNodeId << ")(ifIdx=" << ifIdx << ") t=" << Simulator::Now().GetMilliSeconds() << "\n";
	this->nodeOut << " - " << str << "\n";
#endif

	int myPosInTrace = rs.getPositionInTrace(this->node);

	// handling error cases: not found in the trace
	if(myPosInTrace < 0) return;

	// reverse route
	Flow reverseFlow(rs.getFlow().getDst(),
			rs.getFlow().getDstPort(),
			rs.getFlow().getSrc(),
			rs.getFlow().getSrcPort(),
			rs.getFlow().getType());
	Route* reverseRoute = new Route(reverseFlow, clientNodeId, myPosInTrace);
	this->routeTable->addRoute(reverseRoute);

#ifdef DEBUG_PRINT
	// debug
	NS_LOG_UNCOND("  [RT] added route: " << reverseRoute->toString());
#endif
#ifdef DEBUG_NODE_OUT
	this->nodeOut << "  [RT] added route: " << reverseRoute->toString() << "\n";
#endif

	if(this->nodeId == rs.getFlow().getDst()){
		// This is the destination of the route.
		// setup only reverse route and finish.
		return;
	}

	// forward route
	uint32_t nextHopInTrace = rs.getNextHopOfCurrentNode(this->node);

	// Check the neighbor exists as a next hop.
	NeighborEntry* ncEntry = this->ncTable->get(nextHopInTrace);
	if(!ncEntry){
#ifdef DEBUG_PRINT
		NS_LOG_UNCOND("  [NC] nextHop ("<< nextHopInTrace << ") is not a neighbor!");
#endif
#ifdef DEBUG_NODE_OUT
		this->nodeOut << "  [NC] nextHop ("<< nextHopInTrace << ") is not a neighbor!" << "\n";
#endif
		return;
	}

	Route* route = new Route(rs.getFlow(), nextHopInTrace, rs.getTrace().size() - myPosInTrace - 1);
	this->routeTable->addRoute(route);

#ifdef DEBUG_PRINT
	// debug
	NS_LOG_UNCOND("  [RT] added route: " << route->toString());
#endif
#ifdef DEBUG_NODE_OUT
	this->nodeOut << "  [RT] added route: " << route->toString() << "\n";
#endif
	this->flowTable->setQoSReq(rs.getFlow(), route->getNextHop(), rs.getQosReq());
	this->flowTable->getFlowEntry(rs.getFlow())->setSrcRoute(rs.getTrace());
	ncEntry->addFlow(rs.getFlow());
	//this->flowTable->setAllocatedBW(rs.getFlow(), nextHopIfIdx, rs.getQosReq().getBandwidth());

	// 170822, set hop-by-hop QoSReq for local repair
	// calculate hop-by-hop QoSReq.
	int h = rs.getTrace().size();
	double aDelay = rs.getQosReq().getDelay() - rs.getEndToEndQuality().getDelay();
	double deltaDelay = aDelay / (double)h;
	double hopDelayReq = (double)(this->ncTable->get(nextHopInTrace)->getDelay()) + deltaDelay;

	double aJitter = rs.getQosReq().getJitter() - rs.getEndToEndQuality().getJitter();
	double deltaJitter = aJitter / (double)h;
	double hopJitterReq = (double)this->ncTable->get(nextHopInTrace)->getAverageJitter() + deltaJitter;

	QoSRequirement hopQosReq;
	hopQosReq.setBandwidth(rs.getQosReq().getBandwidth());
	hopQosReq.setDelay(hopDelayReq);
	hopQosReq.setJitter(hopJitterReq);
	hopQosReq.setLossRate(rs.getQosReq().getLossRate());
	this->flowTable->setHopQoSReq(rs.getFlow(), nextHopInTrace, hopQosReq);

	// Send RouteSetup to the nexthop.
	sendRoutingPacket(ncEntry->getIp(), rs.serialize());
}

void MyNode::handlePathProbe(string str, Ipv4Address clientIP, int ifIdx){
	uint32_t clientNodeId = nodeIdMap->getNodeId(clientIP);
	PathProbe probe;
	probe.parse(str);

#ifdef DEBUG_PRINT
	// debug
	NS_LOG_UNCOND("[Node "<< this->nodeId <<"] handlePathProbe (from "<< clientNodeId << ")(ifIdx=" << ifIdx << ") t=" << Simulator::Now().GetMilliSeconds());
	NS_LOG_UNCOND(" - " << str);
#endif
#ifdef DEBUG_NODE_OUT
	this->nodeOut << "[Node "<< this->nodeId <<"] handlePathProbe (from "<< clientNodeId << ")(ifIdx=" << ifIdx << ") t=" << Simulator::Now().GetMilliSeconds() << "\n";
	this->nodeOut << " - " << str << "\n";
#endif

	if(this->nodeId == probe.getFlow().getDst()){
		// This node is the destination of the flow.
		// Nothing has to be done.
		return;
	}

	// Accumulate the QoS values of the next-hop of the flow.
	uint32_t nextHop = this->routeTable->getNextHop(probe.getFlow());
	if(nextHop == NODEID_NOT_FOUND){
		// No next-hop found for the flow!!
		return;
	}

	NeighborEntry* nextHopEntry = this->ncTable->get(nextHop);
	if(!nextHopEntry){
		// No such neighbor entry (maybe not nexthop)
		return;
	}

	FlowEntry* flowEntry = this->flowTable->getFlowEntry(probe.getFlow());
	if(!flowEntry){
		// Error: No such flow entry.
		return;
	}

	// 171019, get available BW
	//double availableBW = MyConfig::instance().getMaxBandwidth() - flowEntry->getAvgRealTimeBandwidth();
	double availableBW = getMyAvailableBandwidth(nextHopEntry) + flowEntry->getAvgRealTimeBandwidth();
	if(availableBW < 0.0) availableBW = 0.0;

	// debug
#ifdef DEBUG_NODE_OUT
	this->nodeOut << "  - flow avgRtBW = " << flowEntry->getAvgRealTimeBandwidth() << "\n";
	this->nodeOut << "  - available BW = " << availableBW << "\n";
#endif

	// Measure the hop-by-hop link quality.
	LinkQuality hopLq(
			availableBW,
			nextHopEntry->getAverageDelay(),
			nextHopEntry->getAverageJitter(),
			nextHopEntry->getLossRate());

	// 170830, check hop-by-hop QoSReq
	if(flowEntry->getHopQosReq().isSatisfactory(&hopLq)){
#ifdef DEBUG_NODE_OUT
		this->nodeOut << " - @@ hop-by-hop QoS satisfactory.\n";
		//this->nodeOut << "   - hopReq: " << flowEntry->getHopQosReq().serialize() << "\n";
		//this->nodeOut << "   - hopLQ : " << hopLq.serialize() << "\n";
#endif

		// The partial path is satisfactory.
		// Accumulate end-to-end link quality and relay the PathProbe to the nexthop.
		accumulateLinkQuality(probe.getLinkQuality(), nextHopEntry);
		//probe.addTrace(this->nodeId);
		sendRoutingPacket(nextHopEntry->getIp(), probe.serialize());
		flowEntry->setUnsatisfactoryCount(0);

	} else {
		// Check threshold.
		int flowUnsatisfactoryCount = flowEntry->getUnsatisfactoryCount();
		if(flowUnsatisfactoryCount < MyConfig::instance().getFlowUnsatisThreshold()){
			flowEntry->setUnsatisfactoryCount(++flowUnsatisfactoryCount);
			return;
		}

		// FlowUnsatisfactoryCount exceeds threshold.
		//debug
#ifdef DEBUG_NODE_OUT
		this->nodeOut << " - @@ hop-by-hop QoS NOT satisfactory.\n";
		this->nodeOut << "   - hopReq: " << flowEntry->getHopQosReq().serialize() << "\n";
		this->nodeOut << "   - hopLQ : " << hopLq.serialize() << "\n";

		this->nodeOut << " - ## QoSReq NOT satisfactory! (unsatisfactoryCount=" << flowEntry->getUnsatisfactoryCount() << ")" << "\n";
		this->nodeOut << "  -lq    : " << probe.getLinkQuality()->serialize() << "\n";
		this->nodeOut << "  -qosReq: " << probe.getQosReq().serialize() << "\n";
		this->nodeOut << flowTable->printFlowTable(this->nodeId) << "\n";
#endif

		if(this->scheme == BASELINE) {
			/*
			 * Existing work as baseline.
			 * Reply ARERR back to source.
			 */

#ifdef DEBUG_PRINT
			//debug
			NS_LOG_UNCOND(" - BASELINE: send ARERR.");
#endif
#ifdef DEBUG_NODE_OUT
			this->nodeOut << " - BASELINE: send ARERR." << "\n";
#endif

			ARERR arerr(probe.getFlow(), this->nodeId, probe.getQosReq());
			sendRoutingPacket(clientIP, arerr.serialize());
		} else if(this->scheme == SCHEME_1) {
			/*
			 * Scheme 1.
			 * Select local repair node based on average occupied BW of neighbors.
			 */

#ifdef DEBUG_PRINT
			//debug
			NS_LOG_UNCOND(" - SCHEME 1: find detour based on average occupied BW.");
#endif
#ifdef DEBUG_NODE_OUT
			this->nodeOut << " - SCHEME 1: find detour based on average occupied BW." << "\n";
#endif

			// Find detour node candidates: neighbors who know the next-hop.
			vector<NeighborEntry*> detours = this->ncTable->getDetourNodes(nextHopEntry->getNodeId(), probe.getTrace());
			if(detours.size() > 0){
				// Sort candidates by average occupied bandwidth.
				NeighborEntry* selected = NULL;
				for(NeighborEntry* candidate : detours){
					if(!selected){
						selected = candidate;
					} else {
						if(this->flowTable->getOccupiedBandwidth(selected->getNodeId())
							< this->flowTable->getOccupiedBandwidth(candidate->getNodeId())){
							selected = candidate;
						}
					}
				}

				// Set candidate as a detour node.
				if(selected){
#ifdef DEBUG_PRINT
					NS_LOG_UNCOND(" - detour node selected as: " << selected->getNodeId() << ", avgOccBw=" << this->flowTable->getOccupiedBandwidth(selected->getNodeId()));
#endif
#ifdef DEBUG_NODE_OUT
					this->nodeOut << " - detour node selected as: " << selected->getNodeId() << ", avgOccBw=" << this->flowTable->getOccupiedBandwidth(selected->getNodeId()) << "\n";
#endif
					vector<uint32_t> trace = probe.getTrace();
					int myPosInTrace = 0;
					for(vector<int>::size_type i=0; i< trace.size(); i++){
						if(this->nodeId == trace[i]) myPosInTrace = i;
					}
					trace.insert(trace.begin()+1+myPosInTrace, selected->getNodeId());

					performLocalRepair(
							nextHopEntry->getNodeId(), selected->getNodeId(), clientNodeId,
							probe.getFlow(), trace,
							probe.getQosReq(), probe.getLinkQuality());
				}
			}

		} else if(this->scheme == SCHEME_2){
			/*
			 * Scheme 2.
			 * Get top-K unpopular flows of candidate nexthop neighbors, and select one.
			 */

#ifdef DEBUG_PRINT
			//debug
			NS_LOG_UNCOND(" - SCHEME 2: find detour based on average occupied BW.");
#endif
#ifdef DEBUG_NODE_OUT
			this->nodeOut << " - SCHEME 2: find detour based on average occupied BW." << "\n";
#endif
			// Get a seqNo for the new FlowCheck-replies.
			// Generate FlowCheckRecvTable for the seqNo.
			int seqNo = this->flowCheckSeqNo++;
			FlowCheckRecvTable* table = new FlowCheckRecvTable();
			table->setFlow(probe.getFlow());
			table->setQosReq(probe.getQosReq());
			table->setNextHopToSrc(clientNodeId);
			table->setPrevNextHop(nextHop);
			table->setEndToEndQuality(probe.getLinkQuality());
			table->setSrcRoute(probe.getTrace());
			this->flowCheckRecvMap[seqNo] = table;

			// Create and broadcast FlowCheck-request with the seqNo.
			FlowCheck flowCheck(ROUTE_FLOWCHECK_REQUEST);
			flowCheck.setSeqNo(seqNo);
			sendRoutingPacket(Ipv4Address::GetBroadcast(), flowCheck.serialize());

			// Schedule the selectNodeFromFlowCheck function.
			Time flowCheckRecvInterval = Seconds(0.1);
			Simulator::Schedule(flowCheckRecvInterval, &MyNode::selectNodeFromFlowCheck, this, seqNo);

		} else if(this->scheme == SCHEME_3){
			// Proposed scheme.
			// Create arreq as an intermediate node.
			// trace: pre-write src to here.
			// lifetime: hopcount from here to dst
			// qosreq: copy
			// linkquality: src to here
			// seqNO: ?? where can i find it?
			//ARREPRecvEntry* arrepEntry = this->arrepRecvTable->getEntry(probe.getFlow());
			int newSeqNo = probe.getRouteSeqNo() + 1;
			int hopCountToDst = this->routeTable->getRoute(probe.getFlow())->getHopCount();
			ARREQ arreq(probe.getFlow(), newSeqNo, hopCountToDst, probe.getQosReq());
			arreq.setLinkQuality(probe.getLinkQuality()); // use link quality without my forward link

			arreq.setTrace(probe.getTrace()); // trace until myself
			// TODO: 171203. trace should be partitioned until this node.
			// partitionTrace(probe.getTrace(), this->nodeId);


			// 170905: Hop-by-hop local repair?
			// What kind of information does this node have?
			// - neighbor's occupied bandwidth
			// - neighbor's flow stat?
			// - trace's next-next-hop
			// - if neighbor knows the route to the intermediate node?
			// Ask neighbors to summarize their neighbors' traffic status and get replies.

			// Broadcast ARREQ.
			arreq.addTrace(this->nodeId);
#ifdef DEBUG_PRINT
			NS_LOG_UNCOND("[Node "<< this->nodeId << "] ARREQ as local repair(intf 0): " << arreq.serialize());
#endif
#ifdef DEBUG_NODE_OUT
			this->nodeOut << "[Node "<< this->nodeId << "] ARREQ as local repair(intf 0): " << arreq.serialize() << "\n";
#endif
			sendRoutingPacket(Ipv4Address::GetBroadcast(), arreq.serialize());

			ARREPRecvEntry* arrepEntry = this->arrepRecvTable->getEntry(probe.getFlow());
			if(arrepEntry){
				//debug
#ifdef DEBUG_NODE_OUT
				this->nodeOut << "  #### ARREP List of the previous seqNo:" << "\n";
#endif
				for(ARREP arrep: arrepEntry->getArrepList()){
#ifdef DEBUG_NODE_OUT
					this->nodeOut << "      " <<arrep.serialize() << "\n";
#endif
				}

				this->arrepRecvTable->addEntry(probe.getFlow(), newSeqNo);
				Simulator::Schedule(Seconds(1.0), &MyNode::setupRoute, this, probe.getFlow());
			} else {
				// Error: previous flow must have been added to ARREPRecvTable.
#ifdef DEBUG_NODE_OUT
				this->nodeOut <<"  #### Error: ARREPRecvEntry of this flow is empty!";
#endif
			}
		} else {
			// Error: invalid configurations
		}

		flowEntry->setUnsatisfactoryCount(0);
	}
}

void MyNode::handleSourceRouteUpdate(string str, Ipv4Address clientIP, int ifIdx){
	uint32_t clientNodeId = nodeIdMap->getNodeId(clientIP);
	SourceRouteUpdate srcUpdate;
	srcUpdate.parse(str);

#ifdef DEBUG_PRINT
	// debug
	NS_LOG_UNCOND("[Node "<< this->nodeId <<"] handleSourceRouteUpdate (from "<< clientNodeId << ") t=" << Simulator::Now().GetMilliSeconds());
	NS_LOG_UNCOND(" - " << str);
#endif
#ifdef DEBUG_NODE_OUT
	this->nodeOut << "[Node "<< this->nodeId <<"] handleSourceRouteUpdate (from "<< clientNodeId << ") t=" << Simulator::Now().GetMilliSeconds() << "\n";
	this->nodeOut << " - " << str << "\n";
#endif

	if(srcUpdate.getFlow().getSrc() == this->nodeId){
		// Update sourceRouteMap with the packet.
		this->flowTable->getFlowEntry(srcUpdate.getFlow())->setSrcRoute(srcUpdate.getSrcRoute());

		// debug
		stringstream ss;
		for (uint32_t srcRtNode : srcUpdate.getSrcRoute()){
			ss << srcRtNode << " ";
		}
#ifdef DEBUG_PRINT
		NS_LOG_UNCOND(" - srcRoute: " << ss.str());
#endif
#ifdef DEBUG_NODE_OUT
		this->nodeOut << " - srcRoute: " << ss.str() << "\n";
#endif
	} else {
		// TODO: Change the route (forward route) if next-hop is changed in the trace.
		this->flowTable->getFlowEntry(srcUpdate.getFlow())->setSrcRoute(srcUpdate.getSrcRoute());

		// Forward this packet to the src using the routeTable.
		Flow reverseFlow(
				srcUpdate.getFlow().getDst(),
				srcUpdate.getFlow().getDstPort(),
				srcUpdate.getFlow().getSrc(),
				srcUpdate.getFlow().getSrcPort(),
				srcUpdate.getFlow().getType());
		uint32_t nextHopToSrc = this->routeTable->getRoute(reverseFlow)->getNextHop();

#ifdef DEBUG_PRINT
		// Debug
		NS_LOG_UNCOND(" - forward SourceRouteUpdate to " << nextHopToSrc);
#endif
#ifdef DEBUG_NODE_OUT
		this->nodeOut << " - forward SourceRouteUpdate to " << nextHopToSrc << "\n";
#endif
		sendRoutingPacket(this->ncTable->get(nextHopToSrc)->getIp(), srcUpdate.serialize());
	}
}

void MyNode::handleFlowCheck(string str, Ipv4Address clientIP, int ifIdx){
	uint32_t clientNodeId = nodeIdMap->getNodeId(clientIP);
	FlowCheck* flowCheck = new FlowCheck(ROUTE_FLOWCHECK_REQUEST);
	flowCheck->parse(str);

#ifdef DEBUG_PRINT
	// debug
	NS_LOG_UNCOND("[Node "<< this->nodeId <<"] handleFlowCheck (from "<< clientNodeId << ") t=" << Simulator::Now().GetMilliSeconds());
	NS_LOG_UNCOND(" - " << str);
#endif
#ifdef DEBUG_NODE_OUT
	this->nodeOut << "[Node "<< this->nodeId <<"] handleFlowCheck (from "<< clientNodeId << ") t=" << Simulator::Now().GetMilliSeconds() << "\n";
	this->nodeOut << " - " << str << "\n";
#endif

	if(flowCheck->getMsgType() == ROUTE_FLOWCHECK_REQUEST){
		// Create FlowCheck-reply with the same seqNo of FlowCheck-request.
		FlowCheck flowCheckReply(ROUTE_FLOWCHECK_REPLY);
		flowCheckReply.setSeqNo(flowCheck->getSeqNo());

		// Sort top-K unpopular flows.
		// Create SourceRouteStat for each flow with occupied bandwidth and add it to FlowCheck-reply.
		vector<FlowEntry*> unpopularFlows = this->flowTable->getUnpopularFlows(2);
		for(FlowEntry* unpopularFlow : unpopularFlows){
			SourceRouteStat srcStat;
			srcStat.setTrace(unpopularFlow->getSrcRoute());
			srcStat.setBandwidth(unpopularFlow->getAvgRealTimeBandwidth());
			flowCheckReply.addStat(srcStat);
		}

		// Send the FlowCheck-reply back to the client.
		sendRoutingPacket(clientIP, flowCheckReply.serialize());
		delete flowCheck;
	}
	else if(flowCheck->getMsgType() == ROUTE_FLOWCHECK_REPLY) {
		if(this->flowCheckRecvMap.find(flowCheck->getSeqNo()) == this->flowCheckRecvMap.end()){
			// No such seqNo. Silently discard this message.
			delete flowCheck;
		} else {
			// Add FlowCheck-reply to the map<uint32_t, FlowCheck> where uint32_t is a nodeID.
			FlowCheckRecvTable* table = this->flowCheckRecvMap[flowCheck->getSeqNo()];
			table->addFlowCheckReply(clientNodeId, flowCheck);
		}
	} else {
		// Invalid message type. Discard the message.
		delete flowCheck;
	}
}

void MyNode::handleMyPacket(Ptr<MyNS3Packet> myPkt, int pktSize, FlowType::Type type, Ipv4Address ipAddr) {
	Time now = Simulator::Now();

	// check and do routing
	if(checkDstOfMyPacket(GetPointer(myPkt))){
		// destination is myself.
		// TODO: report data to the application layer(?).
#ifdef DEBUG_PRINT
		//NS_LOG_UNCOND(" - ###### My Application Packet!");
#endif
	} else {
		// forward the packet to the destination.
		Flow flow(myPkt->getSrc(), myPkt->getAppSrcPort(), myPkt->getDst(), myPkt->getAppDstPort(), type);
		Route* route = this->routeTable->getRoute(flow);

		if(route){
#ifdef DEBUG_NODE_OUT
			//this->nodeOut << " - forward MyPkt("<< myPkt->getSrc() <<"-->"<< myPkt->getDst() <<") to " << route->getNextHop() << "\n";
#endif
			NeighborEntry* nextHopEntry = this->ncTable->get(route->getNextHop());
			sendMyPacket(nextHopEntry->getIp(), myPkt, type, pktSize);
		} else {
#ifdef DEBUG_PRINT
			NS_LOG_UNCOND("[Node " << this->nodeId << "][handleMyPkt] Error: No Route Exists!!");
#endif
#ifdef DEBUG_NODE_OUT
			this->nodeOut << "[Node " << this->nodeId << "][handleMyPkt] Error: No Route Exists!!" << "\n";
#endif
		}
	}
	myPkt->Unref();
}

const bool MyNode::checkDstOfMyPacket(const MyNS3Packet* myPkt) const {
	if(myPkt){
		if(this->nodeId == myPkt->getDst()){
			return true;
		} else {
			return false;
		}
	} else {
		return false;
	}
}

int MyNode::getAndIncrementHelloSeqNo() {
	return this->helloSeqNo++;
}

/*
 * Scheme 2: average residual bandwidth
 */
const double MyNode::getAvgResidualBW() {
	/*
	 * for flowEntry in flows:
     *    flowEntry->getOccupiedBW()
     *    Route route = routeTable->getRoute(flowEntry->getFlow());
     *    int nextHop = route->getNextHop();
     *    ncTable->get(nextHop)
     *
	 */

	// all neighbors' list
	map<uint32_t, double> ncResMap;
	map<uint32_t, NeighborEntry*> ncMap = this->ncTable->getMap();
	for(map<uint32_t, NeighborEntry*>::const_iterator it=ncMap.begin(); it!= ncMap.end(); it++){
		ncResMap[it->first] = MyConfig::instance().getMaxBandwidth();
	}

	// set residual bandwidth for each neighbor.
	vector<FlowEntry*> flowEntries = this->flowTable->getAllFlowEntries();
	for(FlowEntry* flowEntry : flowEntries) {
		uint32_t neighborId = this->routeTable->getNextHop(flowEntry->getFlow());
		ncResMap[neighborId] = ncResMap[neighborId] - flowEntry->getAvgRealTimeBandwidth();
	}

	// Calculate the average of residual bandwidth values.
	if(ncResMap.size() > 0){
		double sum = 0.0;
		pair<uint32_t, double> p;
		BOOST_FOREACH (p, ncResMap){
			sum += p.second;
		}
		return sum / (double)ncResMap.size();
	} else {
		return 0.0;
	}
}


/***************************************************
 * STATIC FUNCTIONS
 ***************************************************/

const bool MyNode::isMyAddress(Ptr<Node> node, const Ipv4Address addr){
	Ptr<Ipv4> myIpv4 = node->GetObject<Ipv4>();
	for(uint32_t i=0; i<myIpv4->GetNInterfaces(); i++){
		if(myIpv4->GetAddress(i, 0).GetLocal() == addr){
			return true;
		}
	}
	return false;
}

Ptr<Packet> MyNode::createPacket(string data, int pktSize) {
	uint8_t *buffer = reinterpret_cast<uint8_t*> (&data[0]);
	Ptr<Packet> pkt = Create<Packet>(buffer, pktSize);
	return pkt;
}

uint8_t* MyNode::getPacketDataBuffer(Ptr<Packet> packet) {
	uint8_t *buffer = new uint8_t[packet->GetSize()];
	packet->CopyData(buffer, packet->GetSize());
	return buffer;
}

const int MyNode::checkRoutingMessageType(const string msg) {
	vector<std::string> tokens;
	tokenizeString(msg, tokens, "@");
	return atoi(tokens[0].c_str());
}
