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
	std::map<uint32_t, FlowStat*> flowStats; // statistics of all associated links (neighbor IDs)
	std::vector<uint32_t> neighbors;
	std::vector<uint32_t> srcRoute;
	QoSRequirement qosReq;
	QoSRequirement hopQosReq;
	int flowSeqNo;
	int unsatisfactoryCount;
	bool active;
	int routingFlag;
	bool routeSearching;
	bool controlFlow;
	uint32_t fwdNodeId; // forwarding node ID
	uint32_t prevNodeId; // previous node ID
	int packetSeqNo; // for source nodes only (app-layer performance measurement)
	bool randomnessActivated; // for source nodes only (Soft QoS; traffic randomization)
	double randomnessTime; // for source nodes only (Soft QoS; traffic randomization)

	void initAppReq();
	//void initFlowStats(std::vector<int> ifIdices);
	bool checkControlFlow();
	std::string srcRouteToString();

public:
	FlowEntry(Flow flow, uint32_t nodeId, long startTime);
	FlowEntry(uint32_t nodeId, PacketInfo pktInfo);
	~FlowEntry();
	bool isFlowStatExists(uint32_t nodeId);
	void addFlowStat(uint32_t nodeId);
	void deleteFlowStat(uint32_t nodeId);
	void addPacketInfo(uint32_t nodeId, PacketInfo pktInfo); // packetinfo for a neighbor
	bool isActive() const;
	void setActive(bool active);
	int getFlowSeqNo() const;
	void setFlowSeqNo(int flowSeqNo);
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
	int getNumberOfRealTimePackets(uint32_t nodeId);
	void setAllocatedBandwidth(uint32_t nodeId, double allocatedBW);
	double getAllocatedBandwidth(uint32_t nodeId);
	double getAvgRealTimeBandwidth(uint32_t nodeId);
	double getLastAccumulatedBandwidth(uint32_t nodeId);
	double getRealTimeBandwidth(uint32_t nodeId);
	const double getAvgRealTimeBandwidth() const;
	QoSRequirement getQosReq() const;
	void setQosReq(QoSRequirement qosReq);
	QoSRequirement getHopQosReq() const;
	void setHopQosReq(QoSRequirement& hopQosReq);
	const std::vector<uint32_t>& getSrcRoute() const;
	void setSrcRoute(const std::vector<uint32_t>& srcRoute);
	bool isRandomnessActivated() const;
	void setRandomnessActivated(bool randomnessActivated);
	double getRandomnessTime() const;
	void setRandomnessTime(double randomnessTime);
	std::string toFormattedFlowInfo();
	std::string toString();
	uint32_t getFwdNodeId() const;
	void setFwdNodeId(uint32_t fwdNodeId);
	uint32_t getPrevNodeId() const;
	void setPrevNodeId(uint32_t prevNodeId);
	bool operator <(const FlowEntry &a) const;
	int getPacketSeqNo() const;
	void setPacketSeqNo(int packetSeqNo);
	void incrementPacketSeNo();
};



#endif /* FLOW_FLOW_ENTRY_H_ */
