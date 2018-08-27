/*
 * route_src_rt_update.cc
 *
 *  Created on: Dec 4, 2017
 *      Author: bhshin
 */

#include "route_src_rt_update.h"
#include "string_tokenizer.h"
#include "my_config.h"
#include <sstream>

using namespace std;

SourceRouteUpdate::SourceRouteUpdate(){
	this->seqNo = 0;
}

SourceRouteUpdate::SourceRouteUpdate(Flow flow, int seqNo) {
	this->flow = flow;
	this->seqNo = seqNo;
}

SourceRouteUpdate::SourceRouteUpdate(Flow flow, int seqNo, const std::vector<uint32_t>& srcRoute) {
	this->flow = flow;
	this->seqNo = seqNo;
	this->srcRoute = srcRoute;
}

SourceRouteUpdate::~SourceRouteUpdate() {

}

std::string SourceRouteUpdate::serializeTrace(){
	stringstream ss;
	for(size_t i=0; i<this->srcRoute.size(); i++){
		if(i + 1 < this->srcRoute.size())
			ss << this->srcRoute[i] << ",";
		else
			ss << this->srcRoute[i];
	}
	return ss.str();
}

std::string SourceRouteUpdate::serialize() {
	stringstream ss;
	ss << std::fixed;
	ss << ROUTE_SRC_ROUTE_UPDATE << "@" <<
			this->flow.getSrc() << "@" <<
			this->flow.getSrcPort() << "@" <<
			this->flow.getDst() << "@" <<
			this->flow.getDstPort() << "@" <<
			this->flow.getTypeStr() << "@" <<
			this->seqNo << "@" <<
			serializeTrace();
	return ss.str();
}

const Flow& SourceRouteUpdate::getFlow() const {
	return flow;
}

void SourceRouteUpdate::setFlow(const Flow& flow) {
	this->flow = flow;
}

const std::vector<uint32_t>& SourceRouteUpdate::getSrcRoute() const {
	return srcRoute;
}

void SourceRouteUpdate::setSrcRoute(const std::vector<uint32_t>& srcRoute) {
	this->srcRoute = srcRoute;
}

void SourceRouteUpdate::parseTrace(std::string str){
	std::vector<std::string> tokens;
	tokenizeString(str, tokens, ",");
	for(std::string addr : tokens){
		this->srcRoute.push_back(atoi(addr.c_str()));
	}
}

void SourceRouteUpdate::parse(std::string str) {
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
	int seqNo = atoi(tokens[6].c_str());
	this->setSeqNo(seqNo);
	if(tokens.size() == 8) parseTrace(tokens[7]);
}

int SourceRouteUpdate::getSeqNo() const {
	return seqNo;
}

void SourceRouteUpdate::setSeqNo(int seqNo) {
	this->seqNo = seqNo;
}
