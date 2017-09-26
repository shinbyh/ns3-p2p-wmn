/*
 * neighbor_table.h
 *
 *  Created on: Dec 18, 2016
 *      Author: bhshin
 */

#ifndef SCRATCH_BHSHIN_NEIGHBOR_TABLE_H_
#define SCRATCH_BHSHIN_NEIGHBOR_TABLE_H_

#include <map>
#include <vector>

#include "ns3/ipv4-address.h"
#include "ns3/applications-module.h"
#include "hello.h"
#include "neighbor_entry.h"
#include "my_config.h"

class MyNode; //reference

class NeighborTable {
private:
	MyNode* myNode;
	std::map<uint32_t, NeighborEntry*> ncTable;
	std::vector<uint32_t> keysTimeOrder;
	std::vector<uint32_t> deleteList;

public:
	NeighborTable(MyNode* myNode);
	~NeighborTable();

	uint32_t getNodeId(ns3::Ipv4Address ipAddr);
	void addAndUpdate(ns3::Ptr<ns3::Node> node, ns3::Ipv4Address ipAddr, Hello hello);
	void deleteEntry(uint32_t nodeId);
	void setInactive(uint32_t nodeId);
	NeighborEntry* get(uint32_t nodeId);
	NeighborEntry* get(ns3::Ipv4Address ipAddr);

	double getAllocatedBW(uint32_t nodeId); // TODO: nodeId
	void updateDelay(uint32_t nodeId, long delay);
	void updateJitter(uint32_t nodeId, long jitter);
	void updateNumFlows(uint32_t nodeId, int flows);
	bool isNeighbor(uint32_t nodeId);
	void checkNeighborTimeout(ns3::Ptr<ns3::Node> node);
	void deleteInactiveNeighbors();
	void checkSendDelayMeasurement();
	const std::string printNeighborTable(ns3::Ptr<ns3::Node> node);
};



#endif /* SCRATCH_BHSHIN_NEIGHBOR_TABLE_H_ */
