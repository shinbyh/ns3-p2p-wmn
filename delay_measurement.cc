/*
 * delay_measurement.cc
 *
 *  Created on: Dec 19, 2016
 *      Author: bhshin
 */

#include "delay_measurement.h"
#include "string_tokenizer.h"
#include <sstream>
#include <string>

const std::string& DelayMeasurement::getHeader() const {
	return header;
}

void DelayMeasurement::setHeader(const std::string& header) {
	this->header = header;
}

const ns3::Time& DelayMeasurement::getTimeDifference() const {
	return timeDifference;
}

void DelayMeasurement::setTimeDifference(const ns3::Time& time) {
	this->timeDifference = time;
}

DelayMeasurement::DelayMeasurement() {
	this->seqNo = 0;
}

DelayMeasurement::DelayMeasurement(std::string type, int seqNo, ns3::Time time) {
	this->type = type;
	this->seqNo = seqNo;
	this->timeDifference = time;
}


DelayMeasurement::~DelayMeasurement() {
}

std::string DelayMeasurement::serialize() {
	std::stringstream ss;
	ss << "dm/" <<
		this->type << "/" <<
		this->seqNo << "/" <<
		this->timeDifference;

	return ss.str();
}

void DelayMeasurement::parse(std::string str) {
	std::vector<std::string> tokens;
	tokenizeString(str, tokens, "/");

	this->setHeader(tokens[0]); // not used for now
	this->setType(tokens[1]);
	int seqNo = atoi(tokens[2].c_str());
	this->setSeqNo(seqNo);
	ns3::Time time(atol(tokens[3].c_str()));
	this->setTimeDifference(time);
}



int DelayMeasurement::getSeqNo() const {
	return seqNo;
}

void DelayMeasurement::setSeqNo(int seqNo) {
	this->seqNo = seqNo;
}

const std::string& DelayMeasurement::getType() const {
	return type;
}

void DelayMeasurement::setType(const std::string& type) {
	this->type = type;
}
