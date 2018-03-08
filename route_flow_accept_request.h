/*
 * route_flow_accept_request.h
 *
 *  Created on: Feb 17, 2018
 *      Author: bhshin
 */

#ifndef SCRATCH_P2P_BHSHIN_ROUTE_FLOW_ACCEPT_REQUEST_H_
#define SCRATCH_P2P_BHSHIN_ROUTE_FLOW_ACCEPT_REQUEST_H_

#include "flow.h"
#include "qos_requirement.h"

using namespace std;

class FlowAcceptRequest {
private:
	Flow flow;
	int seqNo;
	uint32_t senderId;
	uint32_t nextHop;
	int TTL;
	QoSRequirement hopQosReq;

public:
	FlowAcceptRequest(Flow flow, int seqNo, uint32_t senderId, uint32_t nextHop, int TTL);
	FlowAcceptRequest();
	virtual ~FlowAcceptRequest();
	const Flow& getFlow() const;
	void setFlow(const Flow& flow);
	const QoSRequirement& getHopQosReq() const;
	void setHopQosReq(const QoSRequirement& hopQosReq);
	uint32_t getSenderId() const;
	void setSenderId(uint32_t senderId);
	int getTtl() const;
	void setTtl(int ttl);
	void decrementTtl();
	uint32_t getNextHop() const;
	void setNextHop(uint32_t nextHop);
	int getSeqNo() const;
	void setSeqNo(int seqNo);
	const string serialize();
	static FlowAcceptRequest parse(string str);
};

#endif /* SCRATCH_P2P_BHSHIN_ROUTE_FLOW_ACCEPT_REQUEST_H_ */
