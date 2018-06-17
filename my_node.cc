/*
 * my_node.cc
 *
 *  Created on: Dec 13, 2016
 *      Author: bhshin
 */

#include "my_node.h"
#include "my_config.h"
#include "my_statistics.h"
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
#define DEBUG_SRC_RT_OUT
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
	this->numOfCtrlPorts = 0;
	this->srcRtDscvCount = 0;
	this->srcRtDscvFailCount = 0;

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
	this->ctrlInfoOut.close();
#endif

#ifdef DEBUG_SRC_RT_OUT
	stringstream ssrt;
	ssrt << "srcrtlog_" << node->GetId() << ".txt";
	this->srcRtOut.open(ssrt.str().c_str(), ofstream::out);
#endif

	// random number generators
	this->rngDelay = CreateObject<UniformRandomVariable> ();
	this->rngDelay->SetAttribute ("Min", DoubleValue (0.0));
	this->rngDelay->SetAttribute ("Max", DoubleValue (300.0));
	this->rngMyPkt = CreateObject<UniformRandomVariable> ();
	this->rngMyPkt->SetAttribute ("Min", DoubleValue (0.0));
	this->rngMyPkt->SetAttribute ("Max", DoubleValue (200.0));
	this->rngRtPkt = CreateObject<UniformRandomVariable> ();
	this->rngRtPkt->SetAttribute ("Min", DoubleValue (0.0));
	this->rngRtPkt->SetAttribute ("Max", DoubleValue (300.0));
	this->flowRandom = CreateObject<UniformRandomVariable> ();
	this->flowRandom->SetAttribute ("Min", DoubleValue (0.0));
	this->flowRandom->SetAttribute ("Max", DoubleValue (100.0));

	this->flowRandomProbability = atof(MyConfig::instance().getValue("FlowRandomProbability").c_str());
	this->flowRandomInterval = atof(MyConfig::instance().getValue("FlowRandomInterval").c_str());
}

