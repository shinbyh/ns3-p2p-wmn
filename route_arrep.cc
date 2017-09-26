/*
 * route_arrep.cc
 *
 *  Created on: Jan 18, 2017
 *      Author: bhshin
 */

#include "route_arrep.h"
#include "my_config.h"
#include "string_tokenizer.h"
#include <sstream>

using namespace ns3;

ARREP::ARREP() {
	this->seqNo = 0;
}

ARREP::ARREP(Flow flow, int seqNo, QoSRequirement qosReq,
		LinkQuality linkQuality, std::vector<uint32_t> trace) {
	this->flow = flow;
	this->seqNo = seqNo;
	this->qosReq = qosReq;
	this->linkQuality = linkQuality;
	this->trace = trace;
}

ARREP::~ARREP() {
}


std::string ARREP::serializeTrace() {
	stringstream ss;
	for(size_t i=0; i<this->trace.size(); i++){
		if(i + 1 < this->trace.size())
			ss << this->trace[i] << ",";
		else
			ss << this->trace[i];
	}
	return ss.str();
}

void ARREP::parseTrace(std::string str) {
	std::vector<std::string> tokens;
	tokenizeString(str, tokens, ",");
	for(std::string addr : tokens){
		addTrace(atoi(addr.c_str()));
	}
}

Flow ARREP::getFlow() {
	return this->flow;
}

ARREP ARREP::deriveFrom(ARREQ arreq) {
	LinkQuality* lq = arreq.getLinkQuality();
	ARREP arrep(arreq.getFlow(), arreq.getSeqNo(), arreq.getQosReq(), *lq, arreq.getTrace());
	return arrep;
}

void ARREP::parse(std::string str) {
	vector<std::string> tokens;
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
	LinkQuality linkQuality = LinkQuality::parse(tokens[8]);
	this->setLinkQuality(linkQuality);

	if(tokens.size() == 10) parseTrace(tokens[9]);
}

std::string ARREP::serialize() {
	stringstream ss;
	ss << std::fixed;
	ss << ROUTE_ARREP << "@" <<
			this->flow.getSrc() << "@" <<
			this->flow.getSrcPort() << "@" <<
			this->flow.getDst() << "@" <<
			this->flow.getDstPort() << "@" <<
			this->flow.getTypeStr() << "@" <<
			this->seqNo << "@" <<
			this->qosReq.serialize() << "@" <<
			this->linkQuality.serialize() << "@" <<
			serializeTrace();
	return ss.str();
}

void ARREP::setFlow(const Flow& flow) {
	this->flow = flow;
}

LinkQuality ARREP::getLinkQuality() {
	return linkQuality;
}

void ARREP::setLinkQuality(const LinkQuality linkQuality) {
	this->linkQuality = linkQuality;
}

const QoSRequirement ARREP::getQosReq() const {
	return qosReq;
}

void ARREP::setQosReq(const QoSRequirement qosReq) {
	this->qosReq = qosReq;
}

int ARREP::getSeqNo() const {
	return seqNo;
}

void ARREP::setSeqNo(int seqNo) {
	this->seqNo = seqNo;
}

const std::vector<uint32_t> ARREP::getTrace() const {
	return trace;
}

void ARREP::setTrace(const std::vector<uint32_t> trace) {
	this->trace = trace;
}

void ARREP::addTrace(uint32_t nodeId) {
	this->trace.push_back(nodeId);
}

void ARREP::removeLastTrace(){
	this->trace.erase(this->trace.begin() + this->trace.size() - 1);
}

/**
 * Gets the position (starting from 0) in the
 * trace for the Ipv4Address.
 */
int ARREP::getTraceIndex(uint32_t addr){
	for(size_t i=0; i<this->trace.size(); i++){
		if(addr == this->trace[i]){
			return i;
		}
	}
	return -1;
}
