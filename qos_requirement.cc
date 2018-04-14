/*
 * qos_requirement.cc
 *
 *  Created on: Jan 17, 2017
 *      Author: bhshin
 */

#include "qos_requirement.h"
#include <sstream>
#include "string_tokenizer.h"
#include <boost/format.hpp>

#include "ns3/core-module.h"

using namespace boost;

QoSRequirement::QoSRequirement() {
	this->bandwidth = 0.0;
	this->delay = 0.0;
	this->jitter = 0.0;
	this->lossRate = 0.0;
}

QoSRequirement::QoSRequirement(double bandwidth, double delay, double jitter,
		double lossRate) {
	this->bandwidth = bandwidth;
	this->delay = delay;
	this->jitter = jitter;
	this->lossRate = lossRate;
}

double QoSRequirement::getBandwidth() const {
	return bandwidth;
}

void QoSRequirement::setBandwidth(double bandwidth) {
	this->bandwidth = bandwidth;
}

double QoSRequirement::getDelay() const {
	return delay;
}

void QoSRequirement::setDelay(double delay) {
	this->delay = delay;
}

double QoSRequirement::getJitter() const {
	return jitter;
}

void QoSRequirement::setJitter(double jitter) {
	this->jitter = jitter;
}

double QoSRequirement::getLossRate() const {
	return lossRate;
}

void QoSRequirement::setLossRate(double lossRate) {
	this->lossRate = lossRate;
}

QoSRequirement::~QoSRequirement() {
}

QoSRequirement QoSRequirement::parse(string str) {
	vector<std::string> tokens;
	tokenizeString(str, tokens, "#");
	double bandwidth = atof(tokens[0].c_str());
	double delay = atof(tokens[1].c_str());
	double jitter = atof(tokens[2].c_str());
	double lossRate = atof(tokens[3].c_str());

	return QoSRequirement(bandwidth, delay, jitter, lossRate);
}

string QoSRequirement::serialize() {
	stringstream ss;
	ss << std::fixed;
	ss << format("%.2f") % this->bandwidth << "#" <<
			format("%.2f") % this->delay << "#" <<
			format("%.2f") % this->jitter << "#" <<
			format("%.2f") % this->lossRate;

	return ss.str();
}

bool QoSRequirement::isSatisfactory(const QoSRequirement& qosReq,
		const LinkQuality& lq) {

	// debug
	NS_LOG_UNCOND("    ---- qosReq: " << qosReq.getBandwidth() << ", lq: " << lq.getBandwidth());

	if(qosReq.getBandwidth() * 0.9 > lq.getBandwidth()){
		return false;
	}

	return true;
}
