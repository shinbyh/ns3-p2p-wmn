/*
 * flowcheck_recv_table.cc
 *
 *  Created on: Jan 16, 2018
 *      Author: bhshin
 */

#include "route_flowcheck_recv_table.h"
#include "my_config.h"

FlowCheckRecvTable::FlowCheckRecvTable() {

}

FlowCheckRecvTable::~FlowCheckRecvTable() {
	map<uint32_t, FlowCheck*>::iterator itr;
	for(itr = table.begin(); itr != table.end(); itr++){
		delete itr->second;
	}
}

void FlowCheckRecvTable::addFlowCheckReply(uint32_t nodeId, FlowCheck* flowCheck) {
	if(flowCheck->getMsgType() == ROUTE_FLOWCHECK_REPLY){
		this->table[nodeId] = flowCheck;
	}
}

uint32_t FlowCheckRecvTable::getOptimalDetourNode() {
	if(table.size() == 0){
		return NODEID_NOT_FOUND;
	}



	// temporary placeholder.
	return NODEID_NOT_FOUND;
}
