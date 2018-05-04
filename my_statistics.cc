/*
 * my_statistics.cc
 *
 *  Created on: May 3, 2018
 *      Author: bhshin
 */

#include "my_statistics.h"
#include <sstream>
#include <fstream>
#include <boost/foreach.hpp>

MyStatistics::MyStatistics() {

}

MyStatistics::~MyStatistics() {
	pair<Flow, FlowGoodput*> p;
	BOOST_FOREACH(p, this->flowGoodputMap){
		delete p.second;
	}
}

void MyStatistics::addGoodput(Flow flow, double goodput) {
	if(this->flowGoodputMap.find(flow) == this->flowGoodputMap.end()){
		FlowGoodput* fg = new FlowGoodput();
		fg->addGoodput(goodput);
		this->flowGoodputMap[flow] = fg;
	} else {
		FlowGoodput* fg = this->flowGoodputMap[flow];
		if(fg){
			fg->addGoodput(goodput);
		} else {
			FlowGoodput* newFg = new FlowGoodput();
			newFg->addGoodput(goodput);
			this->flowGoodputMap[flow] = newFg;
		}
	}
}

void MyStatistics::incrementQoSViolationCount(Flow flow, uint32_t nodeId) {
	if(this->flowGoodputMap.find(flow) == this->flowGoodputMap.end()){
		FlowGoodput* fg = new FlowGoodput();
		fg->incrementQoSViolationCount(nodeId);
		this->flowGoodputMap[flow] = fg;
	} else {
		FlowGoodput* fg = this->flowGoodputMap[flow];
		if(fg){
			fg->incrementQoSViolationCount(nodeId);
		} else {
			FlowGoodput* newFg = new FlowGoodput();
			newFg->incrementQoSViolationCount(nodeId);
			this->flowGoodputMap[flow] = newFg;
		}
	}
}

void MyStatistics::incrementLocalRepairCount(Flow flow, uint32_t nodeId) {
	if(this->flowGoodputMap.find(flow) == this->flowGoodputMap.end()){
		FlowGoodput* fg = new FlowGoodput();
		fg->incrementLocalRepairCount(nodeId);
		this->flowGoodputMap[flow] = fg;
	} else {
		FlowGoodput* fg = this->flowGoodputMap[flow];
		if(fg){
			fg->incrementLocalRepairCount(nodeId);
		} else {
			FlowGoodput* newFg = new FlowGoodput();
			newFg->incrementLocalRepairCount(nodeId);
			this->flowGoodputMap[flow] = newFg;
		}
	}
}

void MyStatistics::incrementRtDiscoveryCount(Flow flow) {
	if(this->flowGoodputMap.find(flow) == this->flowGoodputMap.end()){
		FlowGoodput* fg = new FlowGoodput();
		fg->incrementRtDiscoveryCount();
		this->flowGoodputMap[flow] = fg;
	} else {
		FlowGoodput* fg = this->flowGoodputMap[flow];
		if(fg){
			fg->incrementRtDiscoveryCount();
		} else {
			FlowGoodput* newFg = new FlowGoodput();
			newFg->incrementRtDiscoveryCount();
			this->flowGoodputMap[flow] = newFg;
		}
	}
}

void MyStatistics::writeFlowGoodputStatistics() {
	ofstream flowGoodputOut;
	flowGoodputOut.open("flow_goodput.txt", ofstream::out);
	ofstream flowGoodputQvOut;
	flowGoodputQvOut.open("flow_goodput_qv.txt", ofstream::out);
	ofstream flowGoodputQvAvgOut;
	flowGoodputQvAvgOut.open("flow_goodput_qv_sum.txt", ofstream::out);

	double sumGoodputQv = 0.0;
	int qvCount = 0;
	pair<Flow, FlowGoodput*> p;
	BOOST_FOREACH(p, this->flowGoodputMap){
		FlowGoodput* fg = p.second;
		flowGoodputOut << p.first.toFormattedString() << "\t"
				<< fg->getTotalQosViolationCount() << "\t"
				<< fg->getTotalLocalRepairCount() << "\t"
				<< fg->getRtDiscoveryCount() << "\t"
				<< fg->getAverageGoodput() << "\n";

		// Write statistics only when QoS violations occurred.
		if(fg->getTotalQosViolationCount() > 0){
			flowGoodputQvOut << p.first.toFormattedString() << "\t"
					<< fg->getTotalQosViolationCount() << "\t"
					<< fg->getTotalLocalRepairCount() << "\t"
					<< fg->getRtDiscoveryCount() << "\t"
					<< fg->getAverageGoodput() << "\n";

			sumGoodputQv += fg->getAverageGoodput();
			qvCount++;
		}
	}

	if(qvCount == 0){
		flowGoodputQvAvgOut << "0.0\t0.0\n";
	} else {
		flowGoodputQvAvgOut << sumGoodputQv << "\t" << sumGoodputQv/(double)qvCount << "\n";
	}

	flowGoodputOut.close();
	flowGoodputQvOut.close();
	flowGoodputQvAvgOut.close();
}

FlowGoodput::FlowGoodput() {
	this->rtDiscoveryCount = 0;
}

FlowGoodput::~FlowGoodput() {
}

void FlowGoodput::addGoodput(double goodput) {
	this->goodputs.push_back(goodput);
}

double FlowGoodput::getAverageGoodput() {
	int count = this->goodputs.size();
	if(count == 0) return 0.0;

	double sum = 0.0;
	for(double goodput : this->goodputs){
		sum += goodput;
	}
	return sum / (double)count;
}

const vector<double>& FlowGoodput::getGoodputs() const {
	return goodputs;
}

void FlowGoodput::setGoodputs(const vector<double>& goodputs) {
	this->goodputs = goodputs;
}

int FlowGoodput::getTotalLocalRepairCount() const {
	int sum = 0;
	pair<uint32_t, int> p;
	BOOST_FOREACH(p, this->localRepairCount){
		sum += p.second;
	}
	return sum;
}

void FlowGoodput::setLocalRepairCount(uint32_t nodeId, int localRepairCount) {
	this->localRepairCount[nodeId] = localRepairCount;
}

int FlowGoodput::getTotalQosViolationCount() const {
	int sum = 0;
	pair<uint32_t, int> p;
	BOOST_FOREACH(p, this->qosViolationCount){
		sum += p.second;
	}
	return sum;
}

void FlowGoodput::setQosViolationCount(uint32_t nodeId, int qosViolationCount) {
	this->qosViolationCount[nodeId] = qosViolationCount;
}

int FlowGoodput::getRtDiscoveryCount() const {
	return rtDiscoveryCount;
}

void FlowGoodput::incrementLocalRepairCount(uint32_t nodeId) {
	if(this->localRepairCount.find(nodeId) == this->localRepairCount.end()){
		this->localRepairCount[nodeId] = 1;
	} else {
		int count = this->localRepairCount[nodeId] + 1;
		this->localRepairCount[nodeId] = count;
	}
}

void FlowGoodput::incrementQoSViolationCount(uint32_t nodeId) {
	if(this->qosViolationCount.find(nodeId) == this->qosViolationCount.end()){
		this->qosViolationCount[nodeId] = 1;
	} else {
		int count = this->qosViolationCount[nodeId] + 1;
		this->qosViolationCount[nodeId] = count;
	}
}

void FlowGoodput::setRtDiscoveryCount(int rtDiscoveryCount) {
	this->rtDiscoveryCount = rtDiscoveryCount;
}

void FlowGoodput::incrementRtDiscoveryCount() {
	this->rtDiscoveryCount++;
}
