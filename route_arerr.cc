/*
 * route_arerr.cc
 *
 *  Created on: Jan 18, 2017
 *      Author: bhshin
 */

#include "route_arerr.h"
#include "my_config.h"
#include "string_tokenizer.h"
#include <sstream>

ARERR::ARERR() {
	this->originator = 0;
}

ARERR::ARERR(Flow flow, uint32_t originator, QoSRequirement appReq) {
	this->flow = flow;
	this->originator = originator;
	this->appReq = appReq;
}

ARERR::~ARERR() {
}

const QoSRequirement ARERR::getAppReq() const {
	return appReq;
}

void ARERR::setAppReq(const QoSRequirement appReq) {
	this->appReq = appReq;
}

const Flow ARERR::getFlow() const {
	return flow;
}

void ARERR::setFlow(const Flow flow) {
	this->flow = flow;
}

uint32_t ARERR::getOriginator() const {
	return originator;
}

void ARERR::setOriginator(uint32_t originator) {
	this->originator = originator;
}

std::string ARERR::serialize() {
	std::stringstream ss;
	ss << std::fixed;
	ss << ROUTE_ARERR << "@" <<
			this->flow.getSrc() << "@" <<
			this->flow.getSrcPort() << "@" <<
			this->flow.getDst() << "@" <<
			this->flow.getDstPort() << "@" <<
			this->flow.getTypeStr() << "@" <<
			this->originator << "@" <<
			this->appReq.serialize();
	return ss.str();
}

ARERR ARERR::parse(std::string str) {
	std::vector<std::string> tokens;
	tokenizeString(str, tokens, "@");
	// tokens[0] is an message type indicator. skip it.

	uint32_t src = atoi(tokens[1].c_str());
	int srcPort = atoi(tokens[2].c_str());
	uint32_t dst = atoi(tokens[3].c_str());
	int dstPort = atoi(tokens[4].c_str());
	FlowType::Type type = checkType(tokens[5]);
	Flow flow(src, srcPort, dst, dstPort, type);

	uint32_t originator = atoi(tokens[6].c_str());
	QoSRequirement qosReq = QoSRequirement::parse(tokens[7]);

	return ARERR(flow, originator, qosReq);
}
