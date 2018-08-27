/*
 * route_maintenance_stats.cc
 *
 *  Created on: Jun 25, 2018
 *      Author: bhshin
 */

#include "route_maintenance_stats.h"
#include "my_config.h"
#include "ns3/core-module.h"
#include <fstream>
#include <sstream>

/************************************
 * QoSViolationStat
 ************************************/
QoSViolationStat::QoSViolationStat() {
	this->qvFoundNode = NODEID_NOT_FOUND;
	this->qvStart = ns3::Seconds(0);
	this->qvEnd = ns3::Seconds(0);
	this->resolved = false;
}

QoSViolationStat::~QoSViolationStat() {
}

const ns3::Time& QoSViolationStat::getQvEnd() const {
	return qvEnd;
}

void QoSViolationStat::setQvEnd(const ns3::Time& qvEnd) {
	this->qvEnd = qvEnd;
}

uint32_t QoSViolationStat::getQvFoundNode() const {
	return qvFoundNode;
}

void QoSViolationStat::setQvFoundNode(uint32_t qvFoundNode) {
	this->qvFoundNode = qvFoundNode;
}

const ns3::Time& QoSViolationStat::getQvStart() const {
	return qvStart;
}

void QoSViolationStat::setQvStart(const ns3::Time& qvStart) {
	this->qvStart = qvStart;
}

ns3::Time QoSViolationStat::getRtMtncDuration() {
	return (this->qvEnd - this->qvStart);
}

bool QoSViolationStat::isResolved() const {
	return resolved;
}

void QoSViolationStat::setResolved(bool resolved) {
	this->resolved = resolved;
}



/************************************
 * RouteMaintenanceStat
 ************************************/
RouteMaintenanceStat::RouteMaintenanceStat() {
}

RouteMaintenanceStat::~RouteMaintenanceStat() {
	map<int, QoSViolationStat*>::iterator itr;
	for(itr = this->qvMap.begin(); itr != this->qvMap.end(); itr++){
		delete itr->second;
	}
}

void RouteMaintenanceStat::addQVStartTime(int seqNo, ns3::Time qvStart) {
	if(this->qvMap.find(seqNo) == this->qvMap.end()){
		// Not found. Create a new one.
		QoSViolationStat* qvStat = new QoSViolationStat();
		this->qvMap[seqNo] = qvStat;
	}
	this->qvMap[seqNo]->setQvStart(qvStart);
}

void RouteMaintenanceStat::addQVEndTime(int seqNo, ns3::Time qvEnd) {
	if(this->qvMap.find(seqNo) == this->qvMap.end()){
		// Not found. Create a new one.
		QoSViolationStat* qvStat = new QoSViolationStat();
		this->qvMap[seqNo] = qvStat;
	}
	this->qvMap[seqNo]->setQvEnd(qvEnd);
}

void RouteMaintenanceStat::setQVFoundNodeId(int seqNo, uint32_t nodeId) {
	if(this->qvMap.find(seqNo) == this->qvMap.end()){
		// Not found. Create a new one.
		QoSViolationStat* qvStat = new QoSViolationStat();
		this->qvMap[seqNo] = qvStat;
	}
	this->qvMap[seqNo]->setQvFoundNode(nodeId);
}

void RouteMaintenanceStat::setQVResolved(int seqNo, bool resolved) {
	if(this->qvMap.find(seqNo) == this->qvMap.end()){
		// Not found. Create a new one.
		QoSViolationStat* qvStat = new QoSViolationStat();
		this->qvMap[seqNo] = qvStat;
	}
	this->qvMap[seqNo]->setResolved(resolved);
}

QoSViolationStat* RouteMaintenanceStat::getQVStat(int seqNo) {
	if(this->qvMap.find(seqNo) == this->qvMap.end()){
		// Not found.
		return NULL;
	}
	return this->qvMap[seqNo];
}

string RouteMaintenanceStat::generateFormattedOut(string header) {
	stringstream ss;

	map<int, QoSViolationStat*>::iterator itr;
	for(itr = this->qvMap.begin(); itr != this->qvMap.end(); itr++){
		QoSViolationStat* qvStat = itr->second;
		ss << header << "\t"
				<< itr->first << "\t"
				<< qvStat->getQvFoundNode() << "\t"
				<< qvStat->getQvStart().GetMilliSeconds() << "\t"
				<< qvStat->getQvEnd().GetMilliSeconds() << "\t"
				<< qvStat->getRtMtncDuration().GetMilliSeconds() << "\t"
				<< qvStat->isResolved() << "\n";
	}
	return ss.str();
}

string RouteMaintenanceStat::generateHeaderOut() {
	return "src\tdst\tflType\tseqNo\tfNode\tqvStart\tqvEnd\tdur\tresolved";
}


/************************************
 * RouteMaintenanceStats
 ************************************/
RouteMaintenanceStats::RouteMaintenanceStats() {

}

RouteMaintenanceStats::~RouteMaintenanceStats() {
	map<Flow, RouteMaintenanceStat*>::iterator itr;
	for(itr = this->rtMtncMap.begin(); itr != this->rtMtncMap.end(); itr++){
		delete itr->second;
	}
}

void RouteMaintenanceStats::addQVStartTime(Flow flow, int seqNo, ns3::Time qvStart) {
	if(this->rtMtncMap.find(flow) == this->rtMtncMap.end()){
		// not found. create a new one.
		RouteMaintenanceStat* stat = new RouteMaintenanceStat();
		this->rtMtncMap[flow] = stat;
	}
	this->rtMtncMap[flow]->addQVStartTime(seqNo, qvStart);
}

void RouteMaintenanceStats::addQVEndTime(Flow flow, int seqNo, ns3::Time qvEnd) {
	if(this->rtMtncMap.find(flow) == this->rtMtncMap.end()){
		// not found. create a new one.
		RouteMaintenanceStat* stat = new RouteMaintenanceStat();
		this->rtMtncMap[flow] = stat;
	}
	this->rtMtncMap[flow]->addQVEndTime(seqNo, qvEnd);
}

void RouteMaintenanceStats::setQVFoundNodeId(Flow flow, int seqNo, uint32_t nodeId) {
	if(this->rtMtncMap.find(flow) == this->rtMtncMap.end()){
		// not found. create a new one.
		RouteMaintenanceStat* stat = new RouteMaintenanceStat();
		this->rtMtncMap[flow] = stat;
	}
	this->rtMtncMap[flow]->setQVFoundNodeId(seqNo, nodeId);
}

void RouteMaintenanceStats::setQVResolved(Flow flow, int seqNo, bool resolved) {
	if(this->rtMtncMap.find(flow) == this->rtMtncMap.end()){
		// not found. create a new one.
		RouteMaintenanceStat* stat = new RouteMaintenanceStat();
		this->rtMtncMap[flow] = stat;
	}
	this->rtMtncMap[flow]->setQVResolved(seqNo, resolved);
}

void RouteMaintenanceStats::writeStatsToFile(string fileName) {
	ofstream statOut;
	statOut.open(fileName.c_str(), ofstream::out);
	statOut << RouteMaintenanceStat::generateHeaderOut() << "\n";

	map<Flow, RouteMaintenanceStat*>::iterator itr;
	for(itr = this->rtMtncMap.begin(); itr != this->rtMtncMap.end(); itr++){
		RouteMaintenanceStat* rmStat = itr->second;
		statOut << rmStat->generateFormattedOut(itr->first.toFormattedString());
	}
	statOut.close();
}
