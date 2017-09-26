/*
 * ctrl_flowtable.cc
 *
 *  Created on: Feb 2, 2017
 *      Author: bhshin
 */

#include "ctrl_flowtable.h"
#include <boost/foreach.hpp>
#include <sstream>

ControlFlowTable::ControlFlowTable() {

}

ControlFlowTable::~ControlFlowTable() {
	std::map<int, ControlFlow*>::iterator itr;
	for(itr = ctrlFlowTable.begin(); itr != ctrlFlowTable.end(); itr++){
		delete itr->second;
	}
}

void ControlFlowTable::add(bool isTx, PacketInfo* pktInfo) {
	int ctrlPort = pktInfo->getFlow().getDstPort();
	ControlFlow* ctrlFlow;

	if(this->ctrlFlowTable.find(ctrlPort) == this->ctrlFlowTable.end()){
		// not found, adding a new entry
		ctrlFlow = new ControlFlow(ctrlPort);
		this->ctrlFlowTable[ctrlPort] = ctrlFlow;
		this->keysTimeOrder.push_back(ctrlPort);
	} else {
		// an existing port number
		ctrlFlow = this->ctrlFlowTable[ctrlPort];
	}
	ctrlFlow->addPacketInfo(isTx, pktInfo);
}

void ControlFlowTable::updateRealTimeBandwidth() {
	std::pair<int, ControlFlow*> p;
	BOOST_FOREACH (p, this->ctrlFlowTable){
		ControlFlow* ctrlFlow = p.second;
		ctrlFlow->resetRealTimeBandwidth();
	}
}

std::string ControlFlowTable::getControlOutput(std::string time) {
	std::stringstream ss;
	ss << std::fixed << time;
	for(int ctrlPort : this->keysTimeOrder){
		ControlFlow* ctrlFlow = this->ctrlFlowTable[ctrlPort];
		ss << "\t" << ctrlFlow->getRealTimeBandwidth();
	}
	ss << "\n";
	return ss.str();
}

std::string ControlFlowTable::getControlFlowInfo() {
	std::stringstream ss;
	ss << std::fixed;

	for(int ctrlPort : this->keysTimeOrder){
		ss << ctrlPort << "\n";
	}
	return ss.str();
}