MyNode::~MyNode() {
	// close file pointers
#ifdef DEBUG_FLOW_OUT
	this->flowOut.close();
	this->srcRtOut.close();
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

	pair<int, FlowCheckRecvTable*> p;
	BOOST_FOREACH(p, this->flowCheckRecvMap){
		delete p.second;
	}
	pair<int, FlowAcceptReplyRecvTable*> p2;
	BOOST_FOREACH(p2, this->flowAccRepRecvMap){
		delete p2.second;
	}

	pair<Flow, MyApplication*> p3;
	BOOST_FOREACH(p3, this->myAppMap){
		delete p3.second;
	}
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

double MyNode::getMyUnallocatedBandwidth(NeighborEntry* entry) {
	double totalAllocBW = 0.0;
	for(Flow flow : entry->getFlowList()){
		FlowEntry* flowEntry = this->flowTable->getFlowEntry(flow);
		totalAllocBW += flowEntry->getAllocatedBandwidth(entry->getNodeId());
	}

	double maxBW = MyConfig::instance().getMaxBandwidth();
	return maxBW - totalAllocBW;
}

double MyNode::getMyAvailableBandwidth(NeighborEntry* entry) {
	double totalRtBW = 0.0;
	double maxBW = MyConfig::instance().getMaxBandwidth();
	double availableBW;
	for(Flow aFlow : entry->getFlowList()){
		FlowEntry* flowEntry = this->flowTable->getFlowEntry(aFlow);
		if(flowEntry->isRouteSearching() == false && flowEntry->isActive() == true){
			totalRtBW += flowEntry->getRealTimeBandwidth(entry->getNodeId());
			//totalRtBW += flowEntry->getAvgRealTimeBandwidth(entry->getNodeId());
		}
	}

	availableBW = maxBW - totalRtBW;
	if(availableBW > 0) return availableBW;
	else return 0.0;
}

double MyNode::getMyAvailableBandwidthWithoutFlow(NeighborEntry* entry, Flow flow) {
	double totalRtBW = 0.0;
	double maxBW = MyConfig::instance().getMaxBandwidth();
	double availableBW;
	for(Flow aFlow : entry->getFlowList()){
		if(flow == aFlow) continue;

		FlowEntry* flowEntry = this->flowTable->getFlowEntry(aFlow);
		totalRtBW += flowEntry->getRealTimeBandwidth(entry->getNodeId());
		//totalRtBW += flowEntry->getAvgRealTimeBandwidth(entry->getNodeId());
	}

	availableBW = maxBW - totalRtBW;
	if(availableBW > 0) return availableBW;
	else return 0.0;
}

void MyNode::accumulateLinkQuality(LinkQuality* lq, NeighborEntry* entry) {
	//double availableBW = getMyUnallocatedBandwidth(entry);
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

void MyNode::accumulateLinkQualityWithoutFlow(LinkQuality* lq, NeighborEntry* entry, Flow flow) {
	double availableBW = getMyAvailableBandwidthWithoutFlow(entry, flow);

	// Debug
//	NS_LOG_UNCOND("[Node " << this->nodeId << "] accumulateLinkQualityWithoutFlow, t=" << Simulator::Now().GetMilliSeconds());
//	NS_LOG_UNCOND(" - available BW = " << availableBW);
//	this->nodeOut << "[Node " << this->nodeId << "] accumulateLinkQualityWithoutFlow, t=" << Simulator::Now().GetMilliSeconds() << "\n";
//	this->nodeOut << " - available BW = " << availableBW << "\n";

	if(lq->getBandwidth() == 0.0 || lq->getBandwidth() > availableBW){
		lq->setBandwidth(availableBW);
	}
	lq->setDelay(lq->getDelay() + entry->getAverageDelay());
	lq->setJitter(lq->getJitter() + entry->getAverageJitter());
	double lossRate = lq->getLossRate();
	lossRate *= 1.0 - ((1.0 - lq->getLossRate()) * (1.0 - entry->getLossRate()));
	lq->setLossRate(lossRate);
}

void MyNode::distractLinkQuality(LinkQuality* lq, NeighborEntry* entry){
	//double availableBW = getMyAvailableBandwidth(entry);

	// TODO: implement (not yet used)
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

/**
 * Send a routing packet (string) to the target Ipv4Address.
 * Note that the target must be directly connected with this node.
 * (e.g. point-to-point)
 */
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

	string ctrlOutline = this->ctrlFlowTable->getControlOutput(ss.str());
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

	// CtrlPortInfo
	if(this->numOfCtrlPorts < this->ctrlFlowTable->getNumberOfCtrlPorts()){
		stringstream ssfi;
		ssfi << "ovinfo_" << node->GetId() << ".txt";
		this->ctrlInfoOut.open(ssfi.str().c_str(), ofstream::out);
		this->ctrlInfoOut << this->ctrlFlowTable->getControlFlowInfo();
		this->ctrlInfoOut.close();
		this->numOfCtrlPorts = this->ctrlFlowTable->getNumberOfCtrlPorts();
	}
#endif
}

void MyNode::writeSrcRtLog() {
#ifdef DEBUG_SRC_RT_OUT
	stringstream ss;
	ss << Simulator::Now().GetSeconds();
	ss << "\t" << this->srcRtDscvCount <<
			"\t"<< this->srcRtDscvFailCount << "\n";
	this->srcRtOut << ss.str();

	// Reset route discovery counter variables.
	this->srcRtDscvCount = 0;
	this->srcRtDscvFailCount = 0;
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
 * Handle all flows as a source node.
 */
void MyNode::_checkFlowQoS(Time interval){
	vector<FlowEntry*> flowEntries = flowTable->getAllFlowEntries();
	if(flowEntries.size() > 0){
#ifdef DEBUG_PRINT
		NS_LOG_UNCOND("[Node "<< this->nodeId <<"] _checkFlowQoS (t=" << Simulator::Now().GetSeconds() << ")");
#endif
#ifdef DEBUG_NODE_OUT
		this->nodeOut << "[Node "<< this->nodeId <<"] _checkFlowQoS (t=" << Simulator::Now().GetSeconds() << ")\n";
#endif
	}

	for(FlowEntry* flowEntry : flowEntries) {
#ifdef DEBUG_PRINT
		NS_LOG_UNCOND(" -flow: " << flowEntry->getFlow().toString());
#endif
#ifdef DEBUG_NODE_OUT
		this->nodeOut << " -flow: " << flowEntry->getFlow().toString() << "\n";
#endif
		if( !flowEntry->isActive() || flowEntry->isRouteSearching() ) {
#ifdef DEBUG_PRINT
		NS_LOG_UNCOND("   - inactive");
#endif
#ifdef DEBUG_NODE_OUT
		this->nodeOut << "   - inactive\n";
#endif
			continue;
		}

		Time now = Simulator::Now();
		if(this->nodeId == flowEntry->getFlow().getSrc()){
			// the flow starts from myself.
			uint32_t nextHop = routeTable->getNextHop(flowEntry->getFlow());
			if(nextHop == NODEID_NOT_FOUND){
#ifdef DEBUG_PRINT
				NS_LOG_UNCOND("   - no nextHop!! Route Re-discovery needed!");
#endif
#ifdef DEBUG_NODE_OUT
				this->nodeOut << "   - no nextHop!! Route Re-discovery needed!" << "\n";
#endif
				continue;
			}

			// find nextHop entry to obtain QoS metrics
			NeighborEntry* nextHopEntry = this->ncTable->get(nextHop);
			if(!nextHopEntry) continue;

			// Handle the one-hop flow.
			if(nextHop == flowEntry->getFlow().getDst()){
				// Check QoS violation on the link to the dst.
				// If yes, increment flow unsatisfactory count.
				// If unsatisfactory count exceeds threshold, perform local repair
				// based on schemes, and reset flow unsatisfactory count.
				// Otherwise, reset flow unsatisfactory count.

				double availableBW = getMyUnallocatedBandwidth(nextHopEntry) + flowEntry->getAvgRealTimeBandwidth();
				if(availableBW < 0.0) availableBW = 0.0;

				LinkQuality lq(
						availableBW,
						nextHopEntry->getAverageDelay(),
						nextHopEntry->getAverageJitter(),
						nextHopEntry->getLossRate());

				if(QoSRequirement::isSatisfactory(flowEntry->getQosReq(), lq)){
					flowEntry->setUnsatisfactoryCount(0);
				} else {
					int flowUnsatisfactoryCount = flowEntry->getUnsatisfactoryCount();
					if(flowUnsatisfactoryCount < MyConfig::instance().getFlowUnsatisThreshold()){
						flowEntry->setUnsatisfactoryCount(++flowUnsatisfactoryCount);
					} else {
						// QoS violation (exceeding threshold)
						if(this->scheme == BASELINE) {
							// Broadcast ARREQ with the flow and QoSReq.
							// Schedule the setupRoute function.
						} else if(this->scheme == SCHEME_LOCAL_REPAIR_AVG_BW) {
							// Find detour node candidates: neighbors who know the next-hop.
							vector<NeighborEntry*> detours = this->ncTable->getDetourNodes(nextHopEntry->getNodeId(), flowEntry->getSrcRoute());
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
									NS_LOG_UNCOND("   - detour node selected as: " << selected->getNodeId() << ", avgOccBw=" << this->flowTable->getOccupiedBandwidth(selected->getNodeId()));
#endif
#ifdef DEBUG_NODE_OUT
									this->nodeOut << "   - detour node selected as: " << selected->getNodeId() << ", avgOccBw=" << this->flowTable->getOccupiedBandwidth(selected->getNodeId()) << "\n";
#endif
									vector<uint32_t> trace = flowEntry->getSrcRoute();
									int myPosInTrace = 0;
									for(vector<int>::size_type i=0; i< trace.size(); i++){
										if(this->nodeId == trace[i]) myPosInTrace = i;
									}
									trace.insert(trace.begin()+1+myPosInTrace, selected->getNodeId());

									// Perform local repair as a source.
									performLocalRepair(
											nextHopEntry->getNodeId(), selected->getNodeId(), this->nodeId,
											flowEntry->getFlow(), trace,
											flowEntry->getQosReq(), lq);
									MyStatistics::instance().incrementLocalRepairCount(flowEntry->getFlow(), this->nodeId);
								}
							}
						}

						flowEntry->setUnsatisfactoryCount(0);
					}
				}
			} else {
				// Multi-hop path.
				// Make a default link quality value. (will be accumulated in the receiver-side.)
				LinkQuality lq;
				lq.setBandwidth(MyConfig::instance().getMaxBandwidth());
				// Accumulate link quality of the src~nextHop link.
				//accumulateLinkQuality(&lq, nextHopEntry);
				//double availableBW = MyConfig::instance().getMaxBandwidth() - flowEntry->getAvgRealTimeBandwidth();
				//lq.setBandwidth(availableBW);

				// send PathProbe to the nextHop
				Route* route = this->routeTable->getRoute(flowEntry->getFlow());
				PathProbe probe(flowEntry->getFlow(), route->getHopCount(), flowEntry->getAppReqSeqNo(), flowEntry->getQosReq(), lq);
				probe.setTrace(flowEntry->getSrcRoute());
				sendRoutingPacket(nextHopEntry->getIp(), probe.serialize());
			}
		}
	}

	// Periodically check flows by scheduling the next event.
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
	sendPacketWithJitter(MicroSeconds(jitter), helloSocket, pkt);
	delete hello;

	// MyNode stat
	Flow flow(this->nodeId, 5555, NODEID_BROADCAST, MyConfig::instance().getPortByName("HelloPort"), FlowType::UDP);
	PacketInfo pktInfo((long)(Simulator::Now().GetMilliSeconds()), flow, 0, sizeof(Hello));
	handlePacketInfo(NODEID_BROADCAST, pktInfo);

	// Schedule the next Hello broadcast.
	Simulator::Schedule (interval, &MyNode::checkBroadcastHello, this, interval);
}

void MyNode::checkFlowRandomness(MyNode* myNode, Time interval) {
	myNode->_checkFlowRandomness(interval);
}

/**
 * Periodically check the randomness value of each flow
 * as a source node.
 * If a pseudo random number meets the requirement,
 * then set the flow as "Randomness Activated".
 * Once the randomness of a flow is activated, the flow
 * will generate some more traffic (e.g. 20%) during a
 * specified time interval (e.g. 5 seconds).
 */
void MyNode::_checkFlowRandomness(Time interval) {
	vector<FlowEntry*> flowEntries = flowTable->getAllFlowEntries();
	for(FlowEntry* flowEntry : flowEntries) {
		if(flowEntry->getFlow().getSrc() != this->nodeId) continue;

		if(flowEntry->isRandomnessActivated()){
			if(flowEntry->getRandomnessTime() + this->flowRandomInterval < Simulator::Now().GetSeconds()){
#ifdef DEBUG_PRINT
				NS_LOG_UNCOND("[Node " << this->nodeId << "] _checkFlowRandomness\n - flow randomness stopped (" << flowEntry->getFlow().toString() << "), t=" << Simulator::Now().GetMilliSeconds());
#endif
#ifdef DEBUG_NODE_OUT
				this->nodeOut << "[Node " << this->nodeId << "] _checkFlowRandomness\n - flow randomness stopped (" << flowEntry->getFlow().toString() << "), t=" << Simulator::Now().GetMilliSeconds() << "\n";
#endif
				flowEntry->setRandomnessActivated(false);
			}
			continue;
		}

		int flowNormalIntervalAtStart = atoi(MyConfig::instance().getValue("FlowNormalIntervalAtStart").c_str());
		if(MilliSeconds(flowEntry->getStartTime()) + Seconds(flowNormalIntervalAtStart) > Simulator::Now()){
			this->nodeOut << " - flowEntry.getStartTime = " << flowEntry->getStartTime() << "\n";
			this->nodeOut << " - flowNormalInterval = " << flowNormalIntervalAtStart << "\n";
			this->nodeOut << " - now = " << Simulator::Now().GetSeconds() << "\n";
			continue;
		}

//		//#######################################################################
//		// Debug: test code
//		if(Simulator::Now().GetSeconds() > 45){
//			if(flowEntry->getFlow().getDst() == 8){
//				if(!flowEntry->isRandomnessActivated()){
//					flowEntry->setRandomnessActivated(true);
//					flowEntry->setRandomnessTime(Simulator::Now().GetSeconds());
//#ifdef DEBUG_PRINT
//					NS_LOG_UNCOND("[Node " << this->nodeId << "] _checkFlowRandomness\n - flow randomness start (" << flowEntry->getFlow().toString() << ")");
//#endif
//#ifdef DEBUG_NODE_OUT
//					this->nodeOut << "[Node " << this->nodeId << "] _checkFlowRandomness\n - flow randomness start (" << flowEntry->getFlow().toString() << ")" << "\n";
//#endif
//				}
//			}
//		}
//		//#######################################################################

		// Randomness selection by probability
		uint32_t rnd = this->flowRandom->GetInteger();
		if(rnd <= (uint32_t)(100.0 * this->flowRandomProbability)){
#ifdef DEBUG_PRINT
			NS_LOG_UNCOND("[Node " << this->nodeId << "] _checkFlowRandomness\n - flow randomness start (" << flowEntry->getFlow().toString() << ") t=" << Simulator::Now().GetMilliSeconds());
#endif
#ifdef DEBUG_NODE_OUT
			this->nodeOut << "[Node " << this->nodeId << "] _checkFlowRandomness\n - flow randomness start (" << flowEntry->getFlow().toString() << ") t=" << Simulator::Now().GetMilliSeconds() << "\n";
#endif
			flowEntry->setRandomnessActivated(true);
			flowEntry->setRandomnessTime(Simulator::Now().GetSeconds());
		}
	}

	// Schedule the next flowRandomness check.
	Simulator::Schedule (interval, &MyNode::checkFlowRandomness, this, interval);
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

	if(this->scheme == SCHEME_LOCAL_REPAIR_AVG_BW){
		// average bandwidth occupation of all neighbor link
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

void MyNode::setupRoute(MyNode* myNode, Flow flow, int seqNo) {
	myNode->_setupRoute(flow, seqNo);
}

void MyNode::_setupRoute(Flow flow, int seqNo) {
#ifdef DEBUG_PRINT
	NS_LOG_UNCOND("[Node "<< this->nodeId <<"] setupRoute, flow: " << flow.toString() << "(t=" << Simulator::Now().GetSeconds() << ")");
#endif
#ifdef DEBUG_NODE_OUT
	this->nodeOut << "[Node "<< this->nodeId <<"] setupRoute, flow: " << flow.toString() << "(t=" << Simulator::Now().GetSeconds() << ")\n";
#endif
	ARREPRecvEntry* arrepEntry = this->arrepRecvTable->getEntry(flow);

	if(!arrepEntry || arrepEntry->getArrepList(seqNo).size() == 0){
#ifdef DEBUG_PRINT
		NS_LOG_UNCOND(" - No available route found!!");
#endif
#ifdef DEBUG_NODE_OUT
		this->nodeOut << " - No available route found!!" << "\n";
#endif
		this->srcRtDscvFailCount++;
		return;
	}

	ARREP optimalARREP = getOptimalARREP(arrepEntry->getArrepList(seqNo));
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
		this->flowTable->setQoSReqAsSource(rs.getFlow(), route->getNextHop(), rs.getQosReq());
		this->flowTable->getFlowEntry(rs.getFlow())->setActive(true);
		this->flowTable->getFlowEntry(rs.getFlow())->setRouteSearching(false);
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
			this->flowTable->setQoSReqAsSource(rs.getFlow(), route->getNextHop(), rs.getQosReq());
			this->flowTable->getFlowEntry(rs.getFlow())->setActive(true);
			this->flowTable->getFlowEntry(rs.getFlow())->setRouteSearching(false);
			this->ncTable->get(nextHop)->addFlow(rs.getFlow());
			//this->flowTable->setAllocatedBW(rs.getFlow(), outgoingIface, rs.getQosReq().getBandwidth());

			// 170822, set hop-by-hop QoSReq for local repair
			// calculate hop-by-hop QoSReq.
			int h = rs.getTrace().size();
			double aDelay = rs.getQosReq().getDelay() - rs.getEndToEndQuality()->getDelay();
			double deltaDelay = aDelay / (double)h;
			double hopDelayReq = (double)(this->ncTable->get(nextHop)->getDelay()) + deltaDelay;

			double aJitter = rs.getQosReq().getJitter() - rs.getEndToEndQuality()->getJitter();
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

			this->flowTable->setHopQoSReqAsSource(rs.getFlow(), route->getNextHop(), hopQosReq);
			this->flowTable->getFlowEntry(rs.getFlow())->setSrcRoute(rs.getTrace());
			sendRoutingPacket(this->ncTable->get(nextHop)->getIp(), rs.serialize());
		}
	}

	// schedule data packets based on FlowReuqest
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

		// Send a packet to the nexthop.
		Ptr<MyNS3Packet> myPkt = CreateObject<MyNS3Packet>(flowReq.getFlow().getSrc(), flowReq.getFlow().getSrcPort(), flowReq.getFlow().getDst(), flowReq.getFlow().getDstPort(), msg);
		double flowNormalSendingRate = atof(MyConfig::instance().getValue("FlowNormalSendingRate").c_str());
		Time nextTime = Seconds(1.0/((double)flowReq.getSendingRate() * flowNormalSendingRate));
		FlowEntry* flowEntry = this->flowTable->getFlowEntry(flowReq.getFlow());
		if(flowEntry){
			myPkt->setSeqNo(flowEntry->getPacketSeqNo());
			flowEntry->incrementPacketSeNo();
			if(flowEntry->isRandomnessActivated()){
				flowNormalSendingRate = 1.0;
			}
			QoSRequirement flowQosReq = flowEntry->getQosReq();
			flowQosReq.setBandwidth(((double)flowReq.getSendingRate() * flowNormalSendingRate * 8 * (double)flowReq.getPktSize()));
			flowEntry->setQosReq(flowQosReq);
			nextTime = Seconds(1.0/((double)flowReq.getSendingRate() * flowNormalSendingRate));
		}
		sendMyPacket(this->ncTable->get(route->getNextHop())->getIp(), myPkt, flowReq.getFlow().getType(), flowReq.getPktSize());

		// Schedule the next packet
		Simulator::Schedule(nextTime, &MyNode::schedulePacketsFromFlowRequest, this, flowReq, msg);
	} else {
		// find ncTable to check if dst is my neighbor.
		NeighborEntry* entry = this->ncTable->get(flowReq.getFlow().getDst());
		if(entry){
			// One-hop. Send a packet to the neighbor.
			Ptr<MyNS3Packet> myPkt = CreateObject<MyNS3Packet>(flowReq.getFlow().getSrc(), flowReq.getFlow().getSrcPort(), flowReq.getFlow().getDst(), flowReq.getFlow().getDstPort(), msg);
			double flowNormalSendingRate = atof(MyConfig::instance().getValue("FlowNormalSendingRate").c_str());
			Time nextTime = Seconds(1.0/((double)flowReq.getSendingRate() * flowNormalSendingRate));
			FlowEntry* flowEntry = this->flowTable->getFlowEntry(flowReq.getFlow());
			if(flowEntry){
				myPkt->setSeqNo(flowEntry->getPacketSeqNo());
				flowEntry->incrementPacketSeNo();
				if(flowEntry->isRandomnessActivated()){
					flowNormalSendingRate = 1.0;
				}
				nextTime = Seconds(1.0/((double)flowReq.getSendingRate() * flowNormalSendingRate));
			}
			sendMyPacket(entry->getIp(), myPkt, flowReq.getFlow().getType(), flowReq.getPktSize());

			// schedule the next packet
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

			// 180313, Increment source's route discovery count
			this->srcRtDscvCount++;
			MyStatistics::instance().incrementRtDiscoveryCount(flowReq.getFlow());
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

	bool repaired = false;
	FlowCheckRecvTable* table = this->flowCheckRecvMap[seqNo];
	uint32_t optimalDetour = table->getOptimalDetourNode(1);
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
		MyStatistics::instance().incrementLocalRepairCount(table->getFlow(), this->nodeId);
		repaired = true;
	} else {
		// Optimal node not found
#ifdef DEBUG_PRINT
		NS_LOG_UNCOND(" - optimal detour node not found!!");
#endif
#ifdef DEBUG_NODE_OUT
		this->nodeOut << " - optimal detour node not found!!" << "\n";
#endif
		repaired = false;
	}

	// Send LocalRepairReply to the previous next-hop (sender of LocalRepairRequest).
	NeighborEntry* prevHopEntry = this->ncTable->get(table->getPrevNextHop());
	LocalRepairReply lrrep(table->getFlow(), repaired);
	sendRoutingPacket(prevHopEntry->getIp(), lrrep.serialize());

	// Remove FlowCheck-replies which maps to the seqNo.
	delete this->flowCheckRecvMap[seqNo];
	this->flowCheckRecvMap.erase(seqNo);
}

void MyNode::selectNodeFromFlowAcceptReply(MyNode* myNode, int seqNo) {
	myNode->_selectNodeFromFlowAcceptReply(seqNo);
}

void MyNode::_selectNodeFromFlowAcceptReply(int seqNo) {
#ifdef DEBUG_PRINT
	NS_LOG_UNCOND("[Node " << this->nodeId << "] _selectNodeFromFlowAcceptReply (seq: " << seqNo << ") t=" << Simulator::Now().GetMilliSeconds());
#endif
#ifdef DEBUG_NODE_OUT
	this->nodeOut << "[Node " << this->nodeId << "] _selectNodeFromFlowAcceptReply (seq: " << seqNo << ") t=" << Simulator::Now().GetMilliSeconds() <<"\n";
#endif

	bool repaired = false;
	FlowAcceptReplyRecvTable* table = this->flowAccRepRecvMap[seqNo];
	FlowAcceptReply* optimalReply = table->getOptimalFlowAcceptReply();
	if(optimalReply){
		stringstream ssdt;
		for(uint32_t temp : optimalReply->getDetourIDs()){
			ssdt << " " << temp;
		}
#ifdef DEBUG_PRINT
		NS_LOG_UNCOND(" - optimal detour subpath: " << ssdt.str());
#endif
#ifdef DEBUG_NODE_OUT
		this->nodeOut << " - optimal detour subpath: " << ssdt.str() << "\n";
#endif
		vector<uint32_t> subpath = optimalReply->getDetourIDs();
		vector<uint32_t> newSrcRoute = table->getSrcRoute();
		int myPosInTrace = 0;
		for(vector<int>::size_type i=0; i< newSrcRoute.size(); i++){
			if(this->nodeId == newSrcRoute[i]) myPosInTrace = i;
		}
		newSrcRoute.insert(newSrcRoute.begin()+1+myPosInTrace, subpath.begin(), subpath.end());

		// Perform local repair with the selected optimal nodeID.
		performLocalRepair(
				table->getPrevNextHop(),
				subpath[0],
				table->getNextHopToSrc(),
				table->getFlow(),
				newSrcRoute,
				table->getQosReq(),
				table->getEndToEndQuality());
		MyStatistics::instance().incrementLocalRepairCount(table->getFlow(), this->nodeId);
		repaired = true;
	} else {
		// Optimal detour path not found
#ifdef DEBUG_PRINT
		NS_LOG_UNCOND(" - optimal detour path(node) not found!!");
#endif
#ifdef DEBUG_NODE_OUT
		this->nodeOut << " - optimal detour path(node) not found!!" << "\n";
#endif
		repaired = false;
	}

	// Send LocalRepairReply to the previous next-hop (sender of LocalRepairRequest).
	NeighborEntry* prevHopEntry = this->ncTable->get(table->getPrevNextHop());
	LocalRepairReply lrrep(table->getFlow(), repaired);
	sendRoutingPacket(prevHopEntry->getIp(), lrrep.serialize());

	// Remove FlowCheck-replies which maps to the seqNo.
	delete this->flowAccRepRecvMap[seqNo];
	this->flowAccRepRecvMap.erase(seqNo);
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

void MyNode::checkMyApplicationStatistics(MyNode* myNode, Flow flow) {
	myNode->_checkMyApplicationStatistics(flow);
}

void MyNode::_checkMyApplicationStatistics(Flow flow) {
#ifdef DEBUG_PRINT
	NS_LOG_UNCOND("[Node " << this->nodeId << "] _checkMyApplicationStatistics " << flow.toString());
#endif
#ifdef DEBUG_NODE_OUT
	this->nodeOut << "[Node " << this->nodeId << "] _checkMyApplicationStatistics " << flow.toString() << "\n";
#endif

	MyApplication* myApp = this->myAppMap[flow];
	myApp->updateStatistics();
	//myApp->consumeDataPackets();

	// If the flow lifetime still remains,
	// reschedule the next checkpoint of update.
	if(myApp->getFlowReq().getEndTime() > Simulator::Now()){
		Time nextTime = Seconds(1.0);
		Simulator::Schedule(nextTime, &MyNode::checkMyApplicationStatistics, this, flow);
	}
}


void MyNode::broadcastARREQ(Flow flow, int seqNo, FlowRequest flowReq){
	// debug
#ifdef DEBUG_PRINT
	NS_LOG_UNCOND("[Node " << this->nodeId << "][broadcastARREQ] flow: " << flow.toString() << ", seqNo=" << seqNo);
#endif
#ifdef DEBUG_NODE_OUT
	this->nodeOut << "[Node " << this->nodeId << "][broadcastARREQ] flow: " << flow.toString() << ", seqNo=" << seqNo << "\n";
#endif

	FlowEntry* flowEntry = this->flowTable->getFlowEntry(flow);
	if(flowEntry){
		if(!flowEntry->isRandomnessActivated()){
			double flowNormalSendingRate = atof(MyConfig::instance().getValue("FlowNormalSendingRate").c_str());
			QoSRequirement flowQosReq = flowReq.getQosReq();
			flowQosReq.setBandwidth((double)flowReq.getPktSize() * (double)flowReq.getSendingRate() * 8 * flowNormalSendingRate);
			flowReq.setQosReq(flowQosReq);
		}
	} else {
		double flowNormalSendingRate = atof(MyConfig::instance().getValue("FlowNormalSendingRate").c_str());
		QoSRequirement flowQosReq = flowReq.getQosReq();
		flowQosReq.setBandwidth((double)flowReq.getPktSize() * (double)flowReq.getSendingRate() * 8 * flowNormalSendingRate);
		flowReq.setQosReq(flowQosReq);
	}

	int floodingTTL = atoi(MyConfig::instance().getValue("RouteDiscoveryTTL").c_str());
	ARREQ arreq(flow, seqNo, floodingTTL, flowReq.getQosReq());
	arreq.addTrace(this->nodeId);
	sendRoutingPacket(Ipv4Address::GetBroadcast(), arreq.serialize());

	this->arrepRecvTable->addEntry(flow, seqNo, flowReq);
	Time rtSetupInterval = Seconds(1.0);
	Simulator::Schedule(rtSetupInterval, &MyNode::setupRoute, this, flow, seqNo);
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
			// Add a new route to the routeTable, and add a new flowEntry to the flowTable.
			Route* route = new Route(flowReq.getFlow(), ncEntry->getNodeId(), 1);
			this->routeTable->addRoute(route);
			this->flowTable->setQoSReqAsSource(flowReq.getFlow(), ncEntry->getNodeId(), flowReq.getQosReq());
			vector<uint32_t> srcRoute;
			srcRoute.push_back(this->nodeId);
			srcRoute.push_back(ncEntry->getNodeId());
			this->flowTable->getFlowEntry(flowReq.getFlow())->setSrcRoute(srcRoute);

			double availableBW = getMyUnallocatedBandwidth(ncEntry) + this->flowTable->getFlowEntry(flowReq.getFlow())->getAvgRealTimeBandwidth();
			if(availableBW < 0.0) availableBW = 0.0;

			LinkQuality lq(
					availableBW,
					ncEntry->getAverageDelay(),
					ncEntry->getAverageJitter(),
					ncEntry->getLossRate());
			RouteSetup rs;
			rs.setFlow(flowReq.getFlow());
			rs.setSeqNo(0);
			rs.setQosReq(flowReq.getQosReq());
			rs.setEndToEndQuality(lq);
			rs.setTrace(srcRoute);
			sendRoutingPacket(ncEntry->getIp(), rs.serialize());

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

			// 180313, Increment route discovery count.
			this->srcRtDscvCount++;
			MyStatistics::instance().incrementRtDiscoveryCount(flow);
		}
	}
}

