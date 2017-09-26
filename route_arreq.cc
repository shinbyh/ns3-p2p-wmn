/*
 * route_arreq.cc
 *
 *  Created on: Jan 17, 2017
 *      Author: bhshin
 */

#include "route_arreq.h"
#include <sstream>
#include "string_tokenizer.h"
#include "my_config.h"

using namespace ns3;

ARREQ::ARREQ() {
	this->seqNo = 0;
	this->ttl = 10; // default maximum hops
}

ARREQ::ARREQ(Flow flow, int seqNo, int ttl, QoSRequirement qosReq) {
	this->flow = flow;
	this->seqNo = seqNo;
	this->ttl = ttl;
	this->qosReq = qosReq;
}

ARREQ::~ARREQ() {
}

const Flow ARREQ::getFlow() const {
	return flow;
}

void ARREQ::setFlow(const Flow flow) {
	this->flow = flow;
}

QoSRequirement ARREQ::getQosReq() {
	return qosReq;
}

void ARREQ::setQosReq(const QoSRequirement qosReq) {
	this->qosReq = qosReq;
}

int ARREQ::getSeqNo() const {
	return seqNo;
}

void ARREQ::setSeqNo(int seqNo) {
	this->seqNo = seqNo;
}

const std::vector<uint32_t> ARREQ::getTrace() const {
	return trace;
}

void ARREQ::setTrace(const std::vector<uint32_t> trace) {
	for(uint32_t nodeId : trace){
		this->trace.push_back(nodeId);
	}
}

std::string ARREQ::serializeTrace(){
	stringstream ss;
	for(size_t i=0; i<this->trace.size(); i++){
		if(i + 1 < this->trace.size())
			ss << this->trace[i] << ",";
		else
			ss << this->trace[i];
	}
	return ss.str();
}

void ARREQ::parseTrace(std::string str){
	std::vector<std::string> tokens;
	tokenizeString(str, tokens, ",");
	for(std::string addr : tokens){
		addTrace(atoi(addr.c_str()));
	}
}

std::string ARREQ::serialize() {
	stringstream ss;
	ss << std::fixed;
	ss << ROUTE_ARREQ << "@" <<
			this->flow.getSrc() << "@" <<
			this->flow.getSrcPort() << "@" <<
			this->flow.getDst() << "@" <<
			this->flow.getDstPort() << "@" <<
			this->flow.getTypeStr() << "@" <<
			this->seqNo << "@" <<
			this->ttl << "@" <<
			this->qosReq.serialize() << "@" <<
			this->linkQuality.serialize() << "@" <<
			serializeTrace();
	return ss.str();
}

void ARREQ::parse(std::string str) {
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
	this->setTTL(atoi(tokens[7].c_str()));
	QoSRequirement qosReq = QoSRequirement::parse(tokens[8]);
	this->setQosReq(qosReq);
	LinkQuality linkQuality = LinkQuality::parse(tokens[9]);
	this->setLinkQuality(linkQuality);

	if(tokens.size() == 11) parseTrace(tokens[10]);
}

void ARREQ::addTrace(uint32_t nodeId) {
	this->trace.push_back(nodeId);
}

void ARREQ::removeLastTrace(){
	this->trace.erase(this->trace.begin() + this->trace.size() - 1);
}

LinkQuality* ARREQ::getLinkQuality() {
	return &linkQuality;
}

void ARREQ::setLinkQuality(const LinkQuality linkQuality) {
	this->linkQuality = linkQuality;
}

int ARREQ::getTTL() const {
	return this->ttl;
}

void ARREQ::setTTL(int ttl) {
	this->ttl = ttl;
}

void ARREQ::decrementTTL() {
	if(this->ttl > 0) this->ttl--;
}
