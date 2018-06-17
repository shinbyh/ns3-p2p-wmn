/*
 * route_path_quality_report.cc
 *
 *  Created on: Jun 14, 2018
 *      Author: bhshin
 */

#include "route_path_quality_report.h"
#include "my_config.h"
#include "string_tokenizer.h"
#include <sstream>


PathQualityReport::PathQualityReport(Flow flow, int hopCount) {
	this->flow = flow;
	this->hopCount = hopCount;
}

const LinkQuality& PathQualityReport::getEndToEndQuality() const {
	return endToEndQuality;
}

void PathQualityReport::setEndToEndQuality(const LinkQuality& endToEndQuality) {
	this->endToEndQuality = endToEndQuality;
}

const Flow& PathQualityReport::getFlow() const {
	return flow;
}

void PathQualityReport::setFlow(const Flow& flow) {
	this->flow = flow;
}

int PathQualityReport::getHopCount() const {
	return this->hopCount;
}

void PathQualityReport::setHopCount(int hopCount) {
	this->hopCount = hopCount;
}

const QoSRequirement& PathQualityReport::getQosReq() const {
	return qosReq;
}

void PathQualityReport::setQosReq(const QoSRequirement& qosReq) {
	this->qosReq = qosReq;
}

string PathQualityReport::serialize() {
	stringstream ss;
	ss << std::fixed;
	ss << ROUTE_PATH_QUALITY_REPORT << "@" <<
			this->flow.getSrc() << "@" <<
			this->flow.getSrcPort() << "@" <<
			this->flow.getDst() << "@" <<
			this->flow.getDstPort() << "@" <<
			this->flow.getTypeStr() << "@" <<
			this->hopCount << "@" <<
			this->qosReq.serialize() << "@" <<
			this->endToEndQuality.serialize();
	return ss.str();
}

PathQualityReport PathQualityReport::parse(string str) {
	std::vector<std::string> tokens;
	tokenizeString(str, tokens, "@");
	// tokens[0] is an message type indicator. skip it.

	uint32_t src = atoi(tokens[1].c_str());
	int srcPort = atoi(tokens[2].c_str());
	uint32_t dst = atoi(tokens[3].c_str());
	int dstPort = atoi(tokens[4].c_str());
	FlowType::Type type = checkType(tokens[5]);
	Flow flow(src, srcPort, dst, dstPort, type);
	int hopCount = atoi(tokens[6].c_str());
	QoSRequirement qosReq = QoSRequirement::parse(tokens[7]);
	LinkQuality e2elq = LinkQuality::parse(tokens[8]);

	PathQualityReport pqr(flow, hopCount);
	pqr.setQosReq(qosReq);
	pqr.setEndToEndQuality(e2elq);
	return pqr;
}

PathQualityReport::~PathQualityReport() {
}
