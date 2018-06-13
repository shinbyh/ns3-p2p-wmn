/*
 * route_rtsetup.cc
 *
 *  Created on: Feb 2, 2017
 *      Author: bhshin
 */

#include "route_rtsetup.h"
#include "my_config.h"
#include "my_node.h"
#include "string_tokenizer.h"
#include <sstream>

using namespace ns3;

RouteSetup::RouteSetup() {
	this->seqNo = 0;
}

RouteSetup::RouteSetup(Flow flow, int seqNo, QoSRequirement qosReq,
		std::vector<uint32_t> trace) {
	this->flow = flow;
	this->seqNo = seqNo;
	this->qosReq = qosReq;
	this->trace = trace;
}

RouteSetup::~RouteSetup() {
}

const Flow& RouteSetup::getFlow() const {
	return flow;
}

void RouteSetup::setFlow(const Flow& flow) {
	this->flow = flow;
}

const QoSRequirement& RouteSetup::getQosReq() const {
	return qosReq;
}

void RouteSetup::setQosReq(const QoSRequirement& qosReq) {
	this->qosReq = qosReq;
}

int RouteSetup::getSeqNo() const {
	return seqNo;
}

void RouteSetup::setSeqNo(int seqNo) {
	this->seqNo = seqNo;
}

const std::vector<uint32_t>& RouteSetup::getTrace() const {
	return trace;
}

void RouteSetup::setTrace(const std::vector<uint32_t>& trace) {
	this->trace = trace;
}

std::string RouteSetup::serializeTrace(){
	stringstream ss;
	for(size_t i=0; i<this->trace.size(); i++){
		if(i + 1 < this->trace.size())
			ss << this->trace[i] << ",";
		else
			ss << this->trace[i];
	}
	return ss.str();
}

void RouteSetup::parseTrace(std::string str){
	std::vector<std::string> tokens;
	tokenizeString(str, tokens, ",");
	for(std::string addr : tokens){
		addTrace(atoi(addr.c_str()));
	}
}

void RouteSetup::addTrace(uint32_t addr) {
	this->trace.push_back(addr);
}

int RouteSetup::getPositionInTrace(uint32_t addr) {
	for(size_t i=0; i<this->trace.size(); i++){
		if(addr == this->trace[i]){
			return i;
		}
	}
	return -1;
}

int RouteSetup::getPositionInTrace(Ptr<Node> node) {
	for(size_t i=0; i<this->trace.size(); i++){
		if(node->GetId() == this->trace[i]){
			return i;
		}
	}
	return -1;
}


uint32_t RouteSetup::getNextHop(uint32_t ip) {
	for(size_t i=0; i<this->trace.size(); i++){
		if(this->trace[i] == ip){
			if(i < this->trace.size())
				return this->trace[i+1];
			else return NODEID_NOT_FOUND;
		}
	}
	return NODEID_NOT_FOUND;
}

uint32_t RouteSetup::getNextHopOfCurrentNode(Ptr<Node> node) {
	for(size_t i=0; i<this->trace.size(); i++){
		if(node->GetId() == this->trace[i]){
			if(i < this->trace.size())
				return this->trace[i+1];
			else return NODEID_NOT_FOUND;
		}
	}
	return NODEID_NOT_FOUND;
}

uint32_t RouteSetup::getPrevHopOfCurrentNode(Ptr<Node> node) {
	for(size_t i=0; i<this->trace.size(); i++){
		if(node->GetId() == this->trace[i]){
			if(i < this->trace.size() && i > 0)
				return this->trace[i-1];
			else return NODEID_NOT_FOUND;
		}
	}
	return NODEID_NOT_FOUND;
}

std::string RouteSetup::serialize() {
	stringstream ss;
	ss << std::fixed;
	ss << ROUTE_SETUP << "@" <<
			this->flow.getSrc() << "@" <<
			this->flow.getSrcPort() << "@" <<
			this->flow.getDst() << "@" <<
			this->flow.getDstPort() << "@" <<
			this->flow.getTypeStr() << "@" <<
			this->seqNo << "@" <<
			this->qosReq.serialize() << "@" <<
			this->endToEndQuality.serialize() << "@" <<
			serializeTrace();
	return ss.str();
}

void RouteSetup::parse(std::string str) {
	std::vector<std::string> tokens;
	tokenizeString(str, tokens, "@");
	// tokens[0] is an message type indicator. skip it.

	uint32_t src = atoi(tokens[1].c_str());
	int srcPort = atoi(tokens[2].c_str());
	uint32_t dst = atoi(tokens[3].c_str());
	int dstPort = atoi(tokens[4].c_str());
	FlowType::Type type = checkType(tokens[5]);
	Flow flow(src, srcPort, dst, dstPort, type);
	this->setFlow(flow);

	this->setSeqNo(atoi(tokens[6].c_str()));
	QoSRequirement qosReq = QoSRequirement::parse(tokens[7]);
	this->setQosReq(qosReq);
	LinkQuality e2eQuality = LinkQuality::parse(tokens[8]);
	this->setEndToEndQuality(e2eQuality);

	if(tokens.size() == 10) parseTrace(tokens[9]);
}

void RouteSetup::parseFromARREP(ARREP arrep) {
	this->setFlow(arrep.getFlow());
	this->setSeqNo(arrep.getSeqNo());
	this->setQosReq(arrep.getQosReq());
	this->setEndToEndQuality(arrep.getLinkQuality());
	this->setTrace(arrep.getTrace());
}

LinkQuality* RouteSetup::getEndToEndQuality() {
	return &endToEndQuality;
}

void RouteSetup::setEndToEndQuality(const LinkQuality& endToEndQuality) {
	this->endToEndQuality = endToEndQuality;
}
