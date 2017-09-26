/*
 * route_rtsetup.h
 *
 *  Created on: Feb 2, 2017
 *      Author: bhshin
 */

#ifndef SCRATCH_BHSHIN_ROUTE_RTSETUP_H_
#define SCRATCH_BHSHIN_ROUTE_RTSETUP_H_

#include "flow.h"
#include "qos_requirement.h"
#include "route_arrep.h"
#include "ns3/ipv4-address.h"
#include <vector>
#include <string>

class RouteSetup {
private:
	Flow flow;
	int seqNo;
	QoSRequirement qosReq;
	LinkQuality endToEndQuality;
	std::vector<uint32_t> trace;

	std::string serializeTrace();
	void parseTrace(std::string str);

public:
	RouteSetup();
	RouteSetup(Flow flow, int seqNo, QoSRequirement qosReq, std::vector<uint32_t> trace);
	virtual ~RouteSetup();
	const Flow& getFlow() const;
	void setFlow(const Flow& flow);
	const QoSRequirement& getQosReq() const;
	void setQosReq(const QoSRequirement& qosReq);
	const LinkQuality& getEndToEndQuality() const;
	void setEndToEndQuality(const LinkQuality& endToEndQuality);
	int getSeqNo() const;
	void setSeqNo(int seqNo);
	const std::vector<uint32_t>& getTrace() const;
	void setTrace(const std::vector<uint32_t>& trace);
	void addTrace(uint32_t addr);
	int getPositionInTrace(uint32_t addr);
	int getPositionInTrace(ns3::Ptr<ns3::Node> node);
	uint32_t getNextHop(uint32_t ip);
	uint32_t getNextHopOfCurrentNode(ns3::Ptr<ns3::Node> node);
	std::string serialize();
	void parse(std::string str);
	void parseFromARREP(ARREP arrep);
};

#endif /* SCRATCH_BHSHIN_ROUTE_RTSETUP_H_ */
