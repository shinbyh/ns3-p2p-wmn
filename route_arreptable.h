/*
 * route_arreptable.h
 *
 *  Created on: Jan 25, 2017
 *      Author: bhshin
 */

#ifndef SCRATCH_BHSHIN_ROUTE_ARREPTABLE_H_
#define SCRATCH_BHSHIN_ROUTE_ARREPTABLE_H_

#include <map>
#include <vector>

#include "route_arrep.h"
#include "flow.h"
#include "flow_request.h"
#include "ns3/core-module.h"

class ARREPRecvEntry {
private:
	Flow flow;
	std::vector<ARREP> arrepList;
	ns3::Time lastRecvTime;
	int seqNo;
	int dupArreqCount;
	FlowRequest flowReq;
public:
	ARREPRecvEntry(Flow flow, int seqNo);
	ARREPRecvEntry(Flow flow, int seqNo, FlowRequest flowReq);
	virtual ~ARREPRecvEntry();
	std::vector<ARREP> getArrepList() const;
	void addARREP(ARREP arrep, ns3::Time time);
	int getDupArreqCount() const;
	void incrementDupArreqCount();
	void resetDupArreqCount();
	const Flow getFlow() const;
	const ns3::Time getLastRecvTime() const;
	void setLastRecvTime(const ns3::Time lastRecvTime);
	int getSeqNo() const;
	void setSeqNo(int seqNo);
	const FlowRequest& getFlowReq() const;
	void setFlowReq(const FlowRequest& flowReq);
};

class ARREPRecvTable {
private:
	map<Flow, ARREPRecvEntry*> table;
public:
	ARREPRecvTable();
	virtual ~ARREPRecvTable();
	void addEntry(ARREPRecvEntry* entry);
	void addEntry(Flow flow, int seqNo, FlowRequest flowReq);
	void addEntry(Flow flow, int seqNo);
	void addARREP(ARREP arrep);
	void deleteEntry(Flow flow);
	ARREPRecvEntry* getEntry(Flow flow);
	void printTable();
};

#endif /* SCRATCH_BHSHIN_ROUTE_ARREPTABLE_H_ */
