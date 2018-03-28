/*
 * flowcheck_recv_table.h
 *
 *  Created on: Jan 16, 2018
 *      Author: bhshin
 */

#ifndef SCRATCH_P2P_BHSHIN_ROUTE_FLOWCHECK_RECV_TABLE_H_
#define SCRATCH_P2P_BHSHIN_ROUTE_FLOWCHECK_RECV_TABLE_H_

#include <map>
#include <vector>
#include "flow.h"
#include "qos_requirement.h"
#include "link_quality.h"
#include "route_flowcheck.h"

using namespace std;

class FlowCheckRecvTable {
private:
	Flow flow;
	QoSRequirement qosReq;
	LinkQuality endToEndQuality;
	uint32_t nextHopToSrc;
	uint32_t prevNextHop;
	vector<uint32_t> srcRoute;
	map<uint32_t, FlowCheck*> table;

	bool containsNodeId(uint32_t nodeId);

public:
	FlowCheckRecvTable();
	virtual ~FlowCheckRecvTable();

	void addFlowCheckReply(uint32_t nodeId, FlowCheck* flowCheck);
	uint32_t getOptimalDetourNode(int hopCount);
	const LinkQuality& getEndToEndQuality() const;
	void setEndToEndQuality(const LinkQuality& endToEndQuality);
	const Flow& getFlow() const;
	void setFlow(const Flow& flow);
	uint32_t getNextHopToSrc() const;
	void setNextHopToSrc(uint32_t nextHopToSrc);
	uint32_t getPrevNextHop() const;
	void setPrevNextHop(uint32_t prevNextHop);
	const QoSRequirement& getQosReq() const;
	void setQosReq(const QoSRequirement& qosReq);

	const vector<uint32_t>& getSrcRoute() const {
		return srcRoute;
	}

	void setSrcRoute(const vector<uint32_t>& srcRoute) {
		this->srcRoute = srcRoute;
	}
};

#endif /* SCRATCH_P2P_BHSHIN_ROUTE_FLOWCHECK_RECV_TABLE_H_ */
