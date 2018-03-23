/*
 * flow_stat.cc
 *
 *  Created on: Oct 27, 2016
 *      Author: bhshin
 */

#include "flow_stat.h"
#include "my_config.h" // CIRCULAR_BUFFER_SIZE_FLOWSTAT
#include <boost/circular_buffer.hpp>
#include <sstream>


FlowStat::FlowStat() {
	init();
}

FlowStat::~FlowStat() {

}

void FlowStat::init() {
	this->cbd.set_capacity(CIRCULAR_BUFFER_SIZE_FLOWSTAT);
	this->totalBytes = 0;
	this->totalPackets = 0;
	this->realTimeBandwidth = 0;
	this->realTimePackets = 0;
	this->allocatedBandwidth = 0.0;
	this->numOfFlows = 0;
	//this->used = true;
}

double FlowStat::getAllocatedBandwidth() const {
	return allocatedBandwidth;
}

void FlowStat::setAllocatedBandwidth(double allocatedBandwidth) {
	this->allocatedBandwidth = allocatedBandwidth;
}

int FlowStat::getNumOfFlows() const {
	return numOfFlows;
}

void FlowStat::setNumOfFlows(int numOfFlows) {
	this->numOfFlows = numOfFlows;
}

int FlowStat::getRealTimeBandwidth() const {
	return realTimeBandwidth;
}

void FlowStat::setRealTimeBandwidth(int realTimeBandwidth) {
	this->realTimeBandwidth = realTimeBandwidth;
}

int FlowStat::getRealTimePackets() const {
	return realTimePackets;
}

void FlowStat::setRealTimePackets(int realTimePackets) {
	this->realTimePackets = realTimePackets;
}

int FlowStat::getTotalBytes() const {
	return totalBytes;
}

void FlowStat::setTotalBytes(int totalBytes) {
	this->totalBytes = totalBytes;
}

int FlowStat::getTotalPackets() const {
	return totalPackets;
}

void FlowStat::setTotalPackets(int totalPackets) {
	this->totalPackets = totalPackets;
}

/*bool FlowStat::isUsed() {
	return this->used;
}

void FlowStat::setUsed(bool used) {
	this->used = used;
}*/

void FlowStat::incrementNumOfFlows(int level) {
	this->numOfFlows += level;
}

void FlowStat::decrementNumOfFlows(int level) {
	this->numOfFlows -= level;
}

void FlowStat::accumulatePacket(PacketInfo pktInfo) {
	this->totalBytes += pktInfo.getBytes();
	this->totalPackets++;
	this->realTimeBandwidth += pktInfo.getBytes() * 8;
	this->realTimePackets++;
}

void FlowStat::addToMovingAvg(double realTimeBandwidth){
	this->cbd.push_back(realTimeBandwidth);
}

double FlowStat::getAvgRealTimeBandwidth() {
	if(this->cbd.size() == 0)
		return 0.0;

	boost::circular_buffer<double>::iterator it = this->cbd.begin();
	double sum = 0.0;
	for(it = this->cbd.begin(); it != this->cbd.end(); ++it){
		sum += *it;
	}
	return sum / this->cbd.size();
}

std::string FlowStat::toString() {
	std::stringstream ss;
	ss << "totalPkts=" << totalPackets << ", totalBytes=" << totalBytes
		<< ", RTPkts=" << realTimePackets <<", RTBW=" << realTimeBandwidth
		<< ", allocBW=" << allocatedBandwidth << ", #flows=" << numOfFlows;
	return ss.str();
}
