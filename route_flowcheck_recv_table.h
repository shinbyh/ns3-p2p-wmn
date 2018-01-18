/*
 * flowcheck_recv_table.h
 *
 *  Created on: Jan 16, 2018
 *      Author: bhshin
 */

#ifndef SCRATCH_P2P_BHSHIN_ROUTE_FLOWCHECK_RECV_TABLE_H_
#define SCRATCH_P2P_BHSHIN_ROUTE_FLOWCHECK_RECV_TABLE_H_

#include <map>
#include "route_flowcheck.h"

using namespace std;

class FlowCheckRecvTable {
private:
	map<uint32_t, FlowCheck*> table;

public:
	FlowCheckRecvTable();
	virtual ~FlowCheckRecvTable();

	void addFlowCheckReply(uint32_t nodeId, FlowCheck* flowCheck);
	uint32_t getOptimalDetourNode();
};

#endif /* SCRATCH_P2P_BHSHIN_ROUTE_FLOWCHECK_RECV_TABLE_H_ */
