/*
 * route_arreqtable.h
 *
 *  Created on: Apr 19, 2017
 *      Author: bhshin
 */

#ifndef SCRATCH_BHSHIN_ROUTE_ARREQTABLE_H_
#define SCRATCH_BHSHIN_ROUTE_ARREQTABLE_H_

#include "flow.h"
#include <map>

/**
 * This is used for tracking the recent
 * sequence number for each flow.
 */
class ARREQSentEntry {
private:
	Flow flow;
	int seqNo;

public:
	ARREQSentEntry(Flow flow, int seqNo);
	virtual ~ARREQSentEntry();
	const Flow& getFlow() const;
	int getSeqNo() const;
	void setSeqNo(int seqNo);
	void incrementSeqNo();
};

class ARREQSentTable {
private:
	std::map<Flow, ARREQSentEntry*> table;
public:
	ARREQSentTable();
	virtual ~ARREQSentTable();
	void addEntry(Flow flow, int seqNo);
	ARREQSentEntry* getEntry(Flow flow);
	int getNextSeqNo(Flow flow);
};

#endif /* SCRATCH_BHSHIN_ROUTE_ARREQTABLE_H_ */
