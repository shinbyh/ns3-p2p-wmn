/*
 * node_id_map.cc
 *
 *  Created on: Sep 13, 2017
 *      Author: bhshin
 */

#include "node_id_map.h"

NodeIdMap::NodeIdMap() {
	// TODO Auto-generated constructor stub

}

NodeIdMap::~NodeIdMap() {
	// TODO Auto-generated destructor stub
}

void NodeIdMap::addMapping(ns3::Ipv4Address ipAddr, uint32_t nodeId) {
	this->nodeIdMap[ipAddr] = nodeId;
}

uint32_t NodeIdMap::getNodeId(ns3::Ipv4Address ipAddr) {
	if(this->nodeIdMap.find(ipAddr) == this->nodeIdMap.end()){
		return NODEID_NOT_FOUND;
	}

	return this->nodeIdMap[ipAddr];
}