/**
 * Performs local repair.
 * @param prevNextHop A previous (QoS-violated) nextHop
 * @param newNextHop A new nextHop to destination
 * @param nextHopToSrc A reverse route's nextHop to source (if src, set this as src)
 * @param flow A flow to repair
 * @param srcRoute A new source route trace including newNextHop
 * @param qosReq A QoS requirement
 * @param endToEndQuality Link quality from source to this node
 */
void MyNode::performLocalRepair(uint32_t prevNextHop, uint32_t newNextHop,
		uint32_t nextHopToSrc, Flow flow, vector<uint32_t> srcRoute,
		QoSRequirement qosReq, LinkQuality endToEndQuality) {
#ifdef DEBUG_PRINT
	NS_LOG_UNCOND("[Node "<< this->nodeId << "] performLocalRepair, t=" << Simulator::Now().GetMilliSeconds());
#endif
#ifdef DEBUG_NODE_OUT
	this->nodeOut << "[Node "<< this->nodeId << "] performLocalRepair, t=" << Simulator::Now().GetMilliSeconds() << "\n";
#endif

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
	flowEntry->deleteFlowStat(prevNextHop);
	flowEntry->setFwdNodeId(newNextHop);
	flowEntry->setAllocatedBandwidth(prevNextHop, 0.0);
	flowEntry->addFlowStat(newNextHop);
	flowEntry->setAllocatedBandwidth(newNextHop, qosReq.getBandwidth());
	flowEntry->setSrcRoute(srcRoute);

	// Update ncTable for link quality change.
	this->ncTable->get(prevNextHop)->removeFlow(flow);
	this->ncTable->get(newNextHop)->addFlow(flow);

	if(nextHopToSrc == NODEID_NOT_FOUND || nextHopToSrc == this->nodeId){
		flowEntry->setSrcRoute(srcRoute);
	} else {
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
	}

	// TODO: Send [    ] to the previous next-hop (prevNextHop)
	// to cancel the previous route. (delete route, delete flow)
}

