#ifndef TRAFFIC_MONITOR_PACKET_INFO_H_
#define TRAFFIC_MONITOR_PACKET_INFO_H_

#include <iostream>

#include "ns3/ipv4-address.h"
#include "flow.h"
#include "flow_type.h"

class PacketInfo {
private:
	Flow flow;
	int ifIdx;
	long time;
	int bytes;

	bool tcpFlowStart;
	int tcpPackets;
	double tcpAvgFlowRate;

	void init(long time, const Flow flow, int ifIdx, int bytes);

public:
	PacketInfo(std::string time, const Flow flow, int ifIdx, int bytes);
	PacketInfo(long time, const Flow flow, int ifIdx, int bytes);
	~PacketInfo();

	int getBytes() const;
	void setBytes(int bytes);
	const int getIfIdx() const;
	void setIfIdx(const int ifIdx);
	long getTime() const;
	void setTime(long time);
	double getTcpAvgFlowRate() const;
	void setTcpAvgFlowRate(double tcpAvgFlowRate);
	bool isTcpFlowStart() const;
	void setTcpFlowStart(bool tcpFlowStart);
	int getTcpPackets() const;
	void setTcpPackets(int tcpPackets);
	const Flow getFlow() const;

	void parse(std::string line);
	bool operator == (const PacketInfo &pktInfo) const;
	int hashCode();
	std::string toString();

};

#endif /* TRAFFIC_MONITOR_PACKET_INFO_H_ */
