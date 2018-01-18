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

FlowCheck::FlowCheck(int msgType) {
	this->msgType = msgType;
	this->seqNo = 0;
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
			this->seqNo << "@" <<
			serializeStats();
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

void FlowCheck::parse(string str) {
	vector<string> tokens;
	tokenizeString(str, tokens, "@");

	if(tokens.size() == 2){
		this->msgType = atoi(tokens[0].c_str());
		this->seqNo = atoi(tokens[1].c_str());
	} else if (tokens.size() == 3){
		this->msgType = atoi(tokens[0].c_str());
		this->seqNo = atoi(tokens[1].c_str());
		parseStats(tokens[2]);
	} else {
		// Error. Invalid message format.
	}
}
