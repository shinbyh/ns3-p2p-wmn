/*
 * flow_entry.cc
 *
 *  Created on: Oct 27, 2016
 *      Author: bhshin
 */

#include "flow_entry.h"
#include "flow_stat.h"
#include "my_config.h"
#include <sstream>
#include <boost/foreach.hpp>
#include <boost/format.hpp>
#include "ns3/core-module.h"

using namespace boost;
using namespace ns3;

FlowEntry::FlowEntry(Flow flow, int nodeId, long startTime) {
	this->flow = flow;
	this->active = true;
	this->startTime = startTime;
	this->fwdNodeId = nodeId;
	initAppReq();
	addFlowStat(nodeId);
	//this->ifIdices.push_back(0);
	//this->ifIdices.push_back(1);
	//initFlowStats(this->ifIdices);
}

FlowEntry::FlowEntry(int nodeId, PacketInfo pktInfo) {
	this->flow = pktInfo.getFlow();
	this->active = true;
	this->startTime = pktInfo.getTime();
	this->fwdNodeId = nodeId;
	initAppReq();
	//this->ifIdices.push_back(0);
	//this->ifIdices.push_back(1);
	//initFlowStats(this->ifIdices);
	addFlowStat(nodeId);
	addPacketInfo(nodeId, pktInfo);
}

void FlowEntry::addFlowStat(int nodeId) {
	FlowStat* flowStat = new FlowStat();
	this->flowStats[nodeId] = flowStat;
}

//void FlowEntry::removeFlowStat(int nodeId) {
//
//}

FlowEntry::~FlowEntry() {
	std::map<int, FlowStat*>::iterator itr;
	for(itr = this->flowStats.begin(); itr != this->flowStats.end(); itr++){
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

/*void FlowEntry::initFlowStats(std::vector<int> ifIdices) {
	for(int ifIdx : ifIdices){
		FlowStat* flowStat = new FlowStat();
		flowStats[ifIdx] = flowStat;
	}
}*/

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

void FlowEntry::addPacketInfo(int nodeId, PacketInfo pktInfo) {
	if(!active) setActive(true);
	lastTime = pktInfo.getTime();

	FlowStat* flowStat = this->flowStats[nodeId];
	if(flowStat){
		flowStat->accumulatePacket(pktInfo);
	} else {
		// error: not found
	}
}

bool FlowEntry::checkControlFlow() {
	std::vector<int> ctrlPorts = MyConfig::instance().getControlPorts();

	for(int ctrlPort : ctrlPorts){
		if(this->flow.getSrcPort() == ctrlPort || this->flow.getDstPort() == ctrlPort){
			return true;
		}
	}
	return false;
}

void FlowEntry::resetRealTimeBandwidth() {
	// for all interface, reset realtimeBandwidth
	std::pair<int, FlowStat*> p;
	BOOST_FOREACH(p, this->flowStats){
		FlowStat* stat = p.second;
		stat->addToMovingAvg(stat->getRealTimeBandwidth());
		stat->setRealTimeBandwidth(0);
		stat->setRealTimePackets(0);
	}
}

void FlowEntry::setAllocatedBandwidth(int nodeId, double allocatedBW) {
	if(this->flowStats.find(nodeId) != this->flowStats.end()){
		this->flowStats[nodeId]->setAllocatedBandwidth(allocatedBW);
	} else {
		// error: not found
		NS_LOG_UNCOND(" - ERROR: flowStat["<< nodeId << "] not found!!!!!!!!!!!!");
	}
}

double FlowEntry::getAllocatedBandwidth(int nodeId) {
	if(this->flowStats.find(nodeId) == this->flowStats.end()){
		return 0.0;
	} else {
		return this->flowStats[nodeId]->getAllocatedBandwidth();
	}
}

double FlowEntry::getAvgRealTimeBandwidth(int nodeId) {
	if(this->flowStats.find(nodeId) == this->flowStats.end()){
		return 0.0;
	} else {
		return this->flowStats[nodeId]->getAvgRealTimeBandwidth();
	}
}

const double FlowEntry::getAvgRealTimeBandwidth() const {
	double sum = 0.0;
	int count = 0;

	std::pair<int, FlowStat*> p;
	BOOST_FOREACH(p, this->flowStats){
		if(p.first == this->fwdNodeId){
			//sum += p.second->getAvgRealTimeBandwidth();
			sum += p.second->getRealTimeBandwidth();
			count++;
		}
	}

	if(count != 0) return sum/(double)count;
	else return 0.0;
}

std::string FlowEntry::srcRouteToString(){
	std::stringstream ss;
	for(size_t i=0; i<this->srcRoute.size(); i++){
		if(i + 1 < this->srcRoute.size())
			ss << this->srcRoute[i] << ",";
		else
			ss << this->srcRoute[i];
	}
	return ss.str();
}

std::string FlowEntry::toFormattedFlowInfo() {
	std::stringstream ss;
	ss << this->startTime << " " << this->flow.toFormattedString();
	return ss.str();
}

bool FlowEntry::isFlowStatExists(int nodeId) {
	if(this->flowStats.find(nodeId) == this->flowStats.end()){
		return false;
	} else {
		return true;
	}
}

/**
 * For debug purpose
 */
std::string FlowEntry::toString() {
	std::stringstream ss;
	ss << std::fixed;

	std::pair<int, FlowStat*> p;
	BOOST_FOREACH(p, this->flowStats){
		ss << format("\n%26s  %33s  %7d  %7d  %9.2f  %9.2f  %s")
				% this->flow.toString()
				% this->qosReq.serialize()
				% this->fwdNodeId
				% p.first
				% p.second->getAllocatedBandwidth()
				% p.second->getAvgRealTimeBandwidth()
				% srcRouteToString();
	}
	return ss.str();
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

const std::vector<uint32_t>& FlowEntry::getSrcRoute() const {
	return srcRoute;
}

void FlowEntry::setSrcRoute(const std::vector<uint32_t>& srcRoute) {
	this->srcRoute = srcRoute;
}

bool FlowEntry::operator <(const FlowEntry& a) const {
	return (getAvgRealTimeBandwidth() < a.getAvgRealTimeBandwidth());
}
