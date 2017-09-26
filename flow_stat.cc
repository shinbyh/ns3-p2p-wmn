/*
 * flow_stat.cc
 *
 *  Created on: Oct 27, 2016
 *      Author: bhshin
 */

#include "flow_stat.h"

#include <boost/circular_buffer.hpp>
#include <sstream>


FlowStat::FlowStat() {
	init();
}

FlowStat::~FlowStat() {

}

void FlowStat::init() {
	cbd.set_capacity(CIRCULAR_BUFFER_SIZE);
	totalBytes = 0;
	totalPackets = 0;
	realTimeBandwidth = 0;
	realTimePackets = 0;
	allocatedBandwidth = 0.0;
	numOfFlows = 0;
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

void FlowStat::incrementNumOfFlows(int level) {
	numOfFlows += level;
}

void FlowStat::decrementNumOfFlows(int level) {
	numOfFlows -= level;
}

void FlowStat::accumulatePacket(PacketInfo pktInfo) {
	totalBytes += pktInfo.getBytes();
	totalPackets++;
	realTimeBandwidth += pktInfo.getBytes() * 8;
	realTimePackets++;
}

void FlowStat::addToMovingAvg(double realTimeBandwidth){
	this->cbd.push_back(realTimeBandwidth);
}

double FlowStat::getAvgRealTimeBandwidth() {
	if(cbd.size() == 0)
		return 0.0;

	boost::circular_buffer<double>::iterator it = cbd.begin();
	double sum = 0.0;
	for(it = cbd.begin(); it != cbd.end(); ++it){
		sum += *it;
	}
	return sum / cbd.size();
}

std::string FlowStat::toString() {
	std::stringstream ss;
	ss << "totalPkts=" << totalPackets << ", totalBytes=" << totalBytes
		<< ", RTPkts=" << realTimePackets <<", RTBW=" << realTimeBandwidth
		<< ", allocBW=" << allocatedBandwidth << ", #flows=" << numOfFlows;
	return ss.str();
}
