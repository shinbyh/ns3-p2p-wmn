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
#include "link_quality.h"
#include <vector>

using namespace std;

class FlowAcceptRequest {
private:
	Flow flow;
	int seqNo;
	uint32_t senderId; // initiator
	uint32_t nextHop;
	int TTL;
	QoSRequirement qosReq;
	LinkQuality linkQuality;
	vector<uint32_t> detourTrace;
	string serializeTrace();

public:
	FlowAcceptRequest(Flow flow, int seqNo, uint32_t senderId, uint32_t nextHop, int TTL);
	FlowAcceptRequest();
	virtual ~FlowAcceptRequest();
	const Flow& getFlow() const;
	void setFlow(const Flow& flow);
	const QoSRequirement& getQosReq() const;
	void setQosReq(const QoSRequirement& qosReq);
	uint32_t getSenderId() const;
	void setSenderId(uint32_t senderId);
	int getTtl() const;
	void setTtl(int ttl);
	void decrementTtl();
	uint32_t getNextHop() const;
	void setNextHop(uint32_t nextHop);
	int getSeqNo() const;
	void setSeqNo(int seqNo);
	LinkQuality* getLinkQuality();
	void setLinkQuality(const LinkQuality& linkQuality);
	void addDetourTrace(uint32_t nodeId);
	void setDetourTrace(const vector<uint32_t>& detourTrace);
	const vector<uint32_t>& getDetourTrace() const;
	const string serialize();
	static FlowAcceptRequest parse(string str);
	static vector<uint32_t> parseDetourIDs(std::string str);
};

#endif /* SCRATCH_P2P_BHSHIN_ROUTE_FLOW_ACCEPT_REQUEST_H_ */