void MyNode::performLocalRepairMultiHop(uint32_t prevNextHop, vector<uint32_t> newNextHopIds,
		uint32_t nextHopToSrc, Flow flow, vector<uint32_t> srcRoute,
		QoSRequirement qosReq, LinkQuality endToEndQuality) {
#ifdef DEBUG_PRINT
	NS_LOG_UNCOND("[Node "<< this->nodeId << "] performLocalRepairMultiHop");
#endif
#ifdef DEBUG_NODE_OUT
	this->nodeOut << "[Node "<< this->nodeId << "] performLocalRepairMultiHop" << "\n";
#endif

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
	sendRoutingPacket(this->ncTable->get(newNextHopIds[0])->getIp(), rs.serialize());

	// Change the related route of my routing table.
	Route* route = this->routeTable->getRoute(flow);
	route->setHopCount(route->getHopCount() + 1);
	route->setNextHop(newNextHopIds[0]);

	// Update flowTable with the new nextHop.
	FlowEntry* flowEntry = this->flowTable->getFlowEntry(flow);
	flowEntry->setFwdNodeId(newNextHopIds[0]);
	flowEntry->setAllocatedBandwidth(prevNextHop, 0.0);
	flowEntry->addFlowStat(newNextHopIds[0]);
	flowEntry->setAllocatedBandwidth(newNextHopIds[0], qosReq.getBandwidth());

	// Update ncTable for link quality change.
	this->ncTable->get(prevNextHop)->removeFlow(flow);
	this->ncTable->get(newNextHopIds[0])->addFlow(flow);

	if(nextHopToSrc == NODEID_NOT_FOUND || nextHopToSrc == this->nodeId){
		flowEntry->setSrcRoute(srcRoute);
	} else {
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
	}

	// TODO: Send [    ] to the previous next-hop (prevNextHop)
	// to cancel the previous route. (delete route, delete flow)
}

void MyNode::handlePacketInfo(int senderNodeId, PacketInfo pktInfo) {
	if(isControlPort(pktInfo.getFlow().getDstPort())){
		bool isTx = false;
		if(this->nodeId == pktInfo.getFlow().getSrc()){
			isTx = true;
		}
		this->ctrlFlowTable->add(isTx, &pktInfo);
	} else {
		this->flowTable->addToFlowEntry(senderNodeId, pktInfo);
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
#ifdef DEBUG_PRINT
			NS_LOG_UNCOND(" -ignoring an ARREQ with a routing loop");
#endif
#ifdef DEBUG_NODE_OUT
			this->nodeOut << " -ignoring an ARREQ with a routing loop" << "\n";
#endif
			return;
		}
	}

	// If ARREQ with the same seqNo arrives more than repeat threshold, discard it.
	this->arrepRecvTable->addEntry(arreq.getFlow(), arreq.getSeqNo()); // no flowReq for intermediate nodes
/*	ARREPRecvEntry* arrepEntry = this->arrepRecvTable->getEntry(arreq.getFlow());
	if(arrepEntry->getDupArreqCount() >= 2){
#ifdef DEBUG_PRINT
		NS_LOG_UNCOND(" -ignoring a duplicate ARREQ exceeding threshold");
#endif
#ifdef DEBUG_NODE_OUT
		this->nodeOut << " -ignoring a duplicate ARREQ exceeding threshold" << "\n";
#endif
		return;
	}*/

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
#endif
#ifdef DEBUG_NODE_OUT
	this->nodeOut << "  -bw: " << qosReq.getBandwidth() << " vs " << arreq.getLinkQuality()->getBandwidth() << "\n";
	this->nodeOut << "  -delay: " << qosReq.getDelay() << " vs " << arreq.getLinkQuality()->getDelay() << "\n";
	this->nodeOut << "  -jiter: " << qosReq.getJitter() << " vs " << arreq.getLinkQuality()->getJitter() << "\n";
	this->nodeOut << "  -loss: " << qosReq.getLossRate() << " vs " << arreq.getLinkQuality()->getLossRate() << "\n";
#endif

	if(!QoSRequirement::isSatisfactory(qosReq, arreq.getLinkQuality())){
#ifdef DEBUG_PRINT
		NS_LOG_UNCOND(" -QoSReq is not satisfactory... drop it.");
#endif
#ifdef DEBUG_NODE_OUT
		this->nodeOut << " -QoSReq is not satisfactory... drop it." << "\n";
#endif
		return;
	}

#ifdef DEBUG_PRINT
	NS_LOG_UNCOND(" -QoSReq is satisfactory. ");
#endif
#ifdef DEBUG_NODE_OUT
	this->nodeOut << " -QoSReq is satisfactory. " << "\n";
#endif

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
		NS_LOG_UNCOND(" - add ARREP to ARREPRecvTable, flow: " << arrep.getFlow().toString());
