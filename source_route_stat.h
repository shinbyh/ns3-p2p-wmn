/*
 * source_route_trace.h
 *
 *  Created on: Jan 16, 2018
 *      Author: bhshin
 */

#ifndef SCRATCH_P2P_BHSHIN_SOURCE_ROUTE_STAT_H_
#define SCRATCH_P2P_BHSHIN_SOURCE_ROUTE_STAT_H_

#include <stdint.h>
#include <vector>
#include <string>

namespace std {

class SourceRouteStat {
private:
	vector<uint32_t> trace;
	double bandwidth; // occupied bandwidth by the flow

	void parseTrace(string str);

public:
	SourceRouteStat();
	virtual ~SourceRouteStat();

	double getBandwidth() const {
		return bandwidth;
	}

	void setBandwidth(double bandwidth) {
		this->bandwidth = bandwidth;
	}

	const vector<uint32_t>& getTrace() const {
		return trace;
	}

	void setTrace(const vector<uint32_t>& trace) {
		this->trace = trace;
	}

	void addTrace(uint32_t nodeId);
	int getNodeIdPosition(uint32_t nodeId);
	bool containsNodeId(uint32_t nodeId);
	const string serialize();
	void parse(string str);
};

} /* namespace std */

#endif /* SCRATCH_P2P_BHSHIN_SOURCE_ROUTE_STAT_H_ */
