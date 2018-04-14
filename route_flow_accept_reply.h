/*
 * route_flow_accept_reply.h
 *
 *  Created on: Feb 17, 2018
 *      Author: bhshin
 */

#ifndef SCRATCH_P2P_BHSHIN_ROUTE_FLOW_ACCEPT_REPLY_H_
#define SCRATCH_P2P_BHSHIN_ROUTE_FLOW_ACCEPT_REPLY_H_

#include "flow.h"
#include "link_quality.h"
#include <vector>

using namespace std;

class FlowAcceptReply {
private:
	Flow flow;
	int seqNo;
	uint32_t senderId;
	uint32_t nextHop;
	vector<uint32_t> detourIDs; // a detour subpath
	LinkQuality detourLinkQuality; // link quality of a detour subpath

	const string serializeTrace();

public:
	FlowAcceptReply();
	FlowAcceptReply(Flow flow, int seqNo, uint32_t senderId, uint32_t nextHop);
	virtual ~FlowAcceptReply();
	const vector<uint32_t>& getDetourIDs() const;
	void setDetourIDs(const vector<uint32_t>& detourIDs);
	const LinkQuality& getDetourLinkQuality() const;
	void setDetourLinkQuality(const LinkQuality& detourLinkQuality);
	const Flow& getFlow() const;
	void setFlow(const Flow& flow);
	uint32_t getNextHop() const;
	void setNextHop(uint32_t nextHop);
	uint32_t getSenderId() const;
	void setSenderId(uint32_t senderId);
	int getSeqNo() const;
	void setSeqNo(int seqNo);
	void addDetourID(uint32_t detourID);
	const string serialize();
	static vector<uint32_t> parseDetourIDs(std::string str);
	static FlowAcceptReply parse(string str);
};

#endif /* SCRATCH_P2P_BHSHIN_ROUTE_FLOW_ACCEPT_REPLY_H_ */
