/*
 * route_flow_accept_reply.cc
 *
 *  Created on: Feb 17, 2018
 *      Author: bhshin
 */

#include "route_flow_accept_reply.h"
#include <sstream>
#include "string_tokenizer.h"
#include "my_config.h"


/*FlowAcceptReply::FlowAcceptReply() {
	this->seqNo = 0;
	this->senderId = 9999999;
	this->nextHop = 9999999;
	this->hopCount = 0;
}*/

FlowAcceptReply::FlowAcceptReply(Flow flow, int seqNo, uint32_t senderId,
		uint32_t nextHop, int hopCount) {
	this->flow = flow;
	this->seqNo = seqNo;
	this->senderId = senderId;
	this->nextHop = nextHop;
	this->hopCount = hopCount;
}

const vector<uint32_t>& FlowAcceptReply::getDetourIDs() const {
	return detourIDs;
}

void FlowAcceptReply::setDetourIDs(const vector<uint32_t>& detourIDs) {
	this->detourIDs = detourIDs;
}

const LinkQuality& FlowAcceptReply::getDetourLinkQuality() const {
	return detourLinkQuality;
}

void FlowAcceptReply::setDetourLinkQuality(
		const LinkQuality& detourLinkQuality) {
	this->detourLinkQuality = detourLinkQuality;
}

const Flow& FlowAcceptReply::getFlow() const {
	return flow;
}

void FlowAcceptReply::setFlow(const Flow& flow) {
	this->flow = flow;
}

uint32_t FlowAcceptReply::getNextHop() const {
	return nextHop;
}

void FlowAcceptReply::setNextHop(uint32_t nextHop) {
	this->nextHop = nextHop;
}

uint32_t FlowAcceptReply::getSenderId() const {
	return senderId;
}

void FlowAcceptReply::setSenderId(uint32_t senderId) {
	this->senderId = senderId;
}

int FlowAcceptReply::getSeqNo() const {
	return seqNo;
}

void FlowAcceptReply::setSeqNo(int seqNo) {
	this->seqNo = seqNo;
}

int FlowAcceptReply::getHopCount() const {
	return hopCount;
}

void FlowAcceptReply::setHopCount(int hopCount) {
	this->hopCount = hopCount;
}

FlowAcceptReply::~FlowAcceptReply() {
}

const string FlowAcceptReply::serializeTrace(){
	stringstream ss;
	for(size_t i=0; i<this->detourIDs.size(); i++){
		if(i + 1 < this->detourIDs.size())
			ss << this->detourIDs[i] << ",";
		else
			ss << this->detourIDs[i];
	}
	return ss.str();
}

const string FlowAcceptReply::serialize() {
	stringstream ss;
	ss << std::fixed;
	ss << ROUTE_FLOW_ACCEPT_REPLY << "@" <<
			this->flow.getSrc() << "@" <<
			this->flow.getSrcPort() << "@" <<
			this->flow.getDst() << "@" <<
			this->flow.getDstPort() << "@" <<
			this->flow.getTypeStr() << "@" <<
			this->seqNo << "@" <<
			this->senderId << "@" <<
			this->nextHop << "@" <<
			this->hopCount << "@" <<
			this->detourLinkQuality.serialize() << "@" <<
			serializeTrace();
	return ss.str();
}

/*void FlowAcceptReply::parseTrace(std::string str){
	std::vector<std::string> tokens;
	tokenizeString(str, tokens, ",");
	for(std::string addr : tokens){
		this->detourIDs.push_back(atoi(addr.c_str()));
	}
}*/

vector<uint32_t> FlowAcceptReply::parseDetourIDs(std::string str){
	vector<uint32_t> ids;
	std::vector<std::string> tokens;
	tokenizeString(str, tokens, ",");
	for(std::string addr : tokens){
		ids.push_back(atoi(addr.c_str()));
	}
	return ids;
}

FlowAcceptReply FlowAcceptReply::parse(string str) {
	vector<string> tokens;
	tokenizeString(str, tokens, "@");

	//atoi(tokens[0].c_str()); // message type, not used here.
	uint32_t src = atoi(tokens[1].c_str());
	int srcPort = atoi(tokens[2].c_str());
	uint32_t dst = atoi(tokens[3].c_str());
	int dstPort = atoi(tokens[4].c_str());
	FlowType::Type type = checkType(tokens[5]);
	Flow flow(src, srcPort, dst, dstPort, type);
	//this->setFlow(flow);

	int seqNo = atoi(tokens[6].c_str());
	uint32_t senderId = atoi(tokens[7].c_str());
	uint32_t nextHop = atoi(tokens[8].c_str());
	int hopCount = atoi(tokens[9].c_str());
	LinkQuality detourLinkQuality = LinkQuality::parse(tokens[10]);
	//parseTrace(tokens[11]);
	vector<uint32_t> ids = FlowAcceptReply::parseDetourIDs(tokens[11]);

	FlowAcceptReply reply(flow, seqNo, senderId, nextHop, hopCount);
	reply.setDetourLinkQuality(detourLinkQuality);
	reply.setDetourIDs(ids);
	return reply;
}

