/*
 * route_arreqtable.cc
 *
 *  Created on: Apr 19, 2017
 *      Author: bhshin
 */

#include "route_arreqtable.h"

ARREQSentTable::ARREQSentTable() {
}

ARREQSentTable::~ARREQSentTable() {
	std::map<Flow, ARREQSentEntry*>::iterator itr;
	for(itr = table.begin(); itr != table.end(); itr++){
		delete itr->second;
	}
}

void ARREQSentTable::addEntry(Flow flow, int seqNo) {
	if(this->table.find(flow) == this->table.end()){
		// not found
		ARREQSentEntry* entry = new ARREQSentEntry(flow, seqNo);
		this->table[flow] = entry;
	} else {
		// found
		ARREQSentEntry* entry = this->table[flow];
		if(entry->getSeqNo() < seqNo){
			entry->setSeqNo(seqNo);
		}
	}
}

ARREQSentEntry* ARREQSentTable::getEntry(Flow flow) {
	if(this->table.find(flow) == this->table.end()){
		return 0;
	} else {
		return this->table[flow];
	}
}

int ARREQSentTable::getNextSeqNo(Flow flow) {
	if(this->table.find(flow) == this->table.end()){
		// not found. add a new entry.
		addEntry(flow, 0);
		return 0;
	} else {
		// the entry exists. increment seqNo.
		ARREQSentEntry* entry = this->table[flow];
		entry->incrementSeqNo();
		return entry->getSeqNo();
	}
}

const Flow& ARREQSentEntry::getFlow() const {
	return flow;
}

int ARREQSentEntry::getSeqNo() const {
	return seqNo;
}

ARREQSentEntry::ARREQSentEntry(Flow flow, int seqNo) {
	this->flow = flow;
	this->seqNo = seqNo;
}

ARREQSentEntry::~ARREQSentEntry() {
}

void ARREQSentEntry::setSeqNo(int seqNo) {
	this->seqNo = seqNo;
}

void ARREQSentEntry::incrementSeqNo() {
	this->seqNo++;
}
