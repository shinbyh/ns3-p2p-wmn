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
}

MyNS3Packet::MyNS3Packet() {
	this->appSrcPort = 0;
	this->appDstPort = 0;
	this->src = 0;
	this->dst = 0;
}

uint32_t MyNS3Packet::getDst() const {
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

uint32_t MyNS3Packet::getSrc() const {
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
	myPkt->setMsg(tokens[4]);

	return myPkt;
}

int MyNS3Packet::getAppDstPort() const {
	return appDstPort;
}

void MyNS3Packet::setAppDstPort(int appDstPort) {
	this->appDstPort = appDstPort;
}

int MyNS3Packet::getAppSrcPort() const {
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
