/*
 * app_flowreq_pkt.h
 *
 *  Created on: Mar 25, 2017
 *      Author: bhshin
 */

#ifndef SCRATCH_BHSHIN_APP_FLOWREQ_PKT_H_
#define SCRATCH_BHSHIN_APP_FLOWREQ_PKT_H_

#include "ns3/core-module.h"
#include "flow_request.h"
#include "my_ns3_packet.h"

class AppFlowReqPkt {
private:
	int initNodeId;
	FlowRequest flowReq;
	ns3::Ptr<MyNS3Packet> myPkt;

public:
	AppFlowReqPkt(int initNodeId, FlowRequest flowReq, ns3::Ptr<MyNS3Packet> myPkt);
	~AppFlowReqPkt();
	FlowRequest getFlowReq() const;
	void setFlowReq(const FlowRequest& flowReq);
	ns3::Ptr<MyNS3Packet> getMyPkt() const;
	void setMyPkt(const ns3::Ptr<MyNS3Packet> myPkt);
	int getInitNodeId() const;
	void setInitNodeId(int initNodeId);
};

#endif /* SCRATCH_BHSHIN_APP_FLOWREQ_PKT_H_ */
