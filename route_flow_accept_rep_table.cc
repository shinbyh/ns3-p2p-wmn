/*
 * route_flow_accept_req_table.cc
 *
 *  Created on: Feb 20, 2018
 *      Author: bhshin
 */

#include "my_config.h"
#include <boost/foreach.hpp>
#include <algorithm>
#include "ns3/core-module.h"
#include "route_flow_accept_rep_table.h"

FlowAcceptReplyRecvTable::FlowAcceptReplyRecvTable() {
	this->nextHopToSrc = NODEID_NOT_FOUND;
	this->prevNextHop = NODEID_NOT_FOUND;
}

FlowAcceptReplyRecvTable::~FlowAcceptReplyRecvTable() {
	map<uint32_t, FlowAcceptReply*>::iterator itr;
	for(itr = table.begin(); itr != table.end(); itr++){
		delete itr->second;
	}
}

void FlowAcceptReplyRecvTable::addFlowAcceptReply(uint32_t nodeId,
		FlowAcceptReply* flowAccRep) {
	this->table[nodeId] = flowAccRep;
}

uint32_t FlowAcceptReplyRecvTable::getOptimalDetourNode() {
	uint32_t optimalNode = NODEID_NOT_FOUND;
	LinkQuality baseLq(0.0, 100000.0, 100000.0, 0.99);

	std::pair<uint32_t, FlowAcceptReply*> p;
	BOOST_FOREACH(p, this->table){
		FlowAcceptReply* reply = p.second;
		LinkQuality lq = reply->getDetourLinkQuality();
		if(lq.compare(baseLq) > 0){
			baseLq = lq;
			optimalNode = p.first;
		}
	}

	return optimalNode;
}

/**
 * Returns an optimal FlowAcceptReply message.
 * The message includes a detour subpath (a sequence of detour nodes).
 */
FlowAcceptReply* FlowAcceptReplyRecvTable::getOptimalFlowAcceptReply() {
	//uint32_t optimalNode = NODEID_NOT_FOUND;
	FlowAcceptReply* optimalReply = NULL;
	LinkQuality baseLq(0.0, 100000.0, 100000.0, 0.99);

	std::pair<uint32_t, FlowAcceptReply*> p;
	BOOST_FOREACH(p, this->table){
		FlowAcceptReply* reply = p.second;
		LinkQuality lq = reply->getDetourLinkQuality();
		if(lq.compare(baseLq) > 0){
			baseLq = lq;
			//optimalNode = p.first;
			optimalReply = reply;
		}
	}

	return optimalReply;
}
