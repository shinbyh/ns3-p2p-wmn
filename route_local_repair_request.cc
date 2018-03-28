/*
 * route_local_repair_request.cc
 *
 *  Created on: Mar 23, 2018
 *      Author: bhshin
 */

#include "route_local_repair_request.h"
#include "my_config.h"
#include "string_tokenizer.h"
#include <sstream>

LocalRepairRequest::LocalRepairRequest() {
	this->nextHopToSrc = NODEID_NOT_FOUND;
	this->previousNextHop = NODEID_NOT_FOUND;
}

LocalRepairRequest::~LocalRepairRequest() {

}

const LinkQuality& LocalRepairRequest::getEndToEndQuality() const {
	return endToEndQuality;
}

void LocalRepairRequest::setEndToEndQuality(
		const LinkQuality& endToEndQuality) {
	this->endToEndQuality = endToEndQuality;
}

const Flow& LocalRepairRequest::getFlow() const {
	return flow;
}

void LocalRepairRequest::setFlow(const Flow& flow) {
	this->flow = flow;
}

uint32_t LocalRepairRequest::getNextHopToSrc() const {
	return nextHopToSrc;
}

void LocalRepairRequest::setNextHopToSrc(uint32_t nextHopToSrc) {
	this->nextHopToSrc = nextHopToSrc;
}

uint32_t LocalRepairRequest::getPreviousNextHop() const {
	return previousNextHop;
}

void LocalRepairRequest::setPreviousNextHop(uint32_t previousNextHop) {
	this->previousNextHop = previousNextHop;
}

const QoSRequirement& LocalRepairRequest::getQosReq() const {
	return qosReq;
}

void LocalRepairRequest::setQosReq(const QoSRequirement& qosReq) {
	this->qosReq = qosReq;
}

const std::vector<uint32_t>& LocalRepairRequest::getSrcRoute() const {
	return srcRoute;
}

void LocalRepairRequest::setSrcRoute(const std::vector<uint32_t>& srcRoute) {
	this->srcRoute = srcRoute;
}

std::string LocalRepairRequest::serializeTrace(){
	stringstream ss;
	for(size_t i=0; i<this->srcRoute.size(); i++){
		if(i + 1 < this->srcRoute.size())
			ss << this->srcRoute[i] << ",";
		else
			ss << this->srcRoute[i];
	}
	return ss.str();
}

std::string LocalRepairRequest::serialize() {
	stringstream ss;
	ss << std::fixed;
	ss << ROUTE_LOCAL_REPAIR_REQUEST << "@" <<
			this->flow.getSrc() << "@" <<
			this->flow.getSrcPort() << "@" <<
			this->flow.getDst() << "@" <<
			this->flow.getDstPort() << "@" <<
			this->flow.getTypeStr() << "@" <<
			this->nextHopToSrc << "@" <<
			this->previousNextHop << "@" <<
			this->qosReq.serialize() << "@" <<
			this->endToEndQuality.serialize() << "@" <<
			serializeTrace();
	return ss.str();
}

vector<uint32_t> LocalRepairRequest::parseTrace(std::string str){
	vector<uint32_t> ids;
	std::vector<std::string> tokens;
	tokenizeString(str, tokens, ",");
	for(std::string addr : tokens){
		ids.push_back(atoi(addr.c_str()));
	}
	return ids;
}

LocalRepairRequest LocalRepairRequest::parse(std::string str) {
	std::vector<std::string> tokens;
	tokenizeString(str, tokens, "@");
	// tokens[0] is an message type indicator. skip it.

	uint32_t src = atoi(tokens[1].c_str());
	int srcPort = atoi(tokens[2].c_str());
	uint32_t dst = atoi(tokens[3].c_str());
	int dstPort = atoi(tokens[4].c_str());
	FlowType::Type type = checkType(tokens[5]);
	Flow flow(src, srcPort, dst, dstPort, type);
	uint32_t nextHopToSrc = atoi(tokens[6].c_str());
	uint32_t previousNextHop = atoi(tokens[7].c_str());
	QoSRequirement qosReq = QoSRequirement::parse(tokens[8]);
	LinkQuality e2elq = LinkQuality::parse(tokens[9]);
	std::vector<uint32_t> srcRoute = LocalRepairRequest::parseTrace(tokens[10]);

	LocalRepairRequest lrreq;
	lrreq.setFlow(flow);
	lrreq.setQosReq(qosReq);
	lrreq.setEndToEndQuality(e2elq);
	lrreq.setNextHopToSrc(nextHopToSrc);
	lrreq.setPreviousNextHop(previousNextHop);
	lrreq.setSrcRoute(srcRoute);
	return lrreq;
}
