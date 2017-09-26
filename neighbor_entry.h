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

#include "ns3/ipv4-address.h"
#include "ns3/applications-module.h"

#define CIRCULAR_BUFFER_SIZE 10

class NeighborEntry {
private:
	uint32_t nodeId;
	ns3::Ipv4Address ipAddr;
	std::vector<ns3::Ipv4Address> reachableIPs;
	int numOfFlows;
	double occupiedBw;
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
	double calculateLossRate();

public:
	NeighborEntry();
	NeighborEntry(uint32_t nodeId, ns3::Ipv4Address ipAddr, bool isMeshRouter);
	double getAllocBw() const;
	void setAllocBw(double allocBw);
	long getDelay() const;
	void setDelay(long delay);
	const int getNodeId() const;
	void setNodeId(int nodeId);
	const ns3::Ipv4Address getIp() const;
	void setIp(const ns3::Ipv4Address ipAddr);
	bool isIsActive() const;
	void setIsActive(bool isActive);
	bool isIsMeshRouter() const;
	void setIsMeshRouter(bool isMeshRouter);
	long getLastUpdateTime() const;
	void setLastUpdateTime(long lastUpdateTime);
	double getLossRate() const;
	void setLossRate(double lossRate);
	int getNumOfFlows() const;
	void setNumOfFlows(int numOfFlows);
	double getOccupiedBw() const;
	void setOccupiedBw(double occupiedBw);
	const ns3::Time& getPrevDstDelayMsrTime() const;
	void setPrevDstDelayMsrTime(const ns3::Time& prevDstDelayMsrTime);
	const ns3::Time& getPrevSrcDelayMsrTime() const;
	void setPrevSrcDelayMsrTime(const ns3::Time& prevSrcDelayMsrTime);
	const ns3::Time& getDmSendTime() const;
	void setDmSendTime(const ns3::Time& dmSendTime);
	int getDmSeqNo() const;
	void setDmSeqNo(int dmSeqNo);
	void incrementDmSeqNo();

	bool containsIP(ns3::Ipv4Address ipAddr);
	void addSampleToETX(int sample);
	void addDelayToMovingAvg(long delay);
	void addJitterToMovingAvg(long jitter);
	double getAverageDelay();
	double getAverageJitter();

	const std::string toString();
};



#endif /* NETCTX_NEIGHBOR_ENTRY_H_ */
