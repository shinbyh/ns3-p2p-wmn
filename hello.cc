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
#include <boost/format.hpp>

using namespace boost;

Hello::Hello(){
	this->nodeId = 0;
	this->seqNo = 0;
	this->numOfFlows = 0;
	this->occBW = 0.0;
	this->allocBW = 0.0;
	this->avgOccBW = 0.0;
	this->isRouter = 0;
}

Hello::Hello(uint32_t nodeId, int seqNo, int numOfFlows, double occBW, double allocBW,
		bool isRouter) {
	this->nodeId = nodeId;
	this->seqNo = seqNo;
	this->numOfFlows = numOfFlows;
	this->occBW = occBW;
	this->allocBW = allocBW;
	this->avgOccBW = 0.0;
	this->isRouter = isRouter;
}

uint32_t Hello::getNodeId() const {
	return this->nodeId;
}
void Hello::setNodeId(uint32_t nodeId) {
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

std::string Hello::serializeNeighbors(){
	std::stringstream ss;
	for(size_t i=0; i<this->neighbors.size(); i++){
		if(i + 1 < this->neighbors.size())
			ss << this->neighbors[i] << ",";
		else
			ss << this->neighbors[i];
	}
	return ss.str();
}

void Hello::parseTrace(std::string str){
	std::vector<std::string> tokens;
	tokenizeString(str, tokens, ",");
	for(std::string addr : tokens){
		addNeighbor(atoi(addr.c_str()));
	}
}

const std::string Hello::serialize() {
	std::stringstream ss;
	ss << std::fixed;

	ss << "hello/"
		<< this->nodeId << "/"
		<< this->seqNo << "/"
		<< this->numOfFlows << "/"
		<< format("%.2f") % this->occBW << "/"
		<< format("%.2f") % this->allocBW << "/"
		<< format("%.2f") % this->avgOccBW << "/" // Scheme 1.
		<< this->isRouter << "/"
		<< serializeNeighbors();

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
	this->setAvgOccBw(atof(tokens[6].c_str())); // Scheme 1.
	this->setIsRouter(atoi(tokens[7].c_str()));

	if(tokens.size() == 9) parseTrace(tokens[8]);
}

double Hello::getAvgOccBw() const {
	return avgOccBW;
}

const std::vector<uint32_t>& Hello::getNeighbors() const {
	return neighbors;
}

void Hello::setNeighbors(const std::vector<uint32_t>& neighbors) {
	this->neighbors = neighbors;
}

void Hello::setAvgOccBw(double avgOccBw) {
	this->avgOccBW = avgOccBw;
}

void Hello::addNeighbor(uint32_t nodeId) {
	this->neighbors.push_back(nodeId);
}
