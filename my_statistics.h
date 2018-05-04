/*
 * my_statistics.h
 *
 *  Created on: May 3, 2018
 *      Author: bhshin
 */

#ifndef SCRATCH_P2P_BHSHIN_MY_STATISTICS_H_
#define SCRATCH_P2P_BHSHIN_MY_STATISTICS_H_

#include "flow.h"
#include <map>
#include <vector>

using namespace std;

class FlowGoodput {
private:
	map<uint32_t, int> qosViolationCount; // count for each node
	map<uint32_t, int> localRepairCount; // count for each node
	int rtDiscoveryCount; // count by source only
	vector<double> goodputs; // measured by destination only
public:
	FlowGoodput();
	virtual ~FlowGoodput();

	void addGoodput(double goodput);
	double getAverageGoodput();
	const vector<double>& getGoodputs() const;
	void setGoodputs(const vector<double>& goodputs);
	int getTotalLocalRepairCount() const;
	void setLocalRepairCount(uint32_t nodeId, int localRepairCount);
	void incrementLocalRepairCount(uint32_t nodeId);
	int getTotalQosViolationCount() const;
	void setQosViolationCount(uint32_t nodeId, int qosViolationCount);
	void incrementQoSViolationCount(uint32_t nodeId);
	int getRtDiscoveryCount() const;
	void setRtDiscoveryCount(int rtDiscoveryCount);
	void incrementRtDiscoveryCount();
};

class MyStatistics {
private:
	map<Flow, int> qosViolationMap;
	// vector of all goodputs for each flow
	map<Flow, FlowGoodput*> flowGoodputMap;

public:
	static MyStatistics& instance(){
		static MyStatistics instance;
		return instance;
	}
	MyStatistics();
	virtual ~MyStatistics();

	void addGoodput(Flow flow, double goodput);
	void incrementQoSViolationCount(Flow flow, uint32_t nodeId);
	void incrementLocalRepairCount(Flow flow, uint32_t nodeId);
	void incrementRtDiscoveryCount(Flow flow);
	void writeFlowGoodputStatistics();
};

#endif /* SCRATCH_P2P_BHSHIN_MY_STATISTICS_H_ */
