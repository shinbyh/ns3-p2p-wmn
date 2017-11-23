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
#include "ns3/core-module.h"

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

void FlowTable::addToFlowEntry(PacketInfo pktInfo) {
	if(flowTable.find(pktInfo.getFlow()) == flowTable.end()){
		// not found, new addition
		FlowEntry* entry = new FlowEntry(pktInfo);

		// debug
		//std::cout << "  [FT] creating a new flowEntry: " << pktInfo.toString() << std::endl;

		flowTable[pktInfo.getFlow()] = entry;
		keysTimeOrder.push_back(pktInfo.getFlow());
	} else {
		//std::cout << "  [FT] adding pktInfo to the existing flowEntry: " << pktInfo.toString() << std::endl;
		FlowEntry* entry = flowTable[pktInfo.getFlow()];
		entry->addPacketInfo(pktInfo);
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

void FlowTable::setQoSReq(Flow flow, int ifIdx, QoSRequirement qosReq) {
	if(flowTable.find(flow) == flowTable.end()){
		std::cout << "  [FT][setAllocatedBW] new FlowEntry!!! (bw = " << qosReq.getBandwidth() << ")" << std::endl;
		FlowEntry* entry = new FlowEntry(flow, ns3::Simulator::Now().GetMilliSeconds());
		flowTable[flow] = entry;
		keysTimeOrder.push_back(flow);
		entry->setQosReq(qosReq);
		entry->setAllocatedBandwidth(ifIdx, qosReq.getBandwidth());
	} else {
		FlowEntry* entry = flowTable[flow];
		entry->setQosReq(qosReq);
		entry->setAllocatedBandwidth(ifIdx, qosReq.getBandwidth());
	}
}

void FlowTable::setHopQoSReq(Flow flow, int ifIdx, QoSRequirement qosReq) {
	if(flowTable.find(flow) == flowTable.end()){
		std::cout << "   [FT][setAllocatedBW] new FlowEntry!!! (bw = " << qosReq.getBandwidth() << ")" << std::endl;
		FlowEntry* entry = new FlowEntry(flow, ns3::Simulator::Now().GetMilliSeconds());
		flowTable[flow] = entry;
		keysTimeOrder.push_back(flow);
		entry->setHopQosReq(qosReq);
	} else {
		FlowEntry* entry = flowTable[flow];
		entry->setHopQosReq(qosReq);
	}
}

void FlowTable::setAllocatedBW(Flow flow, int ifIdx, double allocBW) {
	if(flowTable.find(flow) == flowTable.end()){
		std::cout << "   [FT][setAllocatedBW] new FlowEntry!!!!!! " << std::endl;
		FlowEntry* entry = new FlowEntry(flow, ns3::Simulator::Now().GetMilliSeconds());
		flowTable[flow] = entry;
		keysTimeOrder.push_back(flow);
		entry->setAllocatedBandwidth(ifIdx, allocBW);
	} else {
		FlowEntry* entry = flowTable[flow];
		entry->setAllocatedBandwidth(ifIdx, allocBW);
	}
}

/**
 * Get aggregated allocated bandwidth of all
 * flows for an interface.
 */
double FlowTable::getAllocatedBW(int ifIdx) {
	double allocBW = 0.0;
	std::pair<Flow, FlowEntry*> p;
	BOOST_FOREACH (p, flowTable){
		FlowEntry* entry = p.second;
		allocBW += entry->getAllocatedBandwidth(ifIdx);
	}
	return allocBW;
}

double FlowTable::getOccupiedBW(int ifIdx) {
	double occBW = 0.0;
	std::pair<Flow, FlowEntry*> p;
	BOOST_FOREACH (p, flowTable){
		FlowEntry* entry = p.second;
		occBW += entry->getAvgRealTimeBandwidth(ifIdx);
	}
	return occBW;
}

double FlowTable::getAvgResidualBW(){
	double resBW = 0.0;
	std::pair<Flow, FlowEntry*> p;
	BOOST_FOREACH (p, flowTable){
		FlowEntry* entry = p.second;
		resBW += entry->getAvgResidualBandwidth();
	}
	if(flowTable.size() > 0)
		return resBW / (double)flowTable.size();
	else
		return 0.0;
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

	for(Flow flow : keysTimeOrder){
		FlowEntry* entry = flowTable[flow];

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

std::string FlowTable::printFlowTable(int nodeId){
	std::stringstream ss;
	ss << std::fixed << "[Node "<< nodeId << "] FlowTable\n";
	ss << "         flow                        QoS Requirement             allocBW      avgRTBW    avgResBW\n"
		<< "---------------------------------------------------------------------------------------------------\n";

	for(Flow flow : keysTimeOrder){
		FlowEntry* entry = flowTable[flow];

		// filter out control flows
		if(entry->isControlFlow()) continue;

		ss << entry->toString() << "\n";
	}

	return ss.str();
}
