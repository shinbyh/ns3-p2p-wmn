/*
 * my_ns3_packet.cc
 *
 *  Created on: Jan 11, 2017
 *      Author: bhshin
 */

#include "my_ns3_packet.h"
#include <sstream>
#include "string_tokenizer.h"

using namespace ns3;

MyNS3Packet::MyNS3Packet(uint32_t src, int appSrcPort, uint32_t dst, int appDstPort, std::string msg) {
	this->src = src;
	this->appSrcPort = appSrcPort;
	this->dst = dst;
	this->appDstPort = appDstPort;
	this->msg = msg;
	this->seqNo = 0;
	this->genTime = MilliSeconds(0);
}

MyNS3Packet::MyNS3Packet() {
	this->appSrcPort = 0;
	this->appDstPort = 0;
	this->src = 0;
	this->dst = 0;
	this->seqNo = 0;
	this->genTime = MilliSeconds(0);
}

const uint32_t MyNS3Packet::getDst() const {
	return dst;
}

void MyNS3Packet::setDst(uint32_t dst) {
	this->dst = dst;
}

const std::string MyNS3Packet::getMsg() const {
	return msg;
}

void MyNS3Packet::setMsg(const std::string msg) {
	this->msg = msg;
}

const uint32_t MyNS3Packet::getSrc() const {
	return src;
}

void MyNS3Packet::setSrc(uint32_t src) {
	this->src = src;
}

const std::string MyNS3Packet::serialize() {
	std::stringstream ss;
	ss << this->src << "|" <<
			this->appSrcPort << "|" <<
			this->dst << "|" <<
			this->appDstPort << "|" <<
			this->seqNo << "|" <<
			this->genTime.GetMilliSeconds() << "|" <<
			this->msg;

	return ss.str();
}

Ptr<MyNS3Packet> MyNS3Packet::parse(std::string str) {
	Ptr<MyNS3Packet> myPkt = CreateObject<MyNS3Packet>();

	std::vector<std::string> tokens;
	tokenizeString(str, tokens, "|");
	myPkt->setSrc(atoi(tokens[0].c_str()));
	myPkt->setAppSrcPort(atoi(tokens[1].c_str()));
	myPkt->setDst(atoi(tokens[2].c_str()));
	myPkt->setAppDstPort(atoi(tokens[3].c_str()));
	myPkt->setSeqNo(atoi(tokens[4].c_str()));
	ns3::Time genTime = ns3::MilliSeconds(atoi(tokens[5].c_str()));
	myPkt->setGenTime(genTime);
	myPkt->setMsg(tokens[6]);

	return myPkt;
}

const int MyNS3Packet::getAppDstPort() const {
	return appDstPort;
}

void MyNS3Packet::setAppDstPort(int appDstPort) {
	this->appDstPort = appDstPort;
}

const int MyNS3Packet::getAppSrcPort() const {
	return appSrcPort;
}

void MyNS3Packet::setAppSrcPort(int appSrcPort) {
	this->appSrcPort = appSrcPort;
}

MyNS3Packet::~MyNS3Packet() {
}

const int MyNS3Packet::getPktSize() const {
	std::stringstream ss;
	ss << this->src << "|" <<
			this->appSrcPort << "|" <<
			this->dst << "|" <<
			this->appDstPort << "|" <<
			this->msg;

	return ss.str().length() + 10;
}

int MyNS3Packet::getSeqNo() const {
	return seqNo;
}

void MyNS3Packet::setSeqNo(int seqNo) {
	this->seqNo = seqNo;
}

const size_t MyNS3Packet::getDataSize() const {
	return this->msg.size();
}

const ns3::Time& MyNS3Packet::getGenTime() const {
	return genTime;
}

void MyNS3Packet::setGenTime(const ns3::Time& genTime) {
	this->genTime = genTime;
}
