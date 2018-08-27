/*
 * route_flowcheck.h
 *
 *  Created on: Jan 16, 2018
 *      Author: bhshin
 */

#ifndef SCRATCH_P2P_BHSHIN_ROUTE_FLOWCHECK_H_
#define SCRATCH_P2P_BHSHIN_ROUTE_FLOWCHECK_H_

#include <vector>
#include <string>
#include "source_route_stat.h"
#include "flow.h"

using namespace std;

class FlowCheck {
private:
	int msgType;
	Flow flow;
	uint32_t nodeId; // nodeID of the initiator
	uint32_t prevNextHop; // previous (current) next hop
	int seqNo;
	int TTL; // for request only
	uint32_t replierNodeId; // for reply only
	double avgAvailableBw; // average available bandwidth of the neighbors of a replier node
	vector<SourceRouteStat> stats;

	const string serializeStats() const;
	void parseStats(string str);

public:
	FlowCheck(int msgType, int TTL);
	virtual ~FlowCheck();
	int getMsgType() const;
	void setMsgType(int msgType);
	int getSeqNo() const;
	void setSeqNo(int seqNo);
	const vector<SourceRouteStat>& getStats() const;
	void setStats(const vector<SourceRouteStat>& stats);
	void addStat(const SourceRouteStat stat);

	const string serialize() const;
	void parse(string str);
	int getTtl() const;
	void setTtl(int ttl);
	void decrementTTL();
	uint32_t getNodeId() const;
	void setNodeId(uint32_t nodeId);
	uint32_t getReplierNodeId() const;
	void setReplierNodeId(uint32_t replierNodeId);
	uint32_t getPrevNextHop() const;
	void setPrevNextHop(uint32_t prevNextHop);
	double getAvgAvailableBw() const;
	void setAvgAvailableBw(double avgAvailableBw);
	const Flow& getFlow() const;
	void setFlow(const Flow& flow);
};

#endif /* SCRATCH_P2P_BHSHIN_ROUTE_FLOWCHECK_H_ */
