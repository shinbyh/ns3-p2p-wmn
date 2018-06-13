/*
 * flow_table.cc
 *
 *  Created on: Oct 27, 2016
 *      Author: bhshin
 */
#include "flow_table.h"
#include "time_util.h"
#include <boost/foreach.hpp>
#include <sstream> // getAllFlowInfo()
#include <algorithm> // for sorting FlowEntries
#include "ns3/core-module.h"
#include "my_config.h" // for NODEID_NOT_FOUND

using namespace ns3;

FlowTable::FlowTable() {
}

FlowTable::FlowTable(const FlowTable& other) {
}

FlowTable::~FlowTable() {
	std::map<Flow, FlowEntry*>::iterator itr;
	for(itr = flowTable.begin(); itr != flowTable.end(); itr++){
		delete itr->second;
	}
}

void FlowTable::addToFlowEntry(int nodeId, PacketInfo pktInfo) {
	if(flowTable.find(pktInfo.getFlow()) == flowTable.end()){
		// not found, new addition
		FlowEntry* entry = new FlowEntry(nodeId, pktInfo);
		entry->addPacketInfo(nodeId, pktInfo);

		flowTable[pktInfo.getFlow()] = entry;
		keysTimeOrder.push_back(pktInfo.getFlow());
	} else {
		FlowEntry* entry = flowTable[pktInfo.getFlow()];
		if(!entry->isFlowStatExists(nodeId)){
			entry->addFlowStat(nodeId);
		}
		entry->addPacketInfo(nodeId, pktInfo);
	}
}

void FlowTable::updateRealTimeBandwidth() {
	std::pair<Flow, FlowEntry*> p;
	BOOST_FOREACH (p, flowTable){
		FlowEntry* entry = p.second;
		if(entry->isActive()){
			entry->resetRealTimeBandwidth();
		}
	}
}

void FlowTable::setQoSReqAsSource(Flow flow, uint32_t nextHopNodeId, QoSRequirement qosReq) {
	if(this->flowTable.find(flow) == this->flowTable.end()){
		NS_LOG_UNCOND("  [FT][setQoSReq] new FlowEntry as src!!! (bw=" << qosReq.getBandwidth() << ") (next=" << nextHopNodeId << ")");
		FlowEntry* entry = new FlowEntry(flow, nextHopNodeId, ns3::Simulator::Now().GetMilliSeconds());
		this->flowTable[flow] = entry;
		keysTimeOrder.push_back(flow);
	}

	FlowEntry* entry = this->flowTable[flow];
	if(!entry->isFlowStatExists(nextHopNodeId)){
		entry->addFlowStat(nextHopNodeId);
	}

	entry->setQosReq(qosReq);
	entry->setFwdNodeId(nextHopNodeId);
	entry->setAllocatedBandwidth(nextHopNodeId, qosReq.getBandwidth());
}

void FlowTable::setQoSReqAsDestination(Flow flow, uint32_t prevNodeId, QoSRequirement qosReq) {
	if(this->flowTable.find(flow) == this->flowTable.end()){
		NS_LOG_UNCOND("  [FT][setQoSReq] new FlowEntry as dst!!! (bw=" << qosReq.getBandwidth() << ") (prev=" << prevNodeId << ")");
		FlowEntry* entry = new FlowEntry(flow, prevNodeId, ns3::Simulator::Now().GetMilliSeconds());
		this->flowTable[flow] = entry;
		keysTimeOrder.push_back(flow);
	}

	FlowEntry* entry = this->flowTable[flow];
	if(!entry->isFlowStatExists(prevNodeId)){
		entry->addFlowStat(prevNodeId);
	}
	entry->setQosReq(qosReq);
	entry->setPrevNodeId(prevNodeId);
	entry->setAllocatedBandwidth(prevNodeId, qosReq.getBandwidth());
}