#endif
#ifdef DEBUG_NODE_OUT
		this->nodeOut << " - add ARREP to ARREPRecvTable, flow: " << arrep.getFlow().toString() << "\n";
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
		this->flowTable->getFlowEntry(arerr.getFlow())->setRouteSearching(true);

		// 180313, write ARERR log by source
		//this->srcRtDscvCount++;

	} else {
		// intermediate of the flow.
		// relay ARERR to the source.
#ifdef DEBUG_PRINT
		NS_LOG_UNCOND(" - relay ARERR to src...");
#endif
#ifdef DEBUG_NODE_OUT
		this->nodeOut << " - relay ARERR to src..." << "\n";
#endif

		this->flowTable->getFlowEntry(arerr.getFlow())->setRouteSearching(true);

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
		this->nodeOut << "@@@@ reverse flow = " << reverseFlow.toString() << "\n";
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
	NS_LOG_UNCOND("[Node "<< this->nodeId <<"] handleRouteSetup (from "<< clientNodeId << ") t=" << Simulator::Now().GetMilliSeconds());
	NS_LOG_UNCOND(" - " << str);
#endif
#ifdef DEBUG_NODE_OUT
	this->nodeOut << "[Node "<< this->nodeId <<"] handleRouteSetup (from "<< clientNodeId << ") t=" << Simulator::Now().GetMilliSeconds() << "\n";
	this->nodeOut << " - " << str << "\n";
#endif

	int myPosInTrace = rs.getPositionInTrace(this->node);

	// Handling error cases: not found in the trace
	if(myPosInTrace < 0) return;

	// Add a reverse route.
	Flow reverseFlow(rs.getFlow().getDst(),
			rs.getFlow().getDstPort(),
			rs.getFlow().getSrc(),
			rs.getFlow().getSrcPort(),
			rs.getFlow().getType());
	Route* reverseRoute = new Route(reverseFlow, clientNodeId, myPosInTrace);
	this->routeTable->addRoute(reverseRoute);

#ifdef DEBUG_PRINT
	// debug
	NS_LOG_UNCOND("  [RT] added a reverse route: " << reverseRoute->toString());
#endif
#ifdef DEBUG_NODE_OUT
	this->nodeOut << "  [RT] added a reverse route: " << reverseRoute->toString() << "\n";
#endif

	if(this->nodeId == rs.getFlow().getDst()){
		// This is the destination of the route.
		this->flowTable->setQoSReqAsDestination(rs.getFlow(), clientNodeId, rs.getQosReq());
		this->flowTable->getFlowEntry(rs.getFlow())->setSrcRoute(rs.getTrace());
		this->flowTable->getFlowEntry(rs.getFlow())->setActive(true);
		this->flowTable->getFlowEntry(rs.getFlow())->setRouteSearching(false);
		this->ncTable->get(clientNodeId)->addFlow(rs.getFlow());
		return;
	}

	// Add a forward route.
	// Check the neighbor exists as a next hop.
	uint32_t nextHopInTrace = rs.getNextHopOfCurrentNode(this->node);
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

	this->flowTable->setQoSReqAsIntermediateNode(rs.getFlow(), clientNodeId, nextHopInTrace, rs.getQosReq());
	this->flowTable->getFlowEntry(rs.getFlow())->setSrcRoute(rs.getTrace());
	this->flowTable->getFlowEntry(rs.getFlow())->setActive(true);
	this->flowTable->getFlowEntry(rs.getFlow())->setRouteSearching(false);
	ncEntry->addFlow(rs.getFlow());
	NeighborEntry* prevHop = this->ncTable->get(clientNodeId);
	prevHop->addFlow(rs.getFlow());
	//this->flowTable->setAllocatedBW(rs.getFlow(), nextHopIfIdx, rs.getQosReq().getBandwidth());

	// 170822, set hop-by-hop QoSReq for local repair
	// calculate hop-by-hop QoSReq.
	int hopCount = rs.getTrace().size() - 1;
	double remainingDelay = rs.getQosReq().getDelay() - rs.getEndToEndQuality()->getDelay();
	double deltaDelay = remainingDelay / (double)hopCount;
	double hopDelayReq = (double)(this->ncTable->get(nextHopInTrace)->getDelay()) + deltaDelay;

	double remainingJitter = rs.getQosReq().getJitter() - rs.getEndToEndQuality()->getJitter();
	double deltaJitter = remainingJitter / (double)hopCount;
	double hopJitterReq = (double)this->ncTable->get(nextHopInTrace)->getAverageJitter() + deltaJitter;

	QoSRequirement hopQosReq;
	hopQosReq.setBandwidth(rs.getQosReq().getBandwidth());
	hopQosReq.setDelay(hopDelayReq);
	hopQosReq.setJitter(hopJitterReq);
	hopQosReq.setLossRate(rs.getQosReq().getLossRate());
	this->flowTable->setHopQoSReqAsIntermediate(rs.getFlow(), clientNodeId, nextHopInTrace, hopQosReq);

	// Debug
	this->nodeOut << " (Hop-by-hop QoSReq)" << "\n";
	this->nodeOut << "   hopCount = " << hopCount << "\n";
	this->nodeOut << "   remainingDelay = " << remainingDelay << "\n";
	this->nodeOut << "   deltaDelay = " << deltaDelay << "\n";
	this->nodeOut << "   hopDelayReq of Node " << this->nodeId << " = " << hopDelayReq << "\n";

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

	FlowEntry* flowEntry = this->flowTable->getFlowEntry(probe.getFlow());
	if(!flowEntry){
		// Error: No such flow entry.
		return;
	}

	LinkQuality tempLq;
	tempLq.setBandwidth(probe.getLinkQuality()->getBandwidth());
	tempLq.setDelay(probe.getLinkQuality()->getDelay());
	tempLq.setJitter(probe.getLinkQuality()->getJitter());
	tempLq.setLossRate(probe.getLinkQuality()->getLossRate());

	// Accumulate the link quality of the previous-hop link.
	// (Receiver-side accumulation)
	// Current flow's bandwidth is ignored in calculation.
	NeighborEntry* prevHop = this->ncTable->get(clientNodeId);
	accumulateLinkQualityWithoutFlow(probe.getLinkQuality(), prevHop, probe.getFlow());

	if(this->nodeId == probe.getFlow().getDst()){
		// This node is the destination of the flow.
		// Check if the QoSReq is the same as the flowTable entry's QoSReq.
		if(probe.getQosReq() != flowEntry->getQosReq()){
			// Send End-to-end quality to the source.
			this->nodeOut << " - sending PathQualityReport to src (prevHop: " << clientNodeId << ")\n";
			this->nodeOut << " - probe: " << probe.getQosReq().serialize() << "/n";
			this->nodeOut << " - table: " << flowEntry->getQosReq().serialize() << "\n";

			flowEntry->setQosReq(probe.getQosReq());
			PathQualityReport pqr(probe.getFlow(), probe.getTrace().size() - 1);
			pqr.setQosReq(probe.getQosReq());
			pqr.setEndToEndQuality(probe.getLinkQuality());
			sendRoutingPacket(prevHop->getIp(), pqr.serialize());
		}
		return;
	}

	uint32_t nextHopId = this->routeTable->getNextHop(probe.getFlow());
	if(nextHopId == NODEID_NOT_FOUND){
		// No next-hop found for the flow!!
		return;
	}

	NeighborEntry* nextHop = this->ncTable->get(nextHopId);
	if(!nextHop){
		// No such neighbor entry (maybe not nexthop)
		return;
	}

	// 171019, get available bandwidth (Ignore current flow's bandwidth occupation.)
	double availableBW = getMyAvailableBandwidthWithoutFlow(prevHop, probe.getFlow());
	if(availableBW < 0.0) availableBW = 0.0;

	// debug
#ifdef DEBUG_NODE_OUT
	this->nodeOut << "  - flow avgRtBW (prevhop) = " << flowEntry->getAvgRealTimeBandwidth(clientNodeId) << "\n";
	this->nodeOut << "  - flow avgRtBW (nexthop) = " << flowEntry->getAvgRealTimeBandwidth(nextHop->getNodeId()) << "\n";
	this->nodeOut << "  - flow RtBW (prevhop) = " << flowEntry->getRealTimeBandwidth(clientNodeId) << "\n";
	this->nodeOut << "  - flow RtBW (nexthop) = " << flowEntry->getRealTimeBandwidth(nextHop->getNodeId()) << "\n";
	this->nodeOut << "  - my unallocated BW = " << getMyUnallocatedBandwidth(this->ncTable->get(clientNodeId)) << "\n";
	this->nodeOut << "  - my available BW = " << availableBW << "\n";
#endif

	// Measure the hop-by-hop link quality of the previous hop.
	LinkQuality hopLq(
				availableBW,
				prevHop->getAverageDelay(),
				prevHop->getAverageJitter(),
				prevHop->getLossRate());

	// 170830, check hop-by-hop QoSReq
	if(QoSRequirement::isSatisfactory(flowEntry->getHopQosReq(), hopLq)){
#ifdef DEBUG_PRINT
		NS_LOG_UNCOND(" - @@ hop-by-hop QoS satisfactory.");
		NS_LOG_UNCOND("   - hopReq: " << flowEntry->getHopQosReq().serialize());
		NS_LOG_UNCOND("   - hopLQ : " << hopLq.serialize());
#endif
#ifdef DEBUG_NODE_OUT
		this->nodeOut << " - @@ hop-by-hop QoS satisfactory.\n";
		this->nodeOut << "   - hopReq: " << flowEntry->getHopQosReq().serialize() << "\n";
		this->nodeOut << "   - hopLQ : " << hopLq.serialize() << "\n";
#endif

		// The partial path is satisfactory.
		// Relay the PathProbe to the nexthop.
		sendRoutingPacket(nextHop->getIp(), probe.serialize());
		flowEntry->setUnsatisfactoryCount(0);

	} else {
		// If hop-by-hop QoS is not satisfactory, check threshold.
		int flowUnsatisfactoryCount = flowEntry->getUnsatisfactoryCount();
		if(flowUnsatisfactoryCount < MyConfig::instance().getFlowUnsatisThreshold()){
			flowEntry->setUnsatisfactoryCount(++flowUnsatisfactoryCount);
			sendRoutingPacket(nextHop->getIp(), probe.serialize());
			return;
		}

		// FlowUnsatisfactoryCount exceeds threshold.
		// Increment QoS violation count for statistics.
		flowEntry->setUnsatisfactoryCount(0);
		MyStatistics::instance().incrementQoSViolationCount(probe.getFlow(), this->nodeId);

#ifdef DEBUG_NODE_OUT
		this->nodeOut << " - @@ hop-by-hop QoS NOT satisfactory.\n";
		this->nodeOut << "   - hopReq: " << flowEntry->getHopQosReq().serialize() << "\n";
		this->nodeOut << "   - hopLQ : " << hopLq.serialize() << "\n";

		this->nodeOut << " - ## QoSReq NOT satisfactory!\n";
		this->nodeOut << "   - qosReq: " << probe.getQosReq().serialize() << "\n";
		this->nodeOut << "   - pathLQ: " << probe.getLinkQuality()->serialize() << "\n";
#endif

		switch(this->scheme){
		case BASELINE:{
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
			break;
		}
		case SCHEME_PATIAL_DISCOVERY:{
#ifdef DEBUG_PRINT
			//debug
			NS_LOG_UNCOND(" - PATIAL_DISCOVERY: broadcast ARREQ from the intermediate node.");
#endif
#ifdef DEBUG_NODE_OUT
			this->nodeOut << " - PATIAL_DISCOVERY: broadcast ARREQ from the intermediate node." << "\n";
#endif
			// Partial route discovery from an intermediate node.
			// Create ARREQ as an intermediate node.
			// trace: pre-write src to this intermediate node.
			// TTL: default (max) - current hopcount from src
			// qosreq: copy
			// linkquality: src to here (PathProbe's linkquality subtracting nextHop)
			// seqNO: ?? where can i find it?
			//ARREPRecvEntry* arrepEntry = this->arrepRecvTable->getEntry(probe.getFlow());
			int newSeqNo = probe.getRouteSeqNo() + 1;
			int defaultTTL = atoi(MyConfig::instance().getValue("RouteDiscoveryTTL").c_str());
			int TTL = defaultTTL - probe.getHopCount();
			ARREQ arreq(probe.getFlow(), newSeqNo, TTL, probe.getQosReq());
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
				for(ARREP arrep: arrepEntry->getArrepList(newSeqNo)){
#ifdef DEBUG_NODE_OUT
					this->nodeOut << "      " <<arrep.serialize() << "\n";
#endif
				}
				this->arrepRecvTable->addEntry(probe.getFlow(), newSeqNo);
				Simulator::Schedule(Seconds(1.0), &MyNode::setupRoute, this, probe.getFlow(), newSeqNo);
			} else {
				// Error: previous flow must have been added to ARREPRecvTable.
#ifdef DEBUG_NODE_OUT
				this->nodeOut <<"  #### Error: ARREPRecvEntry of this flow is empty!";
#endif
			}
			break;
		}
		case SCHEME_LOCAL_REPAIR_1HOP:
		case SCHEME_LOCAL_REPAIR_2HOPS:
		{
			/*
			 * Scheme 2 and 3.
			 * Send my previous hop to find detour nodes.
			 * Get top-K unpopular flows of candidate nexthop neighbors, and select one.
			 */

			if(prevHop->isLocalRepairing()){
#ifdef DEBUG_NODE_OUT
				NS_LOG_UNCOND(" - already performing local repair of another flow. Discard this QoS violation.");
#endif
#ifdef DEBUG_NODE_OUT
				this->nodeOut << " - already performing local repair of another flow. Discard this QoS violation.\n";
#endif
				return;
			}
			prevHop->setLocalRepairing(true);

#ifdef DEBUG_PRINT
			//debug
			NS_LOG_UNCOND(" - LOCAL REPAIR-1-2HOPS: send LocalRepairRequest to prevHop.");
#endif
#ifdef DEBUG_NODE_OUT
			this->nodeOut << " - LOCAL REPAIR-1-2HOPS: send LocalRepairRequest to prevHop." << "\n";
#endif

			LocalRepairRequest lrreq;
			lrreq.setFlow(probe.getFlow());
			lrreq.setQosReq(probe.getQosReq());
			lrreq.setEndToEndQuality(&tempLq); // LQ of this node should not be included.
			vector<uint32_t> probeTrace = probe.getTrace();
			int nextHopToSrcIdx = -1;
			for(vector<int>::size_type i=0; i< probeTrace.size(); i++){
				if(clientNodeId == probeTrace[i]){
					nextHopToSrcIdx = i;
					break;
				}
			}
			if(nextHopToSrcIdx > 0){
				lrreq.setNextHopToSrc(probeTrace[nextHopToSrcIdx-1]);
			} else {
				lrreq.setNextHopToSrc(NODEID_NOT_FOUND);
			}
			lrreq.setPreviousNextHop(this->nodeId);
			lrreq.setSrcRoute(probe.getTrace()); // The trace is a full source route.
			sendRoutingPacket(clientIP, lrreq.serialize());
			break;
		}
		case SCHEME_LOCAL_REPAIR_AVG_BW:{
			/*
			 * Select local repair node based on average occupied BW of neighbors.
			 */

#ifdef DEBUG_PRINT
			//debug
			NS_LOG_UNCOND(" - LOCAL REPAIR-AVG-BW: find detour based on average occupied BW.");
#endif
#ifdef DEBUG_NODE_OUT
			this->nodeOut << " - LOCAL REPAIR-AVG-BW: find detour based on average occupied BW." << "\n";
#endif
			// Find detour node candidates: neighbors who know the next-hop.
			vector<NeighborEntry*> detours = this->ncTable->getDetourNodes(nextHop->getNodeId(), probe.getTrace());
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
							nextHop->getNodeId(), selected->getNodeId(), clientNodeId,
							probe.getFlow(), trace,
							probe.getQosReq(), probe.getLinkQuality());
					MyStatistics::instance().incrementLocalRepairCount(probe.getFlow(), this->nodeId);
				}
			}

			break;
		}
		default: break;
		}
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
	FlowCheck* flowCheck = new FlowCheck(ROUTE_FLOWCHECK_REQUEST, 0);
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
		if(flowCheck->getTtl() > 1){
			// Broadcast (fwd) FlowCheck-request if TTL > 1.
			flowCheck->decrementTTL();
			sendRoutingPacket(Ipv4Address::GetBroadcast(), flowCheck->serialize());

			// Update routeTable.
			int hopCount = atoi(MyConfig::instance().getValue("Scheme2FlowCheckTTL").c_str()) - flowCheck->getTtl();
			Route* route = this->routeTable->getDefaultRoute(flowCheck->getNodeId());
			if(!route){
				this->routeTable->addDefaultRoute(flowCheck->getNodeId(), clientNodeId, hopCount);
			} else {
				this->routeTable->updateDefaultRoute(flowCheck->getNodeId(), clientNodeId, hopCount);
			}
		}

		// Create FlowCheck-reply with the same seqNo of FlowCheck-request.
		int ttl = atoi(MyConfig::instance().getValue("Scheme2FlowCheckTTL").c_str());
		FlowCheck flowCheckReply(ROUTE_FLOWCHECK_REPLY, ttl);
		flowCheckReply.setSeqNo(flowCheck->getSeqNo());
		flowCheckReply.setNodeId(flowCheck->getNodeId());
		flowCheckReply.setReplierNodeId(this->nodeId);

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
		flowCheck->decrementTTL();
		if(flowCheck->getNodeId() == this->nodeId){
			// The originator is myself.
			if(this->flowCheckRecvMap.find(flowCheck->getSeqNo()) == this->flowCheckRecvMap.end()){
				// No such seqNo. Silently discard this message.
				delete flowCheck;
			} else {
				// Add FlowCheck-reply to the map<uint32_t, FlowCheck> where uint32_t is a nodeID.
				FlowCheckRecvTable* table = this->flowCheckRecvMap[flowCheck->getSeqNo()];
				table->addFlowCheckReply(flowCheck->getReplierNodeId(), flowCheck);

				// Check route for multi-hop nodes
				NeighborEntry* replierEntry = this->ncTable->get(flowCheck->getReplierNodeId());
				if(!replierEntry){
					Route* replierRoute = this->routeTable->getDefaultRoute(flowCheck->getReplierNodeId());
					if(!replierRoute){
						// Create a new default route for the multi-hop node.
						int hopCount = atoi(MyConfig::instance().getValue("Scheme2FlowCheckTTL").c_str()) - flowCheck->getTtl();
						this->routeTable->addDefaultRoute(flowCheck->getReplierNodeId(), clientNodeId, hopCount);
					}
				}
			}
		} else {
			// Forward FlowCheck-reply to the originator.
			Route* route = this->routeTable->getDefaultRoute(flowCheck->getNodeId());
			sendRoutingPacket(this->ncTable->get(route->getNextHop())->getIp(), flowCheck->serialize());
		}
	} else {
		// Invalid message type. Discard the message.
		delete flowCheck;
	}
}

