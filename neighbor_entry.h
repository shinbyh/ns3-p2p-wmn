/*
 * neighbor_entry.h
 *
 *  Created on: Dec 12, 2016
 *      Author: bhshin
 */

#ifndef NETCTX_NEIGHBOR_ENTRY_H_
#define NETCTX_NEIGHBOR_ENTRY_H_

#include <boost/circular_buffer.hpp>
#include <string>
#include <vector>
#include <set>

#include "ns3/ipv4-address.h"
#include "ns3/applications-module.h"
#include "flow.h"

#define CIRCULAR_BUFFER_SIZE 10

class NeighborEntry {
private:
	uint32_t nodeId;
	ns3::Ipv4Address ipAddr;
	std::vector<ns3::Ipv4Address> reachableIPs;
	std::vector<uint32_t> reachableNodeIds;
	std::set<Flow> flowList;
	double occupiedBw;
	double avgOccupiedBw;
	double allocBw;
	long delay;
	double lossRate;
	boost::circular_buffer<double> etxSample;
	boost::circular_buffer<long> avgDelay;
	boost::circular_buffer<long> avgJitter;
	long lastUpdateTime;
	ns3::Time prevDstDelayMsrTime;
	ns3::Time prevSrcDelayMsrTime;
	bool isActive;
	bool isMeshRouter;
	int dmSeqNo; // DelayMeasurement seqNo.
	ns3::Time dmSendTime; // DelayMeasurement Requested time


	void initialize();
	const double calculateLossRate() const;

public:
	NeighborEntry();
	NeighborEntry(uint32_t nodeId, ns3::Ipv4Address ipAddr, bool isMeshRouter);
	void addFlow(Flow flow);
	void removeFlow(Flow flow);
	std::set<Flow> getFlowList();

	const int getNumOfFlows() const;
	const double getAllocBw() const;
	void setAllocBw(double allocBw);
	const long getDelay() const;
	void setDelay(long delay);
	const uint32_t getNodeId() const;
	void setNodeId(uint32_t nodeId);
	const ns3::Ipv4Address getIp() const;
	void setIp(const ns3::Ipv4Address ipAddr);
	const bool isIsActive() const;
	void setIsActive(bool isActive);
	const bool isIsMeshRouter() const;
	void setIsMeshRouter(bool isMeshRouter);
	const long getLastUpdateTime() const;
	void setLastUpdateTime(long lastUpdateTime);
	const double getLossRate() const;
	void setLossRate(double lossRate);
	const double getOccupiedBw() const;
	void setOccupiedBw(double occupiedBw);
	const double getAvgOccupiedBw() const; // Scheme 1 2017.11.16)
	void setAvgOccupiedBw(double avgOccupiedBw); // Scheme 1 (2017.11.16)
	const ns3::Time& getPrevDstDelayMsrTime() const;
	void setPrevDstDelayMsrTime(const ns3::Time& prevDstDelayMsrTime);
	const ns3::Time& getPrevSrcDelayMsrTime() const;
	void setPrevSrcDelayMsrTime(const ns3::Time& prevSrcDelayMsrTime);
	const ns3::Time& getDmSendTime() const;
	void setDmSendTime(const ns3::Time& dmSendTime);
	int getDmSeqNo() const;
	void setDmSeqNo(int dmSeqNo);
	void incrementDmSeqNo();

	const bool containsIP(ns3::Ipv4Address ipAddr) const;
	const bool containsNeighbor(uint32_t nodeId) const;
	void addSampleToETX(int sample);
	void addDelayToMovingAvg(long delay);
	void addJitterToMovingAvg(long jitter);
	const double getAverageDelay() const;
	const double getAverageJitter() const;

	const std::string toString() const;
	const std::vector<uint32_t>& getReachableNodeIds() const;
	void setReachableNodeIds(const std::vector<uint32_t>& reachableNodeIds);
};



#endif /* NETCTX_NEIGHBOR_ENTRY_H_ */
