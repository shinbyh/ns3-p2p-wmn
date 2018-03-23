/*
 * flow_stat.h
 *
 *  Created on: Oct 27, 2016
 *      Author: bhshin
 */

#ifndef FLOW_FLOW_STAT_H_
#define FLOW_FLOW_STAT_H_

#include "packet_info.h"
#include <boost/circular_buffer.hpp>

class FlowStat {

private:
	int totalPackets;
	int totalBytes;
	int realTimePackets;
	int realTimeBandwidth;
	double allocatedBandwidth; // allocated bandwidth during route discovery
	int numOfFlows; // only used by HelloClient
	boost::circular_buffer<double> cbd;

	void init();

public:
	FlowStat();
	~FlowStat();
	double getAllocatedBandwidth() const;
	void setAllocatedBandwidth(double allocatedBandwidth);
	int getNumOfFlows() const;
	void setNumOfFlows(int numOfFlows);
	int getRealTimeBandwidth() const;
	void setRealTimeBandwidth(int realTimeBandwidth);
	int getRealTimePackets() const;
	void setRealTimePackets(int realTimePackets);
	int getTotalBytes() const;
	void setTotalBytes(int totalBytes);
	int getTotalPackets() const;
	void setTotalPackets(int totalPackets);
	//bool isUsed();
	//void setUsed(bool used);
	void incrementNumOfFlows(int level);
	void decrementNumOfFlows(int level);
	void accumulatePacket(PacketInfo pktInfo);
	void addToMovingAvg(double realTimeBandwidth);
	double getAvgRealTimeBandwidth();
	std::string toString();
};

#endif /* FLOW_FLOW_STAT_H_ */
