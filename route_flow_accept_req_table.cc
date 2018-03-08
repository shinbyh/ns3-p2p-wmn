/*
 * route_flow_accept_req_table.cc
 *
 *  Created on: Feb 20, 2018
 *      Author: bhshin
 */

#include "route_flow_accept_req_table.h"
#include "my_config.h"
#include <boost/foreach.hpp>
#include <algorithm>
#include "ns3/core-module.h"

FlowAcceptRequestSentTable::FlowAcceptRequestSentTable() {
	this->nextHopToSrc = NODEID_NOT_FOUND;
	this->prevNextHop = NODEID_NOT_FOUND;
}

FlowAcceptRequestSentTable::~FlowAcceptRequestSentTable() {
	map<uint32_t, FlowAcceptRequest*>::iterator itr;
	for(itr = table.begin(); itr != table.end(); itr++){
		delete itr->second;
	}
}

void FlowAcceptRequestSentTable::addFlowAcceptRequest(uint32_t nodeId,
		FlowAcceptRequest* flowAccReq) {
	this->table[nodeId] = flowAccReq;
}

uint32_t FlowAcceptRequestSentTable::getOptimalDetourNode() {
	// TODO: implement
	return 0;
}
