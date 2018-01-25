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

void FlowTable::setQoSReq(Flow flow, int nodeId, QoSRequirement qosReq) {
	if(this->flowTable.find(flow) == this->flowTable.end()){
		NS_LOG_UNCOND("  [FT][setQoSReq] new FlowEntry!!! (bw=" << qosReq.getBandwidth() << ") (nodeid=" << nodeId << ")");
		FlowEntry* entry = new FlowEntry(flow, nodeId, ns3::Simulator::Now().GetMilliSeconds());
		this->flowTable[flow] = entry;
		keysTimeOrder.push_back(flow);
	}

	FlowEntry* entry = this->flowTable[flow];
	if(!entry->isFlowStatExists(nodeId)){
		entry->addFlowStat(nodeId);
	}
	entry->setQosReq(qosReq);
	entry->setFwdNodeId(nodeId);
	entry->setAllocatedBandwidth(nodeId, qosReq.getBandwidth());
}

void FlowTable::setHopQoSReq(Flow flow, int nodeId, QoSRequirement qosReq) {
	if(flowTable.find(flow) == flowTable.end()){
		std::cout << "   [FT][setHopQoSReq] new FlowEntry!!! (bw = " << qosReq.getBandwidth() << ") (nodeid=" << nodeId << ")" << std::endl;
		FlowEntry* entry = new FlowEntry(flow, nodeId, ns3::Simulator::Now().GetMilliSeconds());
		flowTable[flow] = entry;
		keysTimeOrder.push_back(flow);
		entry->setHopQosReq(qosReq);
	} else {
		FlowEntry* entry = flowTable[flow];
		entry->setHopQosReq(qosReq);
		entry->setFwdNodeId(nodeId);
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
			ss << "\t" << entry->getAvgRealTimeBandwidth();
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
	ss << "         flow                        QoS Requirement               fwd   nodeId     allocBW    avgRTBW    trace\n"
		<< "---------------------------------------------------------------------------------------------------------------------";

	for(Flow flow : keysTimeOrder){
		FlowEntry* entry = flowTable[flow];

		// filter out control flows
		if(entry->isControlFlow()) continue;

		ss << entry->toString() << "\n";
	}

	return ss.str();
}
