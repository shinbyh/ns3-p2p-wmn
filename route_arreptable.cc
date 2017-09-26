/*
 * route_arreptable.cc
 *
 *  Created on: Jan 25, 2017
 *      Author: bhshin
 */

#include "route_arreptable.h"

using namespace ns3;

ARREPRecvEntry::ARREPRecvEntry(Flow flow, int seqNo) {
	this->flow = flow;
	this->seqNo = seqNo;
	this->dupArreqCount = 0;
}

ARREPRecvEntry::ARREPRecvEntry(Flow flow, int seqNo, FlowRequest flowReq) {
	this->flow = flow;
	this->seqNo = seqNo;
	this->dupArreqCount = 0;
	this->flowReq = flowReq;
}

ARREPRecvEntry::~ARREPRecvEntry() {
}

std::vector<ARREP> ARREPRecvEntry::getArrepList() const {
	return arrepList;
}


int ARREPRecvEntry::getDupArreqCount() const {
	return dupArreqCount;
}


const Flow ARREPRecvEntry::getFlow() const {
	return flow;
}

const Time ARREPRecvEntry::getLastRecvTime() const {
	return lastRecvTime;
}

void ARREPRecvEntry::setLastRecvTime(const Time lastRecvTime) {
	this->lastRecvTime = lastRecvTime;
}

int ARREPRecvEntry::getSeqNo() const {
	return seqNo;
}

void ARREPRecvEntry::addARREP(ARREP arrep, Time time) {
	this->arrepList.push_back(arrep);
	this->lastRecvTime = time;
}

void ARREPRecvEntry::incrementDupArreqCount() {
	this->dupArreqCount++;
}

void ARREPRecvEntry::resetDupArreqCount() {
	this->dupArreqCount = 0;
}

void ARREPRecvEntry::setSeqNo(int seqNo) {
	this->seqNo = seqNo;
}

ARREPRecvTable::ARREPRecvTable() {
}

ARREPRecvTable::~ARREPRecvTable() {
	std::map<Flow, ARREPRecvEntry*>::iterator itr;
	for(itr = table.begin(); itr != table.end(); itr++){
		delete itr->second;
	}
}

void ARREPRecvTable::addEntry(ARREPRecvEntry* entry) {
	this->table[entry->getFlow()] = entry;
}

void ARREPRecvTable::addEntry(Flow flow, int seqNo) {
	if(this->table.find(flow) == this->table.end()){
		// not found
		ARREPRecvEntry* entry = new ARREPRecvEntry(flow, seqNo);
		this->table[flow] = entry;
	} else {
		// found
		ARREPRecvEntry* entry = this->table[flow];
		if(entry->getSeqNo() < seqNo){
			entry->setSeqNo(seqNo);
			entry->resetDupArreqCount();
		} else if(entry->getSeqNo() == seqNo){
			entry->incrementDupArreqCount();
		}
	}
}

void ARREPRecvTable::addEntry(Flow flow, int seqNo, FlowRequest flowReq) {
	if(this->table.find(flow) == this->table.end()){
		// not found
		ARREPRecvEntry* entry = new ARREPRecvEntry(flow, seqNo, flowReq);
		this->table[flow] = entry;
	} else {
		// found
		ARREPRecvEntry* entry = this->table[flow];
		if(entry->getSeqNo() < seqNo){
			entry->setSeqNo(seqNo);
			entry->setFlowReq(flowReq);
			entry->resetDupArreqCount();
		} else if(entry->getSeqNo() == seqNo){
			entry->incrementDupArreqCount();
		}
	}
}

void ARREPRecvTable::addARREP(ARREP arrep) {
	if(this->table.find(arrep.getFlow()) != this->table.end()){
		// found
		ARREPRecvEntry* entry = this->table[arrep.getFlow()];
		if(entry && entry->getSeqNo() == arrep.getSeqNo()){
			entry->addARREP(arrep, Simulator::Now());
		}
	}
}

void ARREPRecvTable::deleteEntry(Flow flow) {
	this->table.erase(flow);
}

ARREPRecvEntry* ARREPRecvTable::getEntry(Flow flow) {
	if(this->table.find(flow) == this->table.end()){
		return 0;
	} else {
		return this->table[flow];
	}
}

const FlowRequest& ARREPRecvEntry::getFlowReq() const {
	return flowReq;
}

void ARREPRecvEntry::setFlowReq(const FlowRequest& flowReq) {
	this->flowReq = flowReq;
}

void ARREPRecvTable::printTable(){
	std::map<Flow, ARREPRecvEntry*>::iterator itr;
	for(itr = table.begin(); itr != table.end(); itr++){
		ARREPRecvEntry* entry = itr->second;
		cout << "\t" << entry->getFlow().toString() << " / seqNo:" << entry->getSeqNo() << " / dupC:" << entry->getDupArreqCount() << endl;
	}
}