void MyNode::handleFlowAcceptRequest(string str, ns3::Ipv4Address clientIP, int ifIdx) {
	uint32_t clientNodeId = nodeIdMap->getNodeId(clientIP);
	FlowAcceptRequest request = FlowAcceptRequest::parse(str);
	request.decrementTtl();

#ifdef DEBUG_PRINT
	// debug
	NS_LOG_UNCOND("[Node "<< this->nodeId <<"] handleFlowAcceptRequest (from "<< clientNodeId << ") t=" << Simulator::Now().GetMilliSeconds());
	NS_LOG_UNCOND(" - " << str);
#endif
#ifdef DEBUG_NODE_OUT
	this->nodeOut << "[Node "<< this->nodeId <<"] handleFlowAcceptRequest (from "<< clientNodeId << ") t=" << Simulator::Now().GetMilliSeconds() << "\n";
	this->nodeOut << " - " << str << "\n";
#endif

	// Check if this node has connectivity to the nextHop.
	NeighborEntry* ncEntryNextHop = this->ncTable->get(request.getNextHop());
	if(!ncEntryNextHop){
		return;
	}

	// Check if this node can accept QoS requirement (sender-me-nextHop path).
	NeighborEntry* ncEntryPrevHop = this->ncTable->get(clientNodeId);
	LinkQuality testLq(
			request.getLinkQuality()->getBandwidth(),
			request.getLinkQuality()->getDelay(),
			request.getLinkQuality()->getJitter(),
			request.getLinkQuality()->getLossRate());
	accumulateLinkQualityWithoutFlow(&testLq, ncEntryPrevHop, request.getFlow());
	//accumulateLinkQuality(&testLq, ncEntryPrevHop);
	if(!QoSRequirement::isSatisfactory(request.getQosReq(), testLq)){
		return;
	}

	accumulateLinkQualityWithoutFlow(&testLq, ncEntryNextHop, request.getFlow());
	//accumulateLinkQuality(&testLq, ncEntryNextHop);
	if(QoSRequirement::isSatisfactory(request.getQosReq(), testLq)){
		// Send FlowAcceptReply to the sender.
		FlowAcceptReply reply(request.getFlow(), request.getSeqNo(), request.getSenderId(), request.getNextHop());
		if(request.getDetourTrace().size() > 0){
			reply.setDetourIDs(request.getDetourTrace());
		}
		reply.addDetourID(this->nodeId);
		reply.setDetourLinkQuality(testLq);
		sendRoutingPacket(clientIP, reply.serialize());
	} else {
		// check TTL.
		if(request.getTtl() > 0){
			// Debug
			NS_LOG_UNCOND(" - FlowAcceptRequest.TTL > 0");
			this->nodeOut << " - FlowAcceptRequest.TTL > 0" << "\n";

			// find detour from this node's viewpoint: use ncTable->reachableNodeIds
			// if ncEntry->containsNeighbor(request.getNextHop()):
			pair<uint32_t, NeighborEntry*> p;
			BOOST_FOREACH(p, ncTable->getMap()){
				if(p.first != ncEntryPrevHop->getNodeId() && p.first != ncEntryNextHop->getNodeId()){
					NeighborEntry* temp = p.second;
					if(temp->containsNeighbor(ncEntryNextHop->getNodeId())){
						// Debug
						NS_LOG_UNCOND(" - relay FlowAcceptRequest to " << p.first);
						this->nodeOut << " - relay FlowAcceptRequest to " << p.first << "\n";

						// Get ready to receive FlowAcceptReply. (handleFlowAcceptReply)
						// Need to remember which request has been active: FlowAcceptSentTable.
						// Forward FlowAcceptRequest to this neighbor.
						accumulateLinkQuality(request.getLinkQuality(), ncEntryPrevHop);
						request.addDetourTrace(this->nodeId);
						sendRoutingPacket(temp->getIp(), request.serialize());
					}
				}
			}
		}
	}
}

