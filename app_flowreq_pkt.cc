/*
 * app_flowreq_pkt.cc
 *
 *  Created on: Mar 25, 2017
 *      Author: bhshin
 */
#include "app_flowreq_pkt.h"

FlowRequest AppFlowReqPkt::getFlowReq() const {
	return flowReq;
}

void AppFlowReqPkt::setFlowReq(const FlowRequest& flowReq) {
	this->flowReq = flowReq;
}

ns3::Ptr<MyNS3Packet> AppFlowReqPkt::getMyPkt() const {
	return myPkt;
}

AppFlowReqPkt::AppFlowReqPkt(int initNodeId, FlowRequest flowReq, ns3::Ptr<MyNS3Packet> myPkt) {
	this->initNodeId = initNodeId;
	this->flowReq = flowReq;
	this->myPkt = myPkt;
}

AppFlowReqPkt::~AppFlowReqPkt() {
}

int AppFlowReqPkt::getInitNodeId() const {
	return initNodeId;
}

void AppFlowReqPkt::setInitNodeId(int initNodeId) {
	this->initNodeId = initNodeId;
}

void AppFlowReqPkt::setMyPkt(const ns3::Ptr<MyNS3Packet> myPkt) {
	this->myPkt = myPkt;
}
