/*
 * my_application.h
 *
 *  Created on: Apr 24, 2018
 *      Author: bhshin
 */

#ifndef SCRATCH_P2P_BHSHIN_MY_APPLICATION_H_
#define SCRATCH_P2P_BHSHIN_MY_APPLICATION_H_

#include "ns3/core-module.h"
#include "my_ns3_packet.h"
#include "flow_request.h"
#include <fstream>

using namespace ns3;

class MyApplication {
	std::string name;
	int totalNumberOfPkts; // total number of received packets
	int totalRecvBytes;
	int tempNumberOfPkts;
	int tempRecvBytes;
	int dataRateBytes; // data consumption rate in "bytes-per-second"
	int videoBuffer; // current video buffer in bytes
	int minBufSizeToPlay; // minimum buffer size to play (bytes)
	bool buffering; // buffering state. true == buffering.
	bool pktConsumptionStarted; // pkt consumption state. true == consumption started.
	FlowRequest flowReq;
	Time lastUpdateTime;
	ofstream appDataOut; // app-layer data statistics

public:
	MyApplication(std::string name, FlowRequest flowReq);
	virtual ~MyApplication();

	void handleApplicationPacket(Ptr<MyNS3Packet> myPkt);
	void updateStatistics();
	void _consumeDataPackets();
	int getDataRate() const;
	void setDataRate(int dataRateBytes);
	const FlowRequest& getFlowReq() const;
	void setFlowReq(const FlowRequest& flowReq);
	const std::string& getName() const;
	void setName(const std::string& name);
	const Time& getLastUpdateTime() const;
	void setLastUpdateTime(const Time& lastUpdateTime);
	int getTotalNumberOfPkts() const;
	void setTotalNumberOfPkts(int totalNumberOfPkts);
	int getTotalRecvBytes() const;
	void setTotalRecvBytes(int totalRecvBytes);
	int getDataRateBytes() const;
	void setDataRateBytes(int dataRateBytes);
	int getMinBufSizeToPlay() const;
	void setMinBufSizeToPlay(int minBufSizeToPlay);
	int getVideoBuffer() const;
	void setVideoBuffer(int videoBuffer);
	bool isBuffering() const;
	void setBuffering(bool buffering);

	static void consumeDataPackets(MyApplication* myApp);
};

#endif /* SCRATCH_P2P_BHSHIN_MY_APPLICATION_H_ */