void MyNode::handleFlowAcceptReply(string str, ns3::Ipv4Address clientIP, int ifIdx) {
	uint32_t clientNodeId = nodeIdMap->getNodeId(clientIP);
	FlowAcceptReply* reply = new FlowAcceptReply(FlowAcceptReply::parse(str));

#ifdef DEBUG_PRINT
	NS_LOG_UNCOND("[Node "<< this->nodeId <<"] handleFlowAcceptReply (from "<< clientNodeId << ") t=" << Simulator::Now().GetMilliSeconds());
	NS_LOG_UNCOND(" - " << str);
#endif
#ifdef DEBUG_NODE_OUT
	this->nodeOut << "[Node "<< this->nodeId <<"] handleFlowAcceptReply (from "<< clientNodeId << ") t=" << Simulator::Now().GetMilliSeconds() << "\n";
	this->nodeOut << " - " << str << "\n";
#endif

	vector<uint32_t> detourIds = reply->getDetourIDs();
	if(reply->getSenderId() == this->nodeId){
		FlowAcceptReplyRecvTable* recvTable = this->flowAccRepRecvMap[reply->getSeqNo()];
		if(recvTable){
			recvTable->addFlowAcceptReply(detourIds[0], reply);
		} else {
#ifdef DEBUG_PRINT
			NS_LOG_UNCOND(" - FlowAcceptReplyRecvTable is null. Ignore this message.");
#endif
#ifdef DEBUG_NODE_OUT
			this->nodeOut << " - FlowAcceptReplyRecvTable is null. Ignore this message.\n";
#endif
			return;
		}
	} else {
		// If this message is not mine, then how can I forward reply to the initiator?
		int myPosition = NODEID_NOT_FOUND;
		for(size_t i=0; i<detourIds.size(); i++){
			if(detourIds[i] == this->nodeId){
				myPosition = i;
				break;
			}
		}
		if(myPosition == 0){
			// Forward FlowAcceptReply to the senderId (neighbor).
			sendRoutingPacket(this->ncTable->get(reply->getSenderId())->getIp(), reply->serialize());
		} else {
			uint32_t prevDetourId = detourIds[myPosition-1];
			sendRoutingPacket(this->ncTable->get(prevDetourId)->getIp(), reply->serialize());
		}
	}
}

void MyNode::handleLocalRepairRequest(string str, ns3::Ipv4Address clientIP, int ifIdx) {
	uint32_t clientNodeId = nodeIdMap->getNodeId(clientIP);
	LocalRepairRequest lrreq = LocalRepairRequest::parse(str);

#ifdef DEBUG_PRINT
	NS_LOG_UNCOND("[Node "<< this->nodeId <<"] handleLocalRepairRequest (from "<< clientNodeId << ") t=" << Simulator::Now().GetMilliSeconds());
	NS_LOG_UNCOND(" - " << str);
#endif
#ifdef DEBUG_NODE_OUT
	this->nodeOut << "[Node "<< this->nodeId <<"] handleLocalRepairRequest (from "<< clientNodeId << ") t=" << Simulator::Now().GetMilliSeconds() << "\n";
	this->nodeOut << " - " << str << "\n";
#endif

	switch(this->scheme){
	case SCHEME_LOCAL_REPAIR_1HOP:{
#ifdef DEBUG_PRINT
		//debug
		NS_LOG_UNCOND(" - LOCAL REPAIR-1HOP: find detour based on average occupied BW.");
#endif
#ifdef DEBUG_NODE_OUT
		this->nodeOut << " - LOCAL REPAIR-1HOP: find detour based on average occupied BW." << "\n";
#endif
		// Get a seqNo for the new FlowCheck-replies.
		// Generate FlowCheckRecvTable for the seqNo.
		int seqNo = this->flowCheckSeqNo++;
		FlowCheckRecvTable* table = new FlowCheckRecvTable();
		table->setFlow(lrreq.getFlow());
		table->setQosReq(lrreq.getQosReq());
		if(lrreq.getFlow().getSrc() == this->nodeId){
			if(this->flowTable->getFlowEntry(lrreq.getFlow())->isRouteSearching()){
				// If the flow is under route-searching status, ignore this request.
#ifdef DEBUG_PRINT
				NS_LOG_UNCOND(" - flow.isRouteSearching == true. Dropped LocalRepairRequest.");
#endif
#ifdef DEBUG_NODE_OUT
				this->nodeOut << " - flow.isRouteSearching == true. Dropped LocalRepairRequest." << "\n";
#endif
				return;
			}
			table->setNextHopToSrc(NODEID_NOT_FOUND);
		} else {
			table->setNextHopToSrc(lrreq.getNextHopToSrc());
		}
		table->setPrevNextHop(lrreq.getPreviousNextHop());
		table->setEndToEndQuality(lrreq.getEndToEndQuality());
		table->setSrcRoute(lrreq.getSrcRoute());
		this->flowCheckRecvMap[seqNo] = table;

		// Find neighbors which knows the previous nexthop as its neighbor.
		vector<NeighborEntry*> candidates = this->ncTable->getDetourNodes(lrreq.getPreviousNextHop(), lrreq.getSrcRoute());
		if(candidates.size() > 0){
			for(NeighborEntry* candidate : candidates){
				// Create and send FlowCheck-request with the seqNo.
				int ttl = atoi(MyConfig::instance().getValue("Scheme2FlowCheckTTL").c_str());
				FlowCheck flowCheck(ROUTE_FLOWCHECK_REQUEST, ttl);
				flowCheck.setNodeId(this->nodeId);
				flowCheck.setSeqNo(seqNo);
				sendRoutingPacket(candidate->getIp(), flowCheck.serialize());
			}

			// Schedule the selectNodeFromFlowCheck function.
			Time flowCheckRecvInterval = Seconds(0.1);
			Simulator::Schedule(flowCheckRecvInterval, &MyNode::selectNodeFromFlowCheck, this, seqNo);
		} else {
			// No detour node exists!! Treat it as Route Error.
			if(lrreq.getFlow().getSrc() == this->nodeId){
				this->routeTable->deleteRoute(lrreq.getFlow());
			} else {
				ARERR arerr(lrreq.getFlow(), this->nodeId, lrreq.getQosReq());
				sendRoutingPacket(this->ncTable->get(lrreq.getNextHopToSrc())->getIp(), arerr.serialize());
			}
		}
		break;
	}
	case SCHEME_LOCAL_REPAIR_2HOPS:{
		// Modified Scheme 2
		// FlowAcceptRequest
#ifdef DEBUG_PRINT
		//debug
		NS_LOG_UNCOND(" - LOCAL REPAIR-2HOP: find detour based on average occupied BW.");
#endif
#ifdef DEBUG_NODE_OUT
		this->nodeOut << " - LOCAL REPAIR-2HOP: find detour based on average occupied BW." << "\n";
#endif
		// Create a new FlowAcceptReplyRecvTable.
		int seqNo = this->flowCheckSeqNo++;
		FlowAcceptReplyRecvTable* table = new FlowAcceptReplyRecvTable();
		table->setFlow(lrreq.getFlow());
		table->setQosReq(lrreq.getQosReq());
		if(lrreq.getFlow().getSrc() == this->nodeId){
			if(this->flowTable->getFlowEntry(lrreq.getFlow())->isRouteSearching()){
				// If the flow is under route-searching status, ignore this request.
#ifdef DEBUG_PRINT
				NS_LOG_UNCOND(" - flow.isRouteSearching == true. Dropped LocalRepairRequest.");
#endif
#ifdef DEBUG_NODE_OUT
				this->nodeOut << " - flow.isRouteSearching == true. Dropped LocalRepairRequest." << "\n";
#endif
				return;
			}
			table->setNextHopToSrc(NODEID_NOT_FOUND);
		} else {
			table->setNextHopToSrc(lrreq.getNextHopToSrc());
		}
		table->setPrevNextHop(lrreq.getPreviousNextHop());
		table->setEndToEndQuality(lrreq.getEndToEndQuality());
		table->setSrcRoute(lrreq.getSrcRoute());
		this->flowAccRepRecvMap[seqNo] = table;

		// Find neighbors which knows the previous nexthop as its neighbor.
		vector<NeighborEntry*> candidates = this->ncTable->getDetourNodes(lrreq.getPreviousNextHop(), lrreq.getSrcRoute());
		if(candidates.size() > 0){
			for(NeighborEntry* candidate : candidates){
				// Create and send FlowCheck-request with the seqNo.
				int ttl = atoi(MyConfig::instance().getValue("Scheme3FlowAcceptTTL").c_str());
				FlowAcceptRequest req(lrreq.getFlow(), seqNo, this->nodeId, lrreq.getPreviousNextHop(), ttl);
				req.setQosReq(lrreq.getQosReq());
				req.setLinkQuality(lrreq.getEndToEndQuality());
				sendRoutingPacket(candidate->getIp(), req.serialize());
			}

			// Schedule the selectNodeFromFlowCheck function.
			Time flowAccRepRecvInterval = Seconds(0.1);
			Simulator::Schedule(flowAccRepRecvInterval, &MyNode::selectNodeFromFlowAcceptReply, this, seqNo);
		} else {
			// No detour node exists!! Treat it as Route Error.
			if(lrreq.getFlow().getSrc() == this->nodeId){
				this->routeTable->deleteRoute(lrreq.getFlow());
			} else {
				ARERR arerr(lrreq.getFlow(), this->nodeId, lrreq.getQosReq());
				sendRoutingPacket(this->ncTable->get(lrreq.getNextHopToSrc())->getIp(), arerr.serialize());
			}
		}
		break;
	}
	default: break;
	}
}

