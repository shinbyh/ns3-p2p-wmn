/*
 * ctrl_flowtable.h
 *
 *  Created on: Feb 2, 2017
 *      Author: bhshin
 */

#ifndef SCRATCH_BHSHIN_CTRL_FLOWTABLE_H_
#define SCRATCH_BHSHIN_CTRL_FLOWTABLE_H_

#include "ctrl_flow.h"
#include "packet_info.h"
#include <map>
#include <vector>
#include <string>

class ControlFlowTable {
private:
	std::map<int, ControlFlow*> ctrlFlowTable;
	std::vector<int> keysTimeOrder;
public:
	ControlFlowTable();
	virtual ~ControlFlowTable();
	void add(bool isTx, PacketInfo* pktInfo);
	void updateRealTimeBandwidth();
	std::string getControlOutput(std::string time);
	std::string getControlFlowInfo();
	int getNumberOfCtrlPorts();
};

#endif /* SCRATCH_BHSHIN_CTRL_FLOWTABLE_H_ */
