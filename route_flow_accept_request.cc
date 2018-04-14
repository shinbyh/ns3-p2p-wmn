/*
 * route_flow_accept_request.cc
 *
 *  Created on: Feb 17, 2018
 *      Author: bhshin
 */

#include "route_flow_accept_request.h"
#include <sstream>
#include "string_tokenizer.h"
#include "my_config.h"


FlowAcceptRequest::FlowAcceptRequest(Flow flow, int seqNo, uint32_t senderId, uint32_t nextHop, int TTL) {
	this->flow = flow;
	this->seqNo = seqNo;
	this->senderId = senderId;
	this->nextHop = nextHop;
	this->TTL = TTL;
}

FlowAcceptRequest::FlowAcceptRequest(){
	this->seqNo = 0;
	this->senderId = 9999999;
	this->nextHop = 9999999;
	this->TTL = 0;
}

FlowAcceptRequest::~FlowAcceptRequest() {
}

const Flow& FlowAcceptRequest::getFlow() const {
	return flow;
}

void FlowAcceptRequest::setFlow(const Flow& flow) {
	this->flow = flow;
}

const QoSRequirement& FlowAcceptRequest::getQosReq() const {
	return qosReq;
}

void FlowAcceptRequest::setQosReq(const QoSRequirement& qosReq) {
	this->qosReq = qosReq;
}

uint32_t FlowAcceptRequest::getSenderId() const {
	return senderId;
}

void FlowAcceptRequest::setSenderId(uint32_t senderId) {
	this->senderId = senderId;
}

int FlowAcceptRequest::getTtl() const {
	return TTL;
}

void FlowAcceptRequest::setTtl(int ttl) {
	TTL = ttl;
}

string FlowAcceptRequest::serializeTrace() {
	if(this->detourTrace.size() == 0){
		return "--";
	} else {
		stringstream ss;
		for(size_t i=0; i<this->detourTrace.size(); i++){
			if(i + 1 < this->detourTrace.size())
				ss << this->detourTrace[i] << ",";
			else
				ss << this->detourTrace[i];
		}
		return ss.str();
	}
}

const string FlowAcceptRequest::serialize() {
	stringstream ss;
	ss << std::fixed;
	ss << ROUTE_FLOW_ACCEPT_REQUEST << "@" <<
			this->flow.getSrc() << "@" <<
			this->flow.getSrcPort() << "@" <<
			this->flow.getDst() << "@" <<
			this->flow.getDstPort() << "@" <<
			this->flow.getTypeStr() << "@" <<
			this->seqNo << "@" <<
			this->senderId << "@" <<
			this->nextHop << "@" <<
			this->TTL << "@" <<
			this->qosReq.serialize() << "@" <<
			this->linkQuality.serialize() << "@" <<
			serializeTrace();
	return ss.str();
}

uint32_t FlowAcceptRequest::getNextHop() const {
	return nextHop;
}

int FlowAcceptRequest::getSeqNo() const {
	return seqNo;
}

void FlowAcceptRequest::setSeqNo(int seqNo) {
	this->seqNo = seqNo;
}

void FlowAcceptRequest::setNextHop(uint32_t nextHop) {
	this->nextHop = nextHop;
}

FlowAcceptRequest FlowAcceptRequest::parse(string str) {
	vector<string> tokens;
	tokenizeString(str, tokens, "@");

	//atoi(tokens[0].c_str()); // message type, not used here.
	uint32_t src = atoi(tokens[1].c_str());
	int srcPort = atoi(tokens[2].c_str());
	uint32_t dst = atoi(tokens[3].c_str());
	int dstPort = atoi(tokens[4].c_str());
	FlowType::Type type = checkType(tokens[5]);
	Flow flow(src, srcPort, dst, dstPort, type);
	//this->setFlow(flow);

	int seqNo = atoi(tokens[6].c_str());
	uint32_t senderId = atoi(tokens[7].c_str());
	uint32_t nextHop = atoi(tokens[8].c_str());
	int TTL = atoi(tokens[9].c_str());
	QoSRequirement qosReq = QoSRequirement::parse(tokens[10]);
	LinkQuality lq = LinkQuality::parse(tokens[11]);
	vector<uint32_t> detourTrace = FlowAcceptRequest::parseDetourIDs(tokens[12]);

	FlowAcceptRequest request(flow, seqNo, senderId, nextHop, TTL);
	request.setQosReq(qosReq);
	request.setLinkQuality(lq);
	request.setDetourTrace(detourTrace);
	return request;
}

vector<uint32_t> FlowAcceptRequest::parseDetourIDs(std::string str){
	vector<uint32_t> ids;
	if(str == "--") return ids;

	std::vector<std::string> tokens;
	tokenizeString(str, tokens, ",");
	for(std::string addr : tokens){
		ids.push_back(atoi(addr.c_str()));
	}
	return ids;
}

void FlowAcceptRequest::decrementTtl() {
	if(this->TTL > 0) this->TTL--;
}

LinkQuality* FlowAcceptRequest::getLinkQuality() {
	return &linkQuality;
}

void FlowAcceptRequest::setLinkQuality(const LinkQuality& linkQuality) {
	this->linkQuality = linkQuality;
}

const vector<uint32_t>& FlowAcceptRequest::getDetourTrace() const {
	return detourTrace;
}

void FlowAcceptRequest::addDetourTrace(uint32_t nodeId) {
	this->detourTrace.push_back(nodeId);
}

void FlowAcceptRequest::setDetourTrace(const vector<uint32_t>& detourTrace) {
	this->detourTrace = detourTrace;
}
