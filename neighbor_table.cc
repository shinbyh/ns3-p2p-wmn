/*
 * neighbor_table.cc
 *
 *  Created on: Dec 18, 2016
 *      Author: bhshin
 */

#include "ns3/core-module.h"
#include "ns3/nstime.h"
#include "ns3/internet-apps-module.h"
#include "ns3/internet-module.h"
#include "neighbor_table.h"
#include "delay_measurement.h"
#include "my_node.h"

#include <boost/foreach.hpp>
#include <sstream>

using namespace ns3;

NeighborTable::NeighborTable(MyNode* myNode) {
	this->myNode = myNode;
}

NeighborTable::~NeighborTable() {
	std::map<uint32_t, NeighborEntry*>::iterator itr;
	for(itr = ncTable.begin(); itr != ncTable.end(); itr++){
		delete itr->second;
	}
}

uint32_t NeighborTable::getNodeId(ns3::Ipv4Address ipAddr) {
	pair<uint32_t, NeighborEntry*> p;
	BOOST_FOREACH (p, ncTable){
		NeighborEntry* entry = p.second;
		if(entry->getIp() == ipAddr){
			return p.first;
		}
	}

	// not found
	return NODEID_NOT_FOUND;
}

void NeighborTable::addAndUpdate(ns3::Ptr<ns3::Node> node, ns3::Ipv4Address ipAddr, Hello hello) {
	NeighborEntry* entry;

	if(ncTable.find(hello.getNodeId()) == ncTable.end()){
		// not found
		cout << " [Node "<< node->GetId() << "][NT] adding a new NeighborEntry: " << ipAddr << " (nodeId: " << hello.getNodeId() << ")" << endl;
		entry = new NeighborEntry(hello.getNodeId(), ipAddr, hello.isIsRouter());
		entry->setIsActive(true);
		entry->addSampleToETX(hello.getSeqNo());
		entry->setNumOfFlows(hello.getNumOfFlows());
		entry->setOccupiedBw(hello.getOccBw());
		entry->setAllocBw(hello.getAllocBw());
		//reachable IPs

		this->ncTable[entry->getNodeId()] = entry;
		this->keysTimeOrder.push_back(hello.getNodeId());

	} else {
		entry = ncTable[hello.getNodeId()];
		if(!entry->isIsActive()){
			// re-activate entry
			entry->setIsActive(true);
		}
		entry->addSampleToETX(hello.getSeqNo());
		entry->setNumOfFlows(hello.getNumOfFlows());
		entry->setOccupiedBw(hello.getOccBw());
		entry->setAllocBw(hello.getAllocBw());
		//reachable IPs
	}

	entry->setLastUpdateTime(Simulator::Now().GetMilliSeconds());
}

void NeighborTable::deleteEntry(uint32_t nodeId) {
	ncTable.erase(nodeId);

	int keyIdx = -1;
	for(size_t i=0; i<keysTimeOrder.size(); i++){
		if(keysTimeOrder[i] == nodeId){
			keyIdx = i;
			break;
		}
	}
	if(keyIdx > 0){
		keysTimeOrder.erase(keysTimeOrder.begin() + keyIdx);
	}
}

void NeighborTable::setInactive(uint32_t nodeId) {
	if(ncTable.find(nodeId) == ncTable.end()){
		// not found
	} else {
		NeighborEntry* entry = ncTable[nodeId];
		entry->setIsActive(false);
	}
}

NeighborEntry* NeighborTable::get(uint32_t nodeId) {
	if(ncTable.find(nodeId) == ncTable.end()){
		return 0;
	} else {
		return ncTable[nodeId];
	}
}

NeighborEntry* NeighborTable::get(ns3::Ipv4Address ipAddr) {
	uint32_t nodeId = getNodeId(ipAddr);
	if(nodeId == NODEID_NOT_FOUND){
		return NULL;
	}

	if(ncTable.find(nodeId) == ncTable.end()){
		return NULL;
	} else {
		return ncTable[nodeId];
	}
}

