/*
 * link_quality.cc
 *
 *  Created on: Jan 18, 2017
 *      Author: bhshin
 */

#include "link_quality.h"
#include <sstream>
#include "string_tokenizer.h"
#include <boost/format.hpp>

using namespace boost;

LinkQuality::LinkQuality() {
	this->bandwidth = 0.0;
	this->delay = 0.0;
	this->jitter = 0.0;
	this->lossRate = 0.0;
}

LinkQuality::LinkQuality(double bandwidth, double delay, double jitter,
		double lossRate) {
	this->bandwidth = bandwidth;
	this->delay = delay;
	this->jitter = jitter;
	this->lossRate = lossRate;
}

LinkQuality::LinkQuality(LinkQuality* lq) {
	this->bandwidth = lq->getBandwidth();
	this->delay = lq->getDelay();
	this->jitter = lq->getJitter();
	this->lossRate = lq->getLossRate();
}

const double LinkQuality::getBandwidth() const {
	return bandwidth;
}

void LinkQuality::setBandwidth(double bandwidth) {
	this->bandwidth = bandwidth;
}

const double LinkQuality::getDelay() const {
	return delay;
}

void LinkQuality::setDelay(double delay) {
	this->delay = delay;
}

const double LinkQuality::getJitter() const {
	return jitter;
}

void LinkQuality::setJitter(double jitter) {
	this->jitter = jitter;
}

const double LinkQuality::getLossRate() const {
	return lossRate;
}

void LinkQuality::setLossRate(double lossRate) {
	this->lossRate = lossRate;
}

LinkQuality::~LinkQuality() {
}

/**
 * Compares this link quality with the target.
 * If this one has "better" metric value than the target,
 * the return value is positive (>0). Otherwise, the
 * return value is negative (<0).
 * The exactly same metric values will return zero.
 */
int LinkQuality::compare(const LinkQuality target) const {
	int flag = 0;

	if(this->bandwidth > target.getBandwidth()){
		flag++;
	} else if(this->bandwidth < target.getBandwidth()) {
		flag--;
	}

	if(this->delay < target.getDelay()){
		flag++;
	} else {
		flag--;
	}

	return flag;
}

std::string LinkQuality::serialize() {
	std::stringstream ss;
	ss << std::fixed;
	ss << format("%.2f") % this->bandwidth << "#" <<
			format("%.2f") % this->delay << "#" <<
			format("%.2f") % this->jitter << "#" <<
			format("%.2f") % this->lossRate;

	return ss.str();
}

LinkQuality LinkQuality::parse(std::string str) {
	std::vector<std::string> tokens;
	tokenizeString(str, tokens, "#");
	double bandwidth = atof(tokens[0].c_str());
	double delay = atof(tokens[1].c_str());
	double jitter = atof(tokens[2].c_str());
	double lossRate = atof(tokens[3].c_str());

	return LinkQuality(bandwidth, delay, jitter, lossRate);
}
