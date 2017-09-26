/*
 * ctrl_flow.h
 *
 *  Created on: Feb 2, 2017
 *      Author: bhshin
 */

#ifndef SCRATCH_BHSHIN_CTRL_FLOW_H_
#define SCRATCH_BHSHIN_CTRL_FLOW_H_

#include "packet_info.h"

class ControlFlow {
private:
	int port;
	int totalRecvBytes;
	int totalRecvPackets;
	int totalTxBytes;
	int totalTxPackets;
	int realTimeBandwidth;

public:
	ControlFlow(int port);
	virtual ~ControlFlow();
	int getPort() const;
	void setPort(int port);
	int getRealTimeBandwidth() const;
	void setRealTimeBandwidth(int realTimeBandwidth);
	int getTotalRecvBytes() const;
	void setTotalRecvBytes(int totalRecvBytes);
	int getTotalRecvPackets() const;
	void setTotalRecvPackets(int totalRecvPackets);
	int getTotalTxBytes() const;
	void setTotalTxBytes(int totalTxBytes);
	int getTotalTxPackets() const;
	void setTotalTxPackets(int totalTxPackets);
	void addPacketInfo(bool isTx, PacketInfo* pktInfo);
	void resetRealTimeBandwidth();
};

#endif /* SCRATCH_BHSHIN_CTRL_FLOW_H_ */