void FlowTable::setQoSReqAsIntermediateNode(Flow flow, uint32_t prevNodeId, uint32_t nextHopNodeId, QoSRequirement qosReq) {
	if(this->flowTable.find(flow) == this->flowTable.end()){
		NS_LOG_UNCOND("  [FT][setQoSReq] new FlowEntry as intermediate!!! (bw=" << qosReq.getBandwidth() << ") (prev= "<< prevNodeId << ", next=" << nextHopNodeId << ")");
		FlowEntry* entry = new FlowEntry(flow, nextHopNodeId, ns3::Simulator::Now().GetMilliSeconds());
		this->flowTable[flow] = entry;
		keysTimeOrder.push_back(flow);
	}

	FlowEntry* entry = this->flowTable[flow];
	if(!entry->isFlowStatExists(nextHopNodeId)){
		entry->addFlowStat(nextHopNodeId);
	}
	if(!entry->isFlowStatExists(prevNodeId)){
		entry->addFlowStat(prevNodeId);
	}
	entry->setQosReq(qosReq);
	entry->setFwdNodeId(nextHopNodeId);
	entry->setPrevNodeId(prevNodeId);
	entry->setAllocatedBandwidth(nextHopNodeId, qosReq.getBandwidth());
	entry->setAllocatedBandwidth(prevNodeId, qosReq.getBandwidth());
}

void FlowTable::setHopQoSReqAsSource(Flow flow, uint32_t nextNodeId, QoSRequirement qosReq) {
	if(flowTable.find(flow) == flowTable.end()){
		std::cout << "   [FT][setHopQoSReq] new FlowEntry!!! (bw = " << qosReq.getBandwidth() << ") (next=" << nextNodeId << ")" << std::endl;
		FlowEntry* entry = new FlowEntry(flow, nextNodeId, ns3::Simulator::Now().GetMilliSeconds());
		flowTable[flow] = entry;
		keysTimeOrder.push_back(flow);
		entry->setHopQosReq(qosReq);
	} else {
		FlowEntry* entry = flowTable[flow];
		entry->setHopQosReq(qosReq);
		entry->setFwdNodeId(nextNodeId);
	}
}

void FlowTable::setHopQoSReqAsDestination(Flow flow, uint32_t prevNodeId, QoSRequirement qosReq) {
	if(flowTable.find(flow) == flowTable.end()){
		std::cout << "   [FT][setHopQoSReq] new FlowEntry!!! (bw = " << qosReq.getBandwidth() << ") (prev=" << prevNodeId << ")" << std::endl;
		FlowEntry* entry = new FlowEntry(flow, prevNodeId, ns3::Simulator::Now().GetMilliSeconds());
		flowTable[flow] = entry;
		keysTimeOrder.push_back(flow);
		entry->setHopQosReq(qosReq);
	} else {
		FlowEntry* entry = flowTable[flow];
		entry->setHopQosReq(qosReq);
		entry->setPrevNodeId(prevNodeId);
	}
}

void FlowTable::setHopQoSReqAsIntermediate(Flow flow, uint32_t prevNodeId, uint32_t nextNodeId, QoSRequirement qosReq) {
	if(flowTable.find(flow) == flowTable.end()){
		std::cout << "   [FT][setHopQoSReq] new FlowEntry!!! (bw = " << qosReq.getBandwidth() << ") (prev=" << prevNodeId << ", next="<< nextNodeId << ")" << std::endl;
		FlowEntry* entry = new FlowEntry(flow, prevNodeId, ns3::Simulator::Now().GetMilliSeconds());
		flowTable[flow] = entry;
		keysTimeOrder.push_back(flow);
		entry->setHopQosReq(qosReq);
	} else {
		FlowEntry* entry = flowTable[flow];
		entry->setHopQosReq(qosReq);
		entry->setPrevNodeId(prevNodeId);
		entry->setFwdNodeId(nextNodeId);
	}
}


