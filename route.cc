/*
 * route.cc
 *
 *  Created on: Nov 1, 2016
 *      Author: bhshin
 */

#include "route.h"
#include "route_id_marker.h"
#include <sstream>
#include <boost/format.hpp>

using namespace boost;

Route::Route(Flow flow, uint32_t nextHop, int hopCount) {
	this->flow = flow;
	this->nextHop = nextHop;
	this->hopCount = hopCount;
	this->outgoingIface = 0;

	this->markID = RouteIDMarker::instance().getNextMarkID();
	this->rtTableNumber = RT_MARK_NOT_ALLOCATED;
	this->tcBandwidth = BW_BEST_EFFORT;
	this->tcClassID = this->markID;

	this->pktsPerSec = 0;
	this->lastUpdateTime = ns3::Seconds(1.0);
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
	ss << format("%4d %4d %6d %4d %6d %5d %7d %6d %5d")
			% markID
			% flow.getSrc()
			% flow.getSrcPort()
			% flow.getDst()
			% flow.getDstPort()
			% flow.getType()
			% nextHop
			% hopCount
			% outgoingIface;
	return ss.str();
}

const Flow Route::getFlow() const {
	return this->flow;
}

int Route::getPktsPerSec() const {
	return pktsPerSec;
}

void Route::setPktsPerSec(int pktsPerSec) {
	this->pktsPerSec = pktsPerSec;
}

void Route::incrementPktsPerSec() {
	this->pktsPerSec++;
}

const ns3::Time& Route::getLastUpdateTime() const {
	return lastUpdateTime;
}

void Route::setLastUpdateTime(const ns3::Time& lastUpdateTime) {
	this->lastUpdateTime = lastUpdateTime;
}
