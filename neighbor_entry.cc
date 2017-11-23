/*
 * neighbor_entry.cc
 *
 *  Created on: Dec 12, 2016
 *      Author: bhshin
 */

#include "neighbor_entry.h"
#include <sstream>
#include <boost/format.hpp>

using namespace boost;


NeighborEntry::NeighborEntry() {
	initialize();
}

NeighborEntry::NeighborEntry(uint32_t nodeId, ns3::Ipv4Address ipAddr, bool isMeshRouter) {
	this->nodeId = nodeId;
	this->ipAddr = ipAddr;
	this->isMeshRouter = isMeshRouter;
	initialize();
}

void NeighborEntry::initialize() {
	this->etxSample.set_capacity(CIRCULAR_BUFFER_SIZE);
	this->avgDelay.set_capacity(CIRCULAR_BUFFER_SIZE);
	this->avgJitter.set_capacity(CIRCULAR_BUFFER_SIZE);
	this->dmSeqNo = 0;
}

const double NeighborEntry::getAllocBw() const {
	return allocBw;
}

void NeighborEntry::setAllocBw(double allocBw) {
	this->allocBw = allocBw;
}

const long NeighborEntry::getDelay() const {
	return delay;
}

void NeighborEntry::setDelay(long delay) {
	this->delay = delay;
}

const bool NeighborEntry::isIsActive() const {
	return isActive;
}

void NeighborEntry::setIsActive(bool isActive) {
	this->isActive = isActive;
}

const bool NeighborEntry::isIsMeshRouter() const {
	return isMeshRouter;
}

void NeighborEntry::setIsMeshRouter(bool isMeshRouter) {
	this->isMeshRouter = isMeshRouter;
}

const long NeighborEntry::getLastUpdateTime() const {
	return lastUpdateTime;
}

void NeighborEntry::setLastUpdateTime(long lastUpdateTime) {
	this->lastUpdateTime = lastUpdateTime;
}

const double NeighborEntry::getLossRate() const {
	return lossRate;
}

void NeighborEntry::setLossRate(double lossRate) {
	this->lossRate = lossRate;
}

const int NeighborEntry::getNumOfFlows() const {
	return numOfFlows;
}

void NeighborEntry::setNumOfFlows(int numOfFlows) {
	this->numOfFlows = numOfFlows;
}

const double NeighborEntry::getOccupiedBw() const {
	return occupiedBw;
}

void NeighborEntry::setOccupiedBw(double occupiedBw) {
	this->occupiedBw = occupiedBw;
}

const double NeighborEntry::getAvgOccupiedBw() const {
	return avgOccupiedBw;
}

void NeighborEntry::setAvgOccupiedBw(double avgOccupiedBw) {
	this->avgOccupiedBw = avgOccupiedBw;
}

const ns3::Time& NeighborEntry::getPrevDstDelayMsrTime() const {
	return prevDstDelayMsrTime;
}

void NeighborEntry::setPrevDstDelayMsrTime(const ns3::Time& prevDstDelayMsrTime) {
	this->prevDstDelayMsrTime = prevDstDelayMsrTime;
}

const ns3::Time& NeighborEntry::getPrevSrcDelayMsrTime() const {
	return prevSrcDelayMsrTime;
}

void NeighborEntry::setPrevSrcDelayMsrTime(const ns3::Time& prevSrcDelayMsrTime) {
	this->prevSrcDelayMsrTime = prevSrcDelayMsrTime;
}

const double NeighborEntry::calculateLossRate() const {
	boost::circular_buffer<double>::const_iterator it = etxSample.begin();
	double sum = 0.0;
	for(it = etxSample.begin(); it != etxSample.end(); ++it){
		sum += *it;
	}
	return sum / etxSample.size();
}

const bool NeighborEntry::containsIP(ns3::Ipv4Address ipAddr) const {
	for(ns3::Ipv4Address reachableIP : this->reachableIPs){
		if(reachableIP == ipAddr){
			return true;
		}
	}
	return false;
}

const bool NeighborEntry::containsNeighbor(int nodeId) const {
	for(int neighborId : this->reachableNodeIds){
		if(neighborId == nodeId){
			return true;
		}
	}
	return false;
}

void NeighborEntry::addSampleToETX(int sample) {
	etxSample.push_back(sample);
	lossRate = calculateLossRate();
}

void NeighborEntry::addDelayToMovingAvg(long delay) {
	avgDelay.push_back(delay);
}

void NeighborEntry::addJitterToMovingAvg(long jitter) {
	avgJitter.push_back(jitter);
}

const double NeighborEntry::getAverageDelay() const {
	boost::circular_buffer<long>::const_iterator it = avgDelay.begin();
	double sum = 0.0;
	for(it = avgDelay.begin(); it != avgDelay.end(); ++it){
		sum += *it;
	}
	return sum / avgDelay.size();
}

const double NeighborEntry::getAverageJitter() const {
	boost::circular_buffer<long>::const_iterator it = avgJitter.begin();
	double sum = 0.0;
	for(it = avgJitter.begin(); it != avgJitter.end(); ++it){
		sum += *it;
	}
	return sum / avgJitter.size();
}

const int NeighborEntry::getNodeId() const {
	return this->nodeId;
}
void NeighborEntry::setNodeId(int nodeId) {
	this->nodeId = nodeId;
}

void NeighborEntry::setIp(const ns3::Ipv4Address ipAddr) {
	this->ipAddr = ipAddr;
}

const ns3::Ipv4Address NeighborEntry::getIp() const {
	return this->ipAddr;
}

const ns3::Time& NeighborEntry::getDmSendTime() const {
	return dmSendTime;
}

void NeighborEntry::setDmSendTime(const ns3::Time& dmSendTime) {
	this->dmSendTime = dmSendTime;
}

int NeighborEntry::getDmSeqNo() const {
	return dmSeqNo;
}

void NeighborEntry::setDmSeqNo(int dmSeqNo) {
	this->dmSeqNo = dmSeqNo;
}

void NeighborEntry::incrementDmSeqNo() {
	this->dmSeqNo++;
}

const std::string NeighborEntry::toString() const {
	std::stringstream ss;
	ss << format("%3d%15s %8.1f %8.1f %8.1f %6.1f %6.1f %6.1f  ")
			% this->nodeId
			% this->ipAddr
			% this->allocBw
			% this->occupiedBw
			% this->avgOccupiedBw
			% this->getAverageDelay()
			% this->getAverageJitter()
			% this->getLossRate();

	for(size_t i=0; i<this->reachableNodeIds.size(); i++){
		if(i + 1 < this->reachableNodeIds.size())
			ss << this->reachableNodeIds[i] << ",";
		else
			ss << this->reachableNodeIds[i];
	}
	ss << "\n";

	return ss.str();
}

const std::vector<int>& NeighborEntry::getReachableNodeIds() const {
	return reachableNodeIds;
}

void NeighborEntry::setReachableNodeIds(
		const std::vector<int>& reachableNodeIds) {
	this->reachableNodeIds.resize((int)reachableNodeIds.size());
	std::copy(reachableNodeIds.begin(), reachableNodeIds.end(), this->reachableNodeIds.begin());
}
