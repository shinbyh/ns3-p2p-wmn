/*
 * route_local_repair_reply.cc
 *
 *  Created on: Jun 16, 2018
 *      Author: bhshin
 */

#include "route_local_repair_reply.h"
#include "my_config.h"
#include "string_tokenizer.h"
#include <sstream>

LocalRepairReply::LocalRepairReply(Flow flow, bool repaired) {
	this->flow = flow;
	this->repaired = repaired;
}

LocalRepairReply::~LocalRepairReply() {
}

const Flow& LocalRepairReply::getFlow() const {
	return flow;
}

void LocalRepairReply::setFlow(const Flow& flow) {
	this->flow = flow;
}

//const QoSRequirement& LocalRepairReply::getQosReq() const {
//	return qosReq;
//}

//void LocalRepairReply::setQosReq(const QoSRequirement& qosReq) {
//	this->qosReq = qosReq;
//}

bool LocalRepairReply::isRepaired() const {
	return repaired;
}

void LocalRepairReply::setRepaired(bool repaired) {
	this->repaired = repaired;
}

std::string LocalRepairReply::serialize() {
	stringstream ss;
	ss << std::fixed;
	ss << ROUTE_LOCAL_REPAIR_REPLY << "@" <<
			this->flow.getSrc() << "@" <<
			this->flow.getSrcPort() << "@" <<
			this->flow.getDst() << "@" <<
			this->flow.getDstPort() << "@" <<
			this->flow.getTypeStr() << "@" <<
			this->repaired;
	return ss.str();
}

LocalRepairReply LocalRepairReply::parse(std::string str) {
	std::vector<std::string> tokens;
	tokenizeString(str, tokens, "@");
	// tokens[0] is an message type indicator. skip it.

	uint32_t src = atoi(tokens[1].c_str());
	int srcPort = atoi(tokens[2].c_str());
	uint32_t dst = atoi(tokens[3].c_str());
	int dstPort = atoi(tokens[4].c_str());
	FlowType::Type type = checkType(tokens[5]);
	Flow flow(src, srcPort, dst, dstPort, type);
	bool repaired = atoi(tokens[6].c_str());

	LocalRepairReply lrrep(flow, repaired);
	return lrrep;
}
