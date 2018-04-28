/*
 * route.h
 *
 *  Created on: Nov 1, 2016
 *      Author: bhshin
 */

#ifndef ROUTING_ROUTE_H_
#define ROUTING_ROUTE_H_

#include "flow.h"
#include "ns3/core-module.h"

#define RT_MARK_NOT_ALLOCATED (-100)
#define BW_BEST_EFFORT 0.0

class Route {
private:
	Flow flow;
	uint32_t nextHop;
	int outgoingIface;
	int hopCount;

	/*
	 * TC-related variables
	 */
	int rtTableNumber;
	int markID;
	int tcClassID;
	double tcBandwidth;

public:
	Route(Flow flow, uint32_t nextHop, int hopCount);
	~Route();
	const Flow getFlow() const;
	int getHopCount() const;
	void setHopCount(int hopCount);
	int getMarkId() const;
	const uint32_t getNextHop() const;
	void setNextHop(const uint32_t nextHop);
	const int getOutgoingIface() const;
	void setOutgoingIface(const int outgoingIface);
	int getRtTableNumber() const;
	void setRtTableNumber(int rtTableNumber);
	double getTcBandwidth() const;
	void setTcBandwidth(double tcBandwidth);
	int getTcClassId() const;
	const std::string toString();
};

#endif /* ROUTING_ROUTE_H_ */
