/*
 * route.cc
 *
 *  Created on: Nov 1, 2016
 *      Author: bhshin
 */

#include "route.h"
#include "route_id_marker.h"
#include <sstream>

Route::Route(Flow flow, uint32_t nextHop) {
	this->flow = flow;
	this->nextHop = nextHop;
	this->outgoingIface = 0;
	this->hopCount = 0;

	this->markID = RouteIDMarker::instance().getNextMarkID();
	this->rtTableNumber = RT_MARK_NOT_ALLOCATED;
	this->tcBandwidth = BW_BEST_EFFORT;
	this->tcClassID = this->markID;
}

int Route::getHopCount() const {
	return hopCount;
}

void Route::setHopCount(int hopCount) {
	this->hopCount = hopCount;
}

int Route::getMarkId() const {
	return markID;
}

const uint32_t Route::getNextHop() const {
	return nextHop;
}

void Route::setNextHop(const uint32_t nextHop) {
	this->nextHop = nextHop;
}

const int Route::getOutgoingIface() const {
	return outgoingIface;
}

void Route::setOutgoingIface(const int outgoingIface) {
	this->outgoingIface = outgoingIface;
}

int Route::getRtTableNumber() const {
	return rtTableNumber;
}

void Route::setRtTableNumber(int rtTableNumber) {
	this->rtTableNumber = rtTableNumber;
}

double Route::getTcBandwidth() const {
	return tcBandwidth;
}

void Route::setTcBandwidth(double tcBandwidth) {
	this->tcBandwidth = tcBandwidth;
}

int Route::getTcClassId() const {
	return tcClassID;
}

Route::~Route() {
}

const std::string Route::toString() {
	std::stringstream ss;
	ss << std::fixed;

	ss << markID <<
		"\t" << flow.getSrc() << ":" << flow.getSrcPort() <<
		"\t" << flow.getDst() << ":" << flow.getDstPort() <<
		"\t" << flow.getType() <<
		"\t" << nextHop <<
		"\t" << hopCount <<
		"\t" << outgoingIface <<
		"\t" << tcBandwidth << "kbps" <<
		"\t" << rtTableNumber;

	return ss.str();
}

const Flow Route::getFlow() const {
	return this->flow;
}