double NeighborTable::getAllocatedBW(uint32_t nodeId) {
	if(ncTable.find(nodeId) == ncTable.end()){
		// not found
		return 0.0;
	} else {
		NeighborEntry* entry = ncTable[nodeId];
		return entry->getAllocBw();
	}
}

void NeighborTable::updateDelay(uint32_t nodeId, long delay) {
	if(ncTable.find(nodeId) == ncTable.end()){
		// not found
	} else {
		NeighborEntry* entry = ncTable[nodeId];
		entry->setDelay(delay);
		entry->addDelayToMovingAvg(delay);
		entry->setLastUpdateTime(Simulator::Now().GetMilliSeconds());
		entry->setIsActive(true);
	}
}

void NeighborTable::updateJitter(uint32_t nodeId, long jitter) {
	if(ncTable.find(nodeId) == ncTable.end()){
		// not found
	} else {
		NeighborEntry* entry = ncTable[nodeId];
		entry->addJitterToMovingAvg(jitter);
		entry->setLastUpdateTime(Simulator::Now().GetMilliSeconds());
	}
}

void NeighborTable::updateNumFlows(uint32_t nodeId, int flows) {
	if(ncTable.find(nodeId) == ncTable.end()){
		// not found
	} else {
		NeighborEntry* entry = ncTable[nodeId];
		entry->setNumOfFlows(flows);
		entry->setLastUpdateTime(Simulator::Now().GetMilliSeconds());
	}
}

bool NeighborTable::isNeighbor(uint32_t nodeId) {
	if(ncTable.find(nodeId) == ncTable.end()){
		// not found
		return false;
	} else {
		return true;
	}
}

void NeighborTable::checkNeighborTimeout(ns3::Ptr<ns3::Node> node) {
	ns3::Time now = Simulator::Now();

	pair<uint32_t, NeighborEntry*> p;
	BOOST_FOREACH (p, ncTable){
		NeighborEntry* entry = p.second;
		if(entry->isIsActive() && now.GetMilliSeconds() - entry->getLastUpdateTime() > 5000){
			entry->setIsActive(false);

			//debug
			//cout << "[Node " << node->GetId() << "] set inactive: " << entry->getIp() << endl;
		}

		if( !entry->isIsActive() && now.GetMilliSeconds() - entry->getLastUpdateTime() > 10000){
			this->deleteList.push_back(entry->getNodeId());
		}
	}
}

void NeighborTable::deleteInactiveNeighbors(){
	for(size_t i=0; i<deleteList.size(); i++){
		deleteEntry(deleteList[i]);
	}
	deleteList.clear();
}

void NeighborTable::checkSendDelayMeasurement() {
	int pktSize = 200;
	pair<uint32_t, NeighborEntry*> p;
	BOOST_FOREACH (p, this->ncTable){
		NeighborEntry* entry = p.second;
		if(entry){
			if(entry->isIsActive()){
				// check DM client activated and run if inactive
				DelayMeasurement dm("req", entry->getDmSeqNo(), Simulator::Now());
				entry->setDmSendTime(Simulator::Now());
				this->myNode->sendDelayMeasurement(entry->getIp(), dm, pktSize);
			}
		}
	}
}

const std::string NeighborTable::printNeighborTable(ns3::Ptr<ns3::Node> node) {
	std::stringstream ss;
	ss << "[Node " << node->GetId() << "] NeighborTable (t=" << Simulator::Now().GetSeconds() << ")\n"
		<< " id       IP        allocBW    occBW  delay jitter  loss\n"
		<< "-----------------------------------------------------------------\n";

	pair<uint32_t, NeighborEntry*> p;
	BOOST_FOREACH (p, ncTable){
		NeighborEntry* entry = p.second;
		ss << entry->toString();
	}

	return ss.str();
}
