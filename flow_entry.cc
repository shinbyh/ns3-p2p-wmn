/*
 * flow_entry.cc
 *
 *  Created on: Oct 27, 2016
 *      Author: bhshin
 */

#include "flow_entry.h"
#include "flow_stat.h"
#include <sstream>
#include <boost/foreach.hpp>

#include "my_config.h"

FlowEntry::FlowEntry(Flow flow, long startTime) {
	this->flow = flow;
	this->active = true;
	this->startTime = startTime;
	initAppReq();
	this->ifIdices.push_back(0);
	this->ifIdices.push_back(1);
	initFlowStats(this->ifIdices);
}

FlowEntry::FlowEntry(PacketInfo pktInfo) {
	this->flow = pktInfo.getFlow();
	this->active = true;
	this->startTime = pktInfo.getTime();
	initAppReq();
	this->ifIdices.push_back(0);
	this->ifIdices.push_back(1);
	initFlowStats(this->ifIdices);
	addPacketInfo(pktInfo);
}

FlowEntry::~FlowEntry() {
	std::map<int, FlowStat*>::iterator itr;
	for(itr = flowStats.begin(); itr != flowStats.end(); itr++){
		delete itr->second;
	}
}

void FlowEntry::initAppReq() {
	//this->qosReq initialize?
	appReqSeqNo = 0;
	unsatisfactoryCount = 0;
	routingFlag = FLOW_NO_ROUTE;
	routeSearching = false;
	this->controlFlow = checkControlFlow();
}

void FlowEntry::initFlowStats(std::vector<int> ifIdices) {
	for(int ifIdx : ifIdices){
		FlowStat* flowStat = new FlowStat();
		flowStats[ifIdx] = flowStat;
	}
	/*for(string iface : MyConfig::instance().getIfaces()){
		FlowStat* flowStat = new FlowStat();
		flowStats[iface] = flowStat;
	}*/
}


bool FlowEntry::isActive() const {
	return active;
}

void FlowEntry::setActive(bool active) {
	this->active = active;
}

int FlowEntry::getAppReqSeqNo() const {
	return appReqSeqNo;
}

void FlowEntry::setAppReqSeqNo(int appReqSeqNo) {
	this->appReqSeqNo = appReqSeqNo;
}

const Flow& FlowEntry::getFlow() const {
	return flow;
}

void FlowEntry::setFlow(const Flow& flow) {
	this->flow = flow;
}

bool FlowEntry::isControlFlow() const {
	return controlFlow;
}

void FlowEntry::setControlFlow(bool controlFlow) {
	this->controlFlow = controlFlow;
}

bool FlowEntry::isRouteSearching() const {
	return routeSearching;
}

void FlowEntry::setRouteSearching(bool routeSearching) {
	this->routeSearching = routeSearching;
}

int FlowEntry::getRoutingFlag() const {
	return routingFlag;
}

void FlowEntry::setRoutingFlag(int routingFlag) {
	this->routingFlag = routingFlag;
}

int FlowEntry::getUnsatisfactoryCount() const {
	return unsatisfactoryCount;
}

void FlowEntry::setUnsatisfactoryCount(int unsatisfactoryCount) {
	this->unsatisfactoryCount = unsatisfactoryCount;
}

long FlowEntry::getLastTime() const {
	return lastTime;
}

void FlowEntry::setLastTime(long lastTime) {
	this->lastTime = lastTime;
}

long FlowEntry::getStartTime() const {
	return startTime;
}

void FlowEntry::setStartTime(long startTime) {
	this->startTime = startTime;
}

void FlowEntry::addPacketInfo(PacketInfo pktInfo) {
	if(!active) setActive(true);
	lastTime = pktInfo.getTime();

	if(flowStats.find(pktInfo.getIfIdx()) == flowStats.end()){
		// not found
		FlowStat* flowStat = new FlowStat();
		flowStats[pktInfo.getIfIdx()] = flowStat;
		flowStat->accumulatePacket(pktInfo);
	} else {
		FlowStat* flowStat = flowStats[pktInfo.getIfIdx()];
		flowStat->accumulatePacket(pktInfo);
	}
}

bool FlowEntry::checkControlFlow() {
	std::vector<int> ctrlPorts = MyConfig::instance().getControlPorts();

	for(int ctrlPort : ctrlPorts){
		if(flow.getSrcPort() == ctrlPort || flow.getDstPort() == ctrlPort){
			return true;
		}
	}
	return false;
}

void FlowEntry::resetRealTimeBandwidth() {
	// for all interface, reset realtimeBandwidth
	for(int ifIdx : this->ifIdices){
		resetRealTimeBandwidth(ifIdx);
	}
}

void FlowEntry::resetRealTimeBandwidth(int ifIdx) {
	if(flowStats.find(ifIdx) == flowStats.end()){
		// not found, do nothing.
	} else {
		FlowStat* stat = flowStats[ifIdx];
		stat->addToMovingAvg(stat->getRealTimeBandwidth());
		stat->setRealTimeBandwidth(0);
		stat->setRealTimePackets(0);
	}
}

void FlowEntry::setAllocatedBandwidth(int ifIdx, double allocatedBW) {
	flowStats[ifIdx]->setAllocatedBandwidth(allocatedBW);
}

double FlowEntry::getAllocatedBandwidth(int ifIdx) {
	return flowStats[ifIdx]->getAllocatedBandwidth();
}

double FlowEntry::getAvgRealTimeBandwidth(int ifIdx) {
	return flowStats[ifIdx]->getAvgRealTimeBandwidth();
}

double FlowEntry::getAvgRealTimeBandwidth() {
	double sum = 0.0;

	std::pair<std::string, FlowStat*> p;
	BOOST_FOREACH(p, flowStats){
		//sum += p.second->getAvgRealTimeBandwidth();
		sum += p.second->getRealTimeBandwidth();
	}
	return sum;
}

std::string FlowEntry::toFormattedFlowInfo() {
	std::stringstream ss;
	ss << this->startTime << " " << this->flow.toFormattedString();
	return ss.str();
}

/**
 * For debug purpose
 */
std::string FlowEntry::toString() {
	std::stringstream ss;
	ss << std::fixed;
	ss << "(FlowEntry)\n"
		<< " -Flow: " << flow.toString() << "\n"
		<< " -startTime: " << startTime << "\n"
		<< " -lastTime: " << lastTime << "\n"
		<< " -appReqSeqNo: " << appReqSeqNo << "\n"
		<< " -active: " << active  << "\n"
		<< " -controlFlow: " << controlFlow << "\n"
		<< " -FlowStats: ";

	std::stringstream ss2;
	std::pair<std::string, FlowStat*> pa;
	BOOST_FOREACH (pa, flowStats) {
		ss2 << "(" << pa.first << ") " << pa.second->toString() << "\n";
	}

	return ss.str() + ss2.str();
}

QoSRequirement FlowEntry::getQosReq() const {
	return qosReq;
}

void FlowEntry::setQosReq(QoSRequirement qosReq) {
	this->qosReq = qosReq;
}

QoSRequirement FlowEntry::getHopQosReq() const {
	return hopQosReq;
}

void FlowEntry::setHopQosReq(QoSRequirement& hopQosReq) {
	this->hopQosReq = hopQosReq;
}
