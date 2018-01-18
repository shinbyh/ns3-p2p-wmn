/*
 * route_table.cc
 *
 *  Created on: Nov 1, 2016
 *      Author: bhshin
 */

#include "route_table.h"
#include "my_config.h"
#include <boost/foreach.hpp>
#include <sstream>

using namespace ns3;

RouteTable::RouteTable() {
	//this->ns3StaticRouting = ns3StaticRouting;
	// add a default route?
}

RouteTable::~RouteTable() {
	std::map<Flow, Route*>::iterator itr;
	for(itr = routeTable.begin(); itr != routeTable.end(); itr++){
		delete itr->second;
	}
}

std::string RouteTable::getSubnet(ns3::Ipv4Address ip) {
	return "null";
}

uint32_t RouteTable::getNextHop(Flow flow) {
	if(routeTable.find(flow) == routeTable.end()){
		// not found
		return NODEID_NOT_FOUND;
	} else {
		// found next hop
		return routeTable[flow]->getNextHop();
	}
}

Route* RouteTable::getRoute(Flow flow) {
	if(routeTable.find(flow) == routeTable.end()){
		// not found.
		// make default flow and return Route* of a default flow
		return 0;
	} else {
		// found next hop
		return routeTable[flow];
	}
}

void RouteTable::addRoute(Route* route) {
	routeTable[route->getFlow()] = route;

	// TODO: apply traffic shaping
}

void RouteTable::updateRoute(Flow flow, uint32_t nextHop, int outgoingIface,
		double bandwidth) {
	if(routeTable.find(flow) == routeTable.end()){
		// not found
	} else {
		Route* route = routeTable[flow];
		route->setNextHop(nextHop);
		route->setOutgoingIface(outgoingIface);
		route->setTcBandwidth(bandwidth);

		// TODO: change traffic shaping (tc)

		// debug
		NS_LOG_UNCOND("[RT] updated route: " << route->toString());
	}
}

void RouteTable::deleteRoute(Flow flow) {
	routeTable.erase(flow);

	// TODO: delete traffic shaping (tc)
}

std::string RouteTable::toString() {
	std::stringstream ss;
	ss << std::fixed;
	ss << "[RT] Routing Table (t=" << Simulator::Now().GetSeconds() << ")\n";
	ss << "mkId  src  sport  dst  dport  type nextHop hopCnt iface\n"
		<< "----------------------------------------------------------\n";

	std::pair<Flow, Route*> p;
	BOOST_FOREACH (p, routeTable){
		Route* route = p.second;
		ss << route->toString() << "\n";
	}

	return ss.str();
}
