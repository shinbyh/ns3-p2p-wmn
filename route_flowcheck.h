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

#define FLOW_CHECK_REQUEST 0
#define FLOW_CHECK_REPLY 1

using namespace std;

class FlowCheck {
private:
	int msgType;
	int seqNo;
	vector<SourceRouteStat> stats;

	const string serializeStats() const;
	void parseStats(string str);

public:
	FlowCheck(int msgType);
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
};

#endif /* SCRATCH_P2P_BHSHIN_ROUTE_FLOWCHECK_H_ */
