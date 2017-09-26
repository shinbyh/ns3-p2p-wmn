/*
 * path_probe.cpp
 *
 *  Created on: Apr 13, 2017
 *      Author: bhshin
 */

#include "my_config.h"
#include "route_pathprobe.h"
#include "string_tokenizer.h"

PathProbe::PathProbe() {
	this->hopCount = 0;
	this->routeSeqNo = 0;
}

PathProbe::PathProbe(Flow flow, QoSRequirement qosReq) {
	this->flow = flow;
	this->hopCount = 0;
	this->routeSeqNo = 0;
	this->qosReq = qosReq;
}

PathProbe::PathProbe(Flow flow, int hopCount, int routeSeqNo,
		QoSRequirement qosReq, LinkQuality linkQuality) {
	this->flow = flow;
	this->hopCount = hopCount;
	this->routeSeqNo = routeSeqNo;
	this->qosReq = qosReq;
	this->linkQuality = linkQuality;
}

PathProbe::~PathProbe() {
}

std::string PathProbe::serialize() {
	std::stringstream ss;
	ss << std::fixed;
	ss << PATH_PROBE << "@" <<
			this->flow.getSrc() << "@" <<
			this->flow.getSrcPort() << "@" <<
			this->flow.getDst() << "@" <<
			this->flow.getDstPort() << "@" <<
			this->flow.getTypeStr() << "@" <<
			this->hopCount << "@" <<
			this->routeSeqNo << "@" <<
			this->qosReq.serialize() << "@" <<
			this->linkQuality.serialize() << "@" <<
			serializeTrace();

	return ss.str();
}

void PathProbe::parse(std::string str) {
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

	this->setHopCount(atoi(tokens[6].c_str()));
	this->setRouteSeqNo(atoi(tokens[7].c_str()));
	QoSRequirement qosReq = QoSRequirement::parse(tokens[8]);
	this->setQosReq(qosReq);
	LinkQuality lq = LinkQuality::parse(tokens[9]);
	this->setLinkQuality(lq);
	parseTrace(tokens[10]);
}

QoSRequirement PathProbe::getQosReq() {
	return qosReq;
}

void PathProbe::setQosReq(const QoSRequirement& qosReq) {
	this->qosReq = qosReq;
}

const Flow& PathProbe::getFlow() const {
	return flow;
}

void PathProbe::setFlow(const Flow& flow) {
	this->flow = flow;
}

int PathProbe::getHopCount() const {
	return hopCount;
}

void PathProbe::setHopCount(int hopCount) {
	this->hopCount = hopCount;
}

LinkQuality* PathProbe::getLinkQuality() {
	return &linkQuality;
}

void PathProbe::setLinkQuality(const LinkQuality& linkQuality) {
	this->linkQuality = linkQuality;
}

int PathProbe::getRouteSeqNo() const {
	return routeSeqNo;
}

void PathProbe::setRouteSeqNo(int routeSeqNo) {
	this->routeSeqNo = routeSeqNo;
}

const std::vector<uint32_t>& PathProbe::getTrace() const {
	return trace;
}

void PathProbe::setTrace(const std::vector<uint32_t>& trace) {
	this->trace = trace;
}

void PathProbe::addTrace(uint32_t nodeId) {
	this->trace.push_back(nodeId);
}

std::string PathProbe::serializeTrace(){
	stringstream ss;
	for(size_t i=0; i<this->trace.size(); i++){
		if(i + 1 < this->trace.size())
			ss << this->trace[i] << ",";
		else
			ss << this->trace[i];
	}
	return ss.str();
}

void PathProbe::parseTrace(std::string str){
	std::vector<std::string> tokens;
	tokenizeString(str, tokens, ",");
	for(std::string addr : tokens){
		this->trace.push_back(atoi(addr.c_str()));
	}
}
