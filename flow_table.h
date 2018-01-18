/*
 * flow_table.h
 *
 *  Created on: Oct 27, 2016
 *      Author: bhshin
 */

#ifndef FLOW_FLOW_TABLE_H_
#define FLOW_FLOW_TABLE_H_

#include <map>
#include <vector>
#include "flow_entry.h"
#include "qos_requirement.h"

class FlowTable {
private:
	std::map<Flow, FlowEntry*> flowTable;
	std::vector<Flow> keysTimeOrder;

public:
	FlowTable();
	FlowTable(const FlowTable& other); // prevent construction by copying
	static FlowTable* instance() {
		static FlowTable inst;
		return &inst;
	}
	~FlowTable();

	void addToFlowEntry(int nodeId, PacketInfo pktInfo);
	void updateRealTimeBandwidth();
	void setQoSReq(Flow flow, int ifIdx, QoSRequirement qosReq);
	void setHopQoSReq(Flow flow, int ifIdx, QoSRequirement qosReq);
	FlowEntry* getFlowEntry(Flow flow);
	void markInactiveFlows(long from, long threshold);
	void setFlowInactive(Flow flow);
	std::vector<FlowEntry*> getAllFlowEntries();
	int getNumOfFlows();
	std::vector<FlowEntry*> getUnpopularFlows(size_t k);

	//vector<FlowEntry> getInactiveFlows();
	//void deleteFlowEntry(Flow flow);
	std::string getAllFlowInfo();
	std::string getFormattedFlowOutput(std::string time);
	std::string printFlowTable(int nodeId);
};



#endif /* FLOW_FLOW_TABLE_H_ */
