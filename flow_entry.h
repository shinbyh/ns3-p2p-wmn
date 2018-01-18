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
	std::map<int, FlowStat*> flowStats; // statistics of all associated links (neighbor IDs)
	std::vector<int> neighbors;
	std::vector<uint32_t> srcRoute;
	QoSRequirement qosReq;
	QoSRequirement hopQosReq;
	int appReqSeqNo;
	int unsatisfactoryCount;
	bool active;
	int routingFlag;
	bool routeSearching;
	bool controlFlow;
	int fwdNodeId; // forwarding node ID

	void initAppReq();
	//void initFlowStats(std::vector<int> ifIdices);
	bool checkControlFlow();
	std::string srcRouteToString();

public:
	FlowEntry(Flow flow, int nodeId, long startTime);
	FlowEntry(int nodeId, PacketInfo pktInfo);
	~FlowEntry();
	bool isFlowStatExists(int nodeId);
	void addFlowStat(int nodeId);
	void addPacketInfo(int nodeId, PacketInfo pktInfo); // packetinfo for a neighbor
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
	void setAllocatedBandwidth(int nodeId, double allocatedBW);
	double getAllocatedBandwidth(int nodeId);
	double getAvgRealTimeBandwidth(int nodeId);
	const double getAvgRealTimeBandwidth() const;
	QoSRequirement getQosReq() const;
	void setQosReq(QoSRequirement qosReq);
	QoSRequirement getHopQosReq() const;
	void setHopQosReq(QoSRequirement& hopQosReq);
	const std::vector<uint32_t>& getSrcRoute() const;
	void setSrcRoute(const std::vector<uint32_t>& srcRoute);
	std::string toFormattedFlowInfo();
	std::string toString();

	int getFwdNodeId() const {
		return fwdNodeId;
	}
	void setFwdNodeId(int fwdNodeId) {
		this->fwdNodeId = fwdNodeId;
	}

	bool operator <(const FlowEntry &a) const;
};



#endif /* FLOW_FLOW_ENTRY_H_ */