FlowEntry* FlowTable::getFlowEntry(Flow flow) {
	if(flowTable.find(flow) == flowTable.end()){
		return 0;
	} else {
		return flowTable[flow];
	}
}

void FlowTable::markInactiveFlows(long from, long threshold) {
	std::pair<Flow, FlowEntry*> p;
	BOOST_FOREACH (p, flowTable){
		FlowEntry* entry = p.second;
		if(from - entry->getLastTime() > threshold){
			entry->setActive(false);
		}
	}
}

void FlowTable::setFlowInactive(Flow flow){
	std::pair<Flow, FlowEntry*> p;
	BOOST_FOREACH (p, flowTable){
		FlowEntry* entry = p.second;
		if(entry->getFlow() == flow){
			entry->setActive(false);
		}
	}
}

std::vector<FlowEntry*> FlowTable::getAllFlowEntries() {
	vector<FlowEntry*> flows;
	pair<Flow, FlowEntry*> p;
	BOOST_FOREACH (p, flowTable){
		FlowEntry* flowEntry = p.second;
		flows.push_back(flowEntry);
	}

	return flows;
}

int FlowTable::getNumOfFlows() {
	return keysTimeOrder.size();
}


std::string FlowTable::getAllFlowInfo() {
	std::stringstream ss;
	ss << std::fixed;

	for(Flow flow : keysTimeOrder){
		FlowEntry* entry = flowTable[flow];

		// filter out control flows
		if(entry->isControlFlow()) continue;

		ss << entry->toFormattedFlowInfo() << "\n";
	}
	return ss.str();
}

std::string FlowTable::getFormattedFlowOutput(std::string time) {
	std::stringstream ss;
	ss << std::fixed << time;

	for(Flow flow : this->keysTimeOrder){
		FlowEntry* entry = this->flowTable[flow];

		// filter out control flows
		if(entry->isControlFlow()) continue;

		if(entry->isActive()){
			//ss << "\t" << entry->getAvgRealTimeBandwidth();
			if(entry->getPrevNodeId() != NODEID_NOT_FOUND){
				ss << "\t" << entry->getRealTimeBandwidth(entry->getPrevNodeId());
			} else {
				ss << "\t" << entry->getRealTimeBandwidth(entry->getFwdNodeId());
			}
		} else {
			ss << "\t0";
		}
	}
	//ss << "\n";

	return ss.str();
}

std::vector<FlowEntry*> FlowTable::getUnpopularFlows(size_t k) {
	vector<FlowEntry*> flows = getAllFlowEntries();

	// sort flow entries in an ascending order of bandwidth.
	sort(flows.begin(), flows.end());

	if(flows.size() > k) flows.resize(k);
	return flows;
}

/**
 * Returns the occupied bandwidth of a link between
 * myself and a neighbor node specified by nodeId.
 */
double FlowTable::getOccupiedBandwidth(uint32_t nodeId) {
	double bandwidth = 0.0;

	std::pair<Flow, FlowEntry*> p;
	BOOST_FOREACH (p, flowTable){
		FlowEntry* entry = p.second;
		if(entry->getFwdNodeId() == nodeId){
			bandwidth += entry->getAvgRealTimeBandwidth(nodeId);
		}
	}

	return bandwidth;
}

std::string FlowTable::printFlowTable(uint32_t nodeId){
	std::stringstream ss;
	ss << std::fixed << "[Node "<< nodeId << "] FlowTable (t=" << Simulator::Now().GetSeconds() << ")\n";
	ss << "         flow                        QoS Requirement               fwd   nodeId     allocBW    avgRTBW   rtSch  act    trace\n"
		<< "----------------------------------------------------------------------------------------------------------------------------";

	for(Flow flow : keysTimeOrder){
		FlowEntry* entry = flowTable[flow];

		// filter out control flows
		if(entry->isControlFlow()) continue;

		ss << entry->toString() << "\n";
	}

	return ss.str();
}