void MyNode::handleLocalRepairReply(string str, ns3::Ipv4Address clientIP, int ifIdx) {
	uint32_t clientNodeId = nodeIdMap->getNodeId(clientIP);
	//LocalRepairReply lrrep = LocalRepairReply::parse(str);

#ifdef DEBUG_PRINT
	NS_LOG_UNCOND("[Node "<< this->nodeId <<"] handleLocalRepairReply (from "<< clientNodeId << ") t=" << Simulator::Now().GetMilliSeconds());
	NS_LOG_UNCOND(" - " << str);
#endif
#ifdef DEBUG_NODE_OUT
	this->nodeOut << "[Node "<< this->nodeId <<"] handleLocalRepairReply (from "<< clientNodeId << ") t=" << Simulator::Now().GetMilliSeconds() << "\n";
	this->nodeOut << " - " << str << "\n";
#endif

	NeighborEntry* ncEntry = this->ncTable->get(clientNodeId);
	ncEntry->setLocalRepairing(false);
}

/*
 * Handle PathQualityReport generated from a destination node.
 */
void MyNode::handlePathQualityReport(string str, ns3::Ipv4Address clientIP,
		int ifIdx) {
	uint32_t clientNodeId = nodeIdMap->getNodeId(clientIP);
	PathQualityReport pqr = PathQualityReport::parse(str);

#ifdef DEBUG_PRINT
	NS_LOG_UNCOND("[Node "<< this->nodeId <<"] handlePathQualityReport (from "<< clientNodeId << ") t=" << Simulator::Now().GetMilliSeconds());
	NS_LOG_UNCOND(" - " << str);
#endif
#ifdef DEBUG_NODE_OUT
	this->nodeOut << "[Node "<< this->nodeId <<"] handlePathQualityReport (from "<< clientNodeId << ") t=" << Simulator::Now().GetMilliSeconds() << "\n";
	this->nodeOut << " - " << str << "\n";
#endif

	FlowEntry* flowEntry = this->flowTable->getFlowEntry(pqr.getFlow());
	if(!flowEntry) return;

	flowEntry->setQosReq(pqr.getQosReq());

	// Source node doesn't need to forward PathQualityReport.
	if(pqr.getFlow().getSrc() == this->nodeId) return;

	// Intermediate nodes: forward the PathQualityReport to the previous hop of the source route.
	int myPosInTrace = -1;
	int counter = 0;
	for(uint32_t traceNodes : flowEntry->getSrcRoute()){
		if(traceNodes == this->nodeId){
			myPosInTrace = counter;
			break;
		}
		counter++;
	}

	if(myPosInTrace > 0){
		uint32_t prevHopNodeId = flowEntry->getSrcRoute()[myPosInTrace-1];
		NeighborEntry* prevHop = this->ncTable->get(prevHopNodeId);

		// Update hop-by-hop QoS requirement of this flow.
		int hopCount = pqr.getHopCount();
		double remainingDelay = pqr.getQosReq().getDelay() - pqr.getEndToEndQuality().getDelay();
		double deltaDelay = remainingDelay / (double)hopCount;
		double hopDelayReq = (double)(prevHop->getDelay()) + deltaDelay;

		double remainingJitter = pqr.getQosReq().getJitter() - pqr.getEndToEndQuality().getJitter();
		double deltaJitter = remainingJitter / (double)hopCount;
		double hopJitterReq = prevHop->getAverageJitter() + deltaJitter;

		QoSRequirement hopQosReq;
		hopQosReq.setBandwidth(pqr.getQosReq().getBandwidth());
		hopQosReq.setDelay(hopDelayReq);
		hopQosReq.setJitter(hopJitterReq);
		hopQosReq.setLossRate(pqr.getQosReq().getLossRate());
		this->flowTable->setHopQoSReqAsIntermediate(pqr.getFlow(), prevHopNodeId, clientNodeId, hopQosReq);

		sendRoutingPacket(prevHop->getIp(), str);
	}
}

/**
 * Handle data packet (MyNS3Packet) for an application layer.
 */
void MyNode::handleMyPacket(Ptr<MyNS3Packet> myPkt, int pktSize, FlowType::Type type, Ipv4Address ipAddr) {
	Time now = Simulator::Now();
	//uint32_t clientNodeId = nodeIdMap->getNodeId(ipAddr);

	// check and do routing
	if(checkDstOfMyPacket(GetPointer(myPkt))){
		// Destination is myself.
		// Debug (180418)
		//this->nodeOut << "[Node " << this->nodeId << "] handleMyPacket " << now.GetMilliSeconds() << "\n";
		//this->nodeOut << " - from: " << clientNodeId << "\n";
		//this->nodeOut << " - pkt seqNo = " << myPkt->getSeqNo() << "\n";
		Flow myFlow(myPkt->getSrc(), myPkt->getAppSrcPort(), myPkt->getDst(), myPkt->getAppDstPort(), type);
		MyApplication* myApp = this->myAppMap[myFlow];

		if(myApp->getTotalNumberOfPkts() == 0){
			// If the totalNumberOfPkts is zero,
			// schedule the checkpoint of updating statistics.
			Time nextTime = Seconds(1.0);
			Simulator::Schedule(nextTime, &MyNode::checkMyApplicationStatistics, this, myFlow);
		}

		myApp->handleApplicationPacket(myPkt);
	} else {
		// forward the packet to the destination.
		Flow flow(myPkt->getSrc(), myPkt->getAppSrcPort(), myPkt->getDst(), myPkt->getAppDstPort(), type);
		Route* route = this->routeTable->getRoute(flow);

		if(route){
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

void MyNode::addMyApplication(MyApplication* myApp) {
#ifdef DEBUG_PRINT
	NS_LOG_UNCOND("[Node "<< this->nodeId <<"] addMyApplication, t=" << Simulator::Now().GetMilliSeconds());
	NS_LOG_UNCOND(" -name: " << myApp->getName());
	NS_LOG_UNCOND(" -flow: " << myApp->getFlowReq().getFlow().toString());
#endif
#ifdef DEBUG_NODE_OUT
	this->nodeOut << "[Node "<< this->nodeId <<"] addMyApplication, t=" << Simulator::Now().GetMilliSeconds() << "\n";
	this->nodeOut << " -name: " << myApp->getName() << "\n";
	this->nodeOut << " -flow: " << myApp->getFlowReq().getFlow().toString() << "\n";
#endif

	Flow flow = myApp->getFlowReq().getFlow();
	this->myAppMap[flow] = myApp;
}

const int MyNode::checkRoutingMessageType(const string msg) {
	vector<std::string> tokens;
	tokenizeString(msg, tokens, "@");
	return atoi(tokens[0].c_str());
}
