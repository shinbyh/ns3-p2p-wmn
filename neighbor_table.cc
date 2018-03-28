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
#include "my_config.h"

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

	if(this->ncTable.find(hello.getNodeId()) == this->ncTable.end()){
		// Neighbor entry not found.
		cout << " [Node "<< node->GetId() << "][NT] adding a new NeighborEntry: " << ipAddr << " (nodeId: " << hello.getNodeId() << ")" << endl;
		entry = new NeighborEntry(hello.getNodeId(), ipAddr, hello.isIsRouter());

		this->ncTable[entry->getNodeId()] = entry;
		this->keysTimeOrder.push_back(hello.getNodeId());
	} else {
		// Existing neighbor entry.
		entry = this->ncTable[hello.getNodeId()];
	}

	entry->setIsActive(true);
	entry->setReachableNodeIds(hello.getNeighbors());
	entry->addSampleToETX(hello.getSeqNo());
	//entry->setNumOfFlows(hello.getNumOfFlows()); // not used (2018.01.02)
	entry->setOccupiedBw(hello.getOccBw());
	entry->setAllocBw(hello.getAllocBw());

	if(this->myNode->getScheme() == SCHEME_LOCAL_REPAIR_AVG_BW){
		/*
		 * SCHEME_LOCAL_REPAIR_AVG_BW.
		 * Get average occupied bandwidth of a neighbor.
		 */
		entry->setAvgOccupiedBw(hello.getAvgOccBw());
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

/*double NeighborTable::getAllocatedBW(uint32_t nodeId) {
	if(ncTable.find(nodeId) == ncTable.end()){
		// not found
		return 0.0;
	} else {
		NeighborEntry* entry = ncTable[nodeId];
		return entry->getAllocBw();
	}
}

double NeighborTable::getAvgOccupiedBW() {
	double sum = 0.0;
	int count = 0;

	pair<uint32_t, NeighborEntry*> p;
	BOOST_FOREACH (p, ncTable){
		NeighborEntry* entry = p.second;
		sum += entry->getOccupiedBw();
		count++;
	}

	if(count == 0) return 0.0;
	else return sum/(double)count;
}*/

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

const std::string NeighborTable::printNeighborTable(ns3::Ptr<ns3::Node> node) const {
	std::stringstream ss;
	ss << "[Node " << node->GetId() << "] NeighborTable (t=" << Simulator::Now().GetSeconds() << ")\n"
		<< " id       IP        allocBW    occBW  avgResBW  delay jitter   loss numFlows neighbors\n"
		<< "------------------------------------------------------------------------------------------\n";

	pair<uint32_t, NeighborEntry*> p;
	BOOST_FOREACH (p, ncTable){
		NeighborEntry* entry = p.second;
		ss << entry->toString();
	}

	return ss.str();
}

std::vector<uint32_t> NeighborTable::getNeighborIDs() {
	vector<uint32_t> nodes;

	pair<uint32_t, NeighborEntry*> p;
	BOOST_FOREACH (p, this->ncTable){
		NeighborEntry* entry = p.second;
		if(entry){
			nodes.push_back(entry->getNodeId());
		}
	}
	return nodes;
}

const std::map<uint32_t, NeighborEntry*> NeighborTable::getMap(){
	return this->ncTable;
}

/**
 * Find a set of neighbors whose neighbor contains the node ID.
 * This list is used for selecting candidate detour nodes.
 * @param nodeId A node ID which should be the neighbor of candidate nodes.
 * @param trace A previous source route. This is used for filtering out candidates already in a path.
 */
std::vector<NeighborEntry*> NeighborTable::getDetourNodes(uint32_t nodeId, std::vector<uint32_t> trace) {
	vector<NeighborEntry*> detours;
	pair<uint32_t, NeighborEntry*> p;
	BOOST_FOREACH (p, this->ncTable){
		NeighborEntry* entry = p.second;
		if(entry){
			if(entry->containsNeighbor(nodeId)){
				if(std::find(trace.begin(), trace.end(), entry->getNodeId()) == trace.end()){
					detours.push_back(entry);
				}
			}
		}
	}
	return detours;
}


