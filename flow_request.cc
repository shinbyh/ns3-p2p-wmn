/*
 * flow_request.cc
 *
 *  Created on: Feb 6, 2017
 *      Author: bhshin
 */

#include "flow_request.h"

using namespace ns3;

FlowRequest::FlowRequest() {
	this->pktSize = 0;
	this->sendingRate = 0;
}

FlowRequest::FlowRequest(Flow flow, QoSRequirement qosReq, int pktSize,
		int sendingRate, Time startTime, Time duration) {
	this->flow = flow;
	this->qosReq = qosReq;
	this->pktSize = pktSize;
	this->sendingRate = sendingRate;
	this->startTime = startTime;
	this->duration = duration;
}

FlowRequest::~FlowRequest() {
}

const Time& FlowRequest::getDuration() const {
	return duration;
}

void FlowRequest::setDuration(const Time& duration) {
	this->duration = duration;
}

const Flow& FlowRequest::getFlow() const {
	return flow;
}

void FlowRequest::setFlow(const Flow& flow) {
	this->flow = flow;
}

int FlowRequest::getPktSize() const {
	return pktSize;
}

void FlowRequest::setPktSize(int pktSize) {
	this->pktSize = pktSize;
}

int FlowRequest::getSendingRate() const {
	return sendingRate;
}

void FlowRequest::setSendingRate(int sendingRate) {
	this->sendingRate = sendingRate;
}

const QoSRequirement& FlowRequest::getQosReq() const {
	return qosReq;
}

void FlowRequest::setQosReq(const QoSRequirement& qosReq) {
	this->qosReq = qosReq;
}

const Time& FlowRequest::getStartTime() const {
	return startTime;
}

void FlowRequest::setStartTime(const Time& startTime) {
	this->startTime = startTime;
}
