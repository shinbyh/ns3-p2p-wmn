/*
 * route_flowcheck.cc
 *
 *  Created on: Jan 16, 2018
 *      Author: bhshin
 */

#include "route_flowcheck.h"
#include <sstream>
#include "string_tokenizer.h"
#include "my_config.h"

FlowCheck::FlowCheck(int msgType, int TTL) {
	this->msgType = msgType;
	this->nodeId = NODEID_NOT_FOUND;
	this->seqNo = 0;
	this->TTL = TTL;
	this->prevNextHop = NODEID_NOT_FOUND;
	this->replierNodeId = NODEID_NOT_FOUND;
	this->avgAvailableBw = 0.0;
}

FlowCheck::~FlowCheck() {
}


int FlowCheck::getMsgType() const {
	return msgType;
}

void FlowCheck::setMsgType(int msgType) {
	this->msgType = msgType;
}

int FlowCheck::getSeqNo() const {
	return seqNo;
}

void FlowCheck::setSeqNo(int seqNo) {
	this->seqNo = seqNo;
}

const vector<SourceRouteStat>& FlowCheck::getStats() const {
	return stats;
}

void FlowCheck::setStats(const vector<SourceRouteStat>& stats) {
	this->stats = stats;
}

void FlowCheck::addStat(const SourceRouteStat stat) {
	this->stats.push_back(stat);
}

const string FlowCheck::serializeStats() const {
	stringstream ss;
	for(size_t i=0; i<this->stats.size(); i++){
		SourceRouteStat stat = this->stats[i];
		if(i + 1 < this->stats.size())
			ss << stat.serialize() << "#";
		else
			ss << stat.serialize();
	}
	return ss.str();
}

const string FlowCheck::serialize() const {
	stringstream ss;
	ss << std::fixed;
	ss << this->msgType << "@" <<
			this->flow.getSrc() << "@" <<
			this->flow.getSrcPort() << "@" <<
			this->flow.getDst() << "@" <<
			this->flow.getDstPort() << "@" <<
			this->flow.getTypeStr() << "@" <<
			this->nodeId << "@" <<
			this->seqNo << "@" <<
			this->TTL << "@" <<
			this->prevNextHop << "@" <<
			this->avgAvailableBw << "@";
	if(this->msgType == ROUTE_FLOWCHECK_REPLY){
		ss << this->replierNodeId << "@" << serializeStats();
	}
	return ss.str();
}

void FlowCheck::parseStats(string str) {
	vector<string> tokens;
	tokenizeString(str, tokens, "#");
	for(string statStr : tokens){
		SourceRouteStat stat;
		stat.parse(statStr);
		addStat(stat);
	}
}

int FlowCheck::getTtl() const {
	return TTL;
}

void FlowCheck::setTtl(int ttl) {
	TTL = ttl;
}

void FlowCheck::parse(string str) {
	vector<string> tokens;
	tokenizeString(str, tokens, "@");
	this->msgType = atoi(tokens[0].c_str());
	uint32_t src = atoi(tokens[1].c_str());
	int srcPort = atoi(tokens[2].c_str());
	uint32_t dst = atoi(tokens[3].c_str());
	int dstPort = atoi(tokens[4].c_str());
	FlowType::Type type = checkType(tokens[5]);
	Flow flow(src, srcPort, dst, dstPort, type);
	this->flow = flow;

	this->nodeId = atoi(tokens[6].c_str());
	this->seqNo = atoi(tokens[7].c_str());
	this->TTL =  atoi(tokens[8].c_str());
	this->prevNextHop = atoi(tokens[9].c_str());

	if(this->msgType == ROUTE_FLOWCHECK_REPLY){
		this->avgAvailableBw = atof(tokens[10].c_str());
		this->replierNodeId = atoi(tokens[11].c_str());
		if(tokens.size() >= 13) parseStats(tokens[12]);
	}
}

uint32_t FlowCheck::getNodeId() const {
	return nodeId;
}

uint32_t FlowCheck::getReplierNodeId() const {
	return replierNodeId;
}

void FlowCheck::setReplierNodeId(uint32_t replierNodeId) {
	this->replierNodeId = replierNodeId;
}

void FlowCheck::setNodeId(uint32_t nodeId) {
	this->nodeId = nodeId;
}

void FlowCheck::decrementTTL() {
	if(this->TTL > 0) this->TTL--;
}

uint32_t FlowCheck::getPrevNextHop() const {
	return prevNextHop;
}

void FlowCheck::setPrevNextHop(uint32_t prevNextHop) {
	this->prevNextHop = prevNextHop;
}

double FlowCheck::getAvgAvailableBw() const {
	return avgAvailableBw;
}

void FlowCheck::setAvgAvailableBw(double avgAvailableBw) {
	this->avgAvailableBw = avgAvailableBw;
}

const Flow& FlowCheck::getFlow() const {
	return flow;
}

void FlowCheck::setFlow(const Flow& flow) {
	this->flow = flow;
}
