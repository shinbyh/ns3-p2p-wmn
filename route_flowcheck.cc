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
	this->nodeId = 9999999;
	this->seqNo = 0;
	this->TTL = TTL;
	this->replierNodeId = 9999999;
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
			this->nodeId << "@" <<
			this->seqNo << "@" <<
			this->TTL << "@";
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

	if(this->msgType == ROUTE_FLOWCHECK_REQUEST){
		this->nodeId = atoi(tokens[1].c_str());
		this->seqNo = atoi(tokens[2].c_str());
		this->TTL =  atoi(tokens[3].c_str());
	} else if(this->msgType == ROUTE_FLOWCHECK_REPLY){
		this->nodeId = atoi(tokens[1].c_str());
		this->seqNo = atoi(tokens[2].c_str());
		this->TTL =  atoi(tokens[3].c_str());
		this->replierNodeId = atoi(tokens[4].c_str());
		if(tokens.size() >= 6) parseStats(tokens[5]);
	} else {
		// Error. Invalid message format.
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
