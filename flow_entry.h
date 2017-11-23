/*
 * flow_entry.h
 *
 *  Created on: Oct 27, 2016
 *      Author: bhshin
 */

#ifndef FLOW_FLOW_ENTRY_H_
#define FLOW_FLOW_ENTRY_H_

#include "flow.h"
#include "flow_stat.h"
#include "packet_info.h"
#include "qos_requirement.h"
#include <map>

#define FLOW_PORT_UNAVAILABLE -1
#define FLOW_NO_ROUTE 0
#define FLOW_ROUTE_SEARCHING 1
#define FLOW_HAS_ROUTE 2

class FlowEntry {

private:
	Flow flow;
	long startTime;
	long lastTime;
	std::map<int, FlowStat*> flowStats;
	std::vector<int> ifIdices;
	QoSRequirement qosReq;
	QoSRequirement hopQosReq;
	int appReqSeqNo;
	int unsatisfactoryCount;
	bool active;
	int routingFlag;
	bool routeSearching;
	bool controlFlow;
	void initAppReq();
	void initFlowStats(std::vector<int> ifIdices);
	bool checkControlFlow();
	void resetRealTimeBandwidth(int ifIdx);



public:
	FlowEntry(Flow flow, long startTime);
	FlowEntry(PacketInfo pktInfo);
	~FlowEntry();
	void addPacketInfo(PacketInfo pktInfo);
	bool isActive() const;
	void setActive(bool active);
	int getAppReqSeqNo() const;
	void setAppReqSeqNo(int appReqSeqNo);
	const Flow& getFlow() const;
	void setFlow(const Flow& flow);
	bool isControlFlow() const;
	void setControlFlow(bool controlFlow);
	bool isRouteSearching() const;
	void setRouteSearching(bool routeSearching);
	int getRoutingFlag() const;
	void setRoutingFlag(int routingFlag);
	int getUnsatisfactoryCount() const;
	void setUnsatisfactoryCount(int unsatisfactoryCount);
	long getLastTime() const;
	void setLastTime(long lastTime);
	long getStartTime() const;
	void setStartTime(long startTime);
	void resetRealTimeBandwidth();
	void setAllocatedBandwidth(int iface, double allocatedBW);
	double getAllocatedBandwidth(int iface);
	double getAvgRealTimeBandwidth(int iface);
	double getAvgRealTimeBandwidth();
	double getAvgResidualBandwidth();
	QoSRequirement getQosReq() const;
	void setQosReq(QoSRequirement qosReq);
	QoSRequirement getHopQosReq() const;
	void setHopQosReq(QoSRequirement& hopQosReq);
	std::string toFormattedFlowInfo();
	std::string toString();
};



#endif /* FLOW_FLOW_ENTRY_H_ */
