/*
 * node_id_map.h
 *
 *  Created on: Sep 13, 2017
 *      Author: bhshin
 */

#ifndef SCRATCH_P2P_BHSHIN_NODE_ID_MAP_H_
#define SCRATCH_P2P_BHSHIN_NODE_ID_MAP_H_

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include <map>
#include "my_config.h"

class NodeIdMap {
private:
	std::map<ns3::Ipv4Address, uint32_t> nodeIdMap;

public:
	NodeIdMap();
	~NodeIdMap();

	void addMapping(ns3::Ipv4Address ipAddr, uint32_t nodeId);
	uint32_t getNodeId(ns3::Ipv4Address ipAddr);
};

#endif /* SCRATCH_P2P_BHSHIN_NODE_ID_MAP_H_ */
