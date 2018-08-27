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

FlowEntry::FlowEntry(Flow flow, uint32_t nodeId, long startTime) {
	this->flow = flow;
	this->active = true;
	this->startTime = startTime;
	this->fwdNodeId = NODEID_NOT_FOUND;
	this->prevNodeId = NODEID_NOT_FOUND;
	this->randomnessActivated = false;
	initAppReq();
	addFlowStat(nodeId);
}

FlowEntry::FlowEntry(uint32_t nodeId, PacketInfo pktInfo) {
	this->flow = pktInfo.getFlow();
	this->active = true;
	this->startTime = pktInfo.getTime();
	this->fwdNodeId = NODEID_NOT_FOUND;
	this->prevNodeId = NODEID_NOT_FOUND;
	this->randomnessActivated = false;
	initAppReq();
	addFlowStat(nodeId);
	addPacketInfo(nodeId, pktInfo);
}

FlowEntry::~FlowEntry() {
	std::map<uint32_t, FlowStat*>::iterator itr;
	for(itr = this->flowStats.begin(); itr != this->flowStats.end(); itr++){
		delete itr->second;
	}
}

void FlowEntry::addFlowStat(uint32_t nodeId) {
	FlowStat* flowStat = new FlowStat();
	this->flowStats[nodeId] = flowStat;
}

void FlowEntry::deleteFlowStat(uint32_t nodeId) {
	this->flowStats.erase(nodeId);
}

void FlowEntry::initAppReq() {
	//this->qosReq initialize?
	flowSeqNo = 0;
	packetSeqNo = 0;
	unsatisfactoryCount = 0;
	routingFlag = FLOW_NO_ROUTE;
	routeSearching = false;
	this->controlFlow = checkControlFlow();
}

bool FlowEntry::isActive() const {
	return active;
}

void FlowEntry::setActive(bool active) {
	this->active = active;
}

int FlowEntry::getFlowSeqNo() const {
	return flowSeqNo;
}

void FlowEntry::setFlowSeqNo(int flowSeqNo) {
	this->flowSeqNo = flowSeqNo;
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

void FlowEntry::addPacketInfo(uint32_t nodeId, PacketInfo pktInfo) {
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
	std::pair<uint32_t, FlowStat*> p;
	BOOST_FOREACH(p, this->flowStats){
		FlowStat* stat = p.second;
		stat->addToMovingAvg(stat->getRealTimeBandwidth());
		stat->setRealTimeBandwidth(0);
		stat->setRealTimePackets(0);
	}
}

int FlowEntry::getNumberOfRealTimePackets(uint32_t nodeId){
	if(this->flowStats.find(nodeId) != this->flowStats.end()){
		return this->flowStats[nodeId]->getRealTimePackets();
	} else {
		// error: not found
		return 0;
	}
}

void FlowEntry::setAllocatedBandwidth(uint32_t nodeId, double allocatedBW) {
	if(this->flowStats.find(nodeId) != this->flowStats.end()){
		this->flowStats[nodeId]->setAllocatedBandwidth(allocatedBW);
	} else {
		// error: not found
		NS_LOG_UNCOND(" - ERROR: flowStat["<< nodeId << "] not found!!!!!!!!!!!!");
	}
}

double FlowEntry::getAllocatedBandwidth(uint32_t nodeId) {
	if(this->flowStats.find(nodeId) == this->flowStats.end()){
		return 0.0;
	} else {
		return this->flowStats[nodeId]->getAllocatedBandwidth();
	}
}

double FlowEntry::getAvgRealTimeBandwidth(uint32_t nodeId) {
	if(this->flowStats.find(nodeId) == this->flowStats.end()){
		return 0.0;
	} else {
		return this->flowStats[nodeId]->getAvgRealTimeBandwidth();
	}
}

double FlowEntry::getLastAccumulatedBandwidth(uint32_t nodeId){
	if(this->flowStats.find(nodeId) == this->flowStats.end()){
		return 0.0;
	} else {
		return this->flowStats[nodeId]->getRealTimeBandwidth();
	}
}

double FlowEntry::getRealTimeBandwidth(uint32_t nodeId){
	if(this->flowStats.find(nodeId) == this->flowStats.end()){
		return 0.0;
	} else {
		int64_t elapsedMilliSec = Simulator::Now().GetMilliSeconds() % 1000;
		if(elapsedMilliSec != 0){
			return (double)this->flowStats[nodeId]->getRealTimeBandwidth() * (1000.0 / (double)elapsedMilliSec);
		} else {
			return this->flowStats[nodeId]->getRealTimeBandwidth();
		}
	}
}

/**
 * Get the average of all links for the flow.
 * For intermediate nodes, there are both previous and next hops.
 * This function calculates the average bandwidth of both hops.
 */
const double FlowEntry::getAvgRealTimeBandwidth() const {
	double sum = 0.0;
	int count = 0;

	std::pair<uint32_t, FlowStat*> p;
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

bool FlowEntry::isFlowStatExists(uint32_t nodeId) {
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

	std::pair<uint32_t, FlowStat*> p;
	BOOST_FOREACH(p, this->flowStats){
		ss << format("\n%26s  %33s  %7d  %7d  %9.2f  %9.2f  %5d %4d  %4d  %s")
				% this->flow.toString()
				% this->qosReq.serialize()
				% this->fwdNodeId
				% p.first
				% p.second->getAllocatedBandwidth()
				% p.second->getAvgRealTimeBandwidth()
				% this->isRouteSearching()
				% this->isActive()
				% this->flowSeqNo
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

bool FlowEntry::isRandomnessActivated() const {
	return randomnessActivated;
}

void FlowEntry::setRandomnessActivated(bool randomnessActivated) {
	this->randomnessActivated = randomnessActivated;
}

double FlowEntry::getRandomnessTime() const {
	return randomnessTime;
}

void FlowEntry::setRandomnessTime(double randomnessTime) {
	this->randomnessTime = randomnessTime;
}

uint32_t FlowEntry::getFwdNodeId() const {
	return fwdNodeId;
}

void FlowEntry::setFwdNodeId(uint32_t fwdNodeId) {
	this->fwdNodeId = fwdNodeId;
	if(this->flowStats.find(fwdNodeId) == this->flowStats.end()){
		addFlowStat(fwdNodeId);
	}
}

uint32_t FlowEntry::getPrevNodeId() const {
	return prevNodeId;
}

void FlowEntry::setPrevNodeId(uint32_t prevNodeId) {
	this->prevNodeId = prevNodeId;
	if(this->flowStats.find(prevNodeId) == this->flowStats.end()){
		addFlowStat(prevNodeId);
	}
}

int FlowEntry::getPacketSeqNo() const {
	return packetSeqNo;
}

void FlowEntry::setPacketSeqNo(int packetSeqNo) {
	this->packetSeqNo = packetSeqNo;
}

void FlowEntry::incrementPacketSeNo(){
	this->packetSeqNo++;
}

bool FlowEntry::operator <(const FlowEntry& a) const {
	return (getAvgRealTimeBandwidth() < a.getAvgRealTimeBandwidth());
}
