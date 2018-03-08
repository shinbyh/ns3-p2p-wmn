/*
 * route_flow_accept_req_table.h
 *
 *  Created on: Feb 20, 2018
 *      Author: bhshin
 */

#ifndef SCRATCH_P2P_BHSHIN_ROUTE_FLOW_ACCEPT_REQ_TABLE_H_
#define SCRATCH_P2P_BHSHIN_ROUTE_FLOW_ACCEPT_REQ_TABLE_H_

#include <map>
#include <vector>
#include "flow.h"
#include "qos_requirement.h"
#include "link_quality.h"
#include "route_flow_accept_request.h"

using namespace std;

class FlowAcceptRequestSentTable {
private:
	Flow flow;
	QoSRequirement qosReq;
	LinkQuality endToEndQuality;
	uint32_t nextHopToSrc;
	uint32_t prevNextHop;
	vector<uint32_t> srcRoute;
	map<uint32_t, FlowAcceptRequest*> table;

public:
	FlowAcceptRequestSentTable();
	virtual ~FlowAcceptRequestSentTable();

	void addFlowAcceptRequest(uint32_t nodeId, FlowAcceptRequest* flowCheck);
	uint32_t getOptimalDetourNode();

	const vector<uint32_t>& getSrcRoute() const {
		return srcRoute;
	}

	void setSrcRoute(const vector<uint32_t>& srcRoute) {
		this->srcRoute = srcRoute;
	}

	const LinkQuality& getEndToEndQuality() const {
		return endToEndQuality;
	}

	void setEndToEndQuality(const LinkQuality& endToEndQuality) {
		this->endToEndQuality = endToEndQuality;
	}

	const Flow& getFlow() const {
		return flow;
	}

	void setFlow(const Flow& flow) {
		this->flow = flow;
	}

	uint32_t getNextHopToSrc() const {
		return nextHopToSrc;
	}

	void setNextHopToSrc(uint32_t nextHopToSrc) {
		this->nextHopToSrc = nextHopToSrc;
	}

	uint32_t getPrevNextHop() const {
		return prevNextHop;
	}

	void setPrevNextHop(uint32_t prevNextHop) {
		this->prevNextHop = prevNextHop;
	}

	const QoSRequirement& getQosReq() const {
		return qosReq;
	}

	void setQosReq(const QoSRequirement& qosReq) {
		this->qosReq = qosReq;
	}
};

#endif /* SCRATCH_P2P_BHSHIN_ROUTE_FLOW_ACCEPT_REQ_TABLE_H_ */
