/*
 * route_table.h
 *
 *  Created on: Nov 1, 2016
 *      Author: bhshin
 */

#ifndef ROUTING_ROUTE_TABLE_H_
#define ROUTING_ROUTE_TABLE_H_

#include <map>
#include "flow.h"
#include "route.h"
#include "ns3/core-module.h"
#include "ns3/ipv4-address.h"

class RouteTable {
private:
	std::map<Flow, Route*> routeTable;
	//void getDefaultRouteTable();

public:
	RouteTable();
	virtual ~RouteTable();
	static RouteTable& instance(){
		static RouteTable instance;
		return instance;
	}
	std::string getSubnet(ns3::Ipv4Address ip);
	uint32_t getNextHop(Flow flow);
	Route* getRoute(Flow flow);
	void addRoute(Route* route);
	void updateRoute(Flow flow, uint32_t nextHop, int outgoingIface, double bandwidth);
	void deleteRoute(Flow flow);
	std::string toString();
};

#endif /* ROUTING_ROUTE_TABLE_H_ */
