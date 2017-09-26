/*
 * hello.cc
 *
 *  Created on: Dec 7, 2016
 *      Author: bhshin
 */

#include "hello.h"
#include <sstream>
#include <string>
#include "string_tokenizer.h"

Hello::Hello(){
	this->nodeId = 0;
	this->seqNo = 0;
	this->numOfFlows = 0;
	this->occBW = 0.0;
	this->allocBW = 0.0;
	this->isRouter = 0;
}

Hello::Hello(int nodeId, int seqNo, int numOfFlows, double occBW, double allocBW,
		bool isRouter) {
	this->nodeId = nodeId;
	this->seqNo = seqNo;
	this->numOfFlows = numOfFlows;
	this->occBW = occBW;
	this->allocBW = allocBW;
	this->isRouter = isRouter;
}

int Hello::getNodeId() const {
	return this->nodeId;
}
void Hello::setNodeId(int nodeId) {
	this->nodeId = nodeId;
}

double Hello::getAllocBw() const {
	return allocBW;
}

void Hello::setAllocBw(double allocBw) {
	this->allocBW = allocBw;
}

bool Hello::isIsRouter() const {
	return isRouter;
}

void Hello::setIsRouter(bool isRouter) {
	this->isRouter = isRouter;
}

int Hello::getNumOfFlows() const {
	return numOfFlows;
}

void Hello::setNumOfFlows(int numOfFlows) {
	this->numOfFlows = numOfFlows;
}

double Hello::getOccBw() const {
	return occBW;
}

void Hello::setOccBw(double occBw) {
	this->occBW = occBw;
}

int Hello::getSeqNo() const {
	return seqNo;
}

void Hello::setSeqNo(int seqNo) {
	this->seqNo = seqNo;
}

const std::string Hello::serialize() {
	std::stringstream ss;
	ss << std::fixed;

	ss << "hello/"
		<< this->nodeId << "/"
		<< this->seqNo << "/"
		<< this->numOfFlows << "/"
		<< this->occBW << "/"
		<< this->allocBW << "/"
		<< this->isRouter;

	return ss.str();
}

void Hello::parse(std::string line) {
	std::vector<std::string> tokens;
	tokenizeString(line, tokens, "/");

	this->setNodeId(atoi(tokens[1].c_str()));
	this->setSeqNo(atoi(tokens[2].c_str()));
	this->setNumOfFlows(atoi(tokens[3].c_str()));
	this->setOccBw(atof(tokens[4].c_str()));
	this->setAllocBw(atof(tokens[5].c_str()));
	this->setIsRouter(atoi(tokens[6].c_str()));
}
