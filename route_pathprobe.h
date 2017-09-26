/*
 * path_probe.h
 *
 *  Created on: Apr 13, 2017
 *      Author: bhshin
 */

#ifndef SCRATCH_BHSHIN_PATH_PROBE_H_
#define SCRATCH_BHSHIN_PATH_PROBE_H_

#include "flow.h"
#include "qos_requirement.h"
#include "link_quality.h"

class PathProbe {
private:
	Flow flow;
	int hopCount;
	int routeSeqNo;
	QoSRequirement qosReq;
	LinkQuality linkQuality;
	std::vector<uint32_t> trace;

	std::string serializeTrace();
	void parseTrace(std::string str);
public:
	PathProbe();
	PathProbe(Flow flow, QoSRequirement qosReq);
	PathProbe(Flow flow, int hopCount, int routeSeqNo, QoSRequirement qosReq, LinkQuality linkQuality);
	virtual ~PathProbe();

	std::string serialize();
	void parse(std::string str);
	const Flow& getFlow() const;
	void setFlow(const Flow& flow);
	int getHopCount() const;
	void setHopCount(int hopCount);
	LinkQuality* getLinkQuality();
	void setLinkQuality(const LinkQuality& linkQuality);
	QoSRequirement getQosReq();
	void setQosReq(const QoSRequirement& qosReq);
	int getRouteSeqNo() const;
	void setRouteSeqNo(int routeSeqNo);
	const std::vector<uint32_t>& getTrace() const;
	void setTrace(const std::vector<uint32_t>& trace);
	void addTrace(uint32_t nodeId);
};

#endif /* SCRATCH_BHSHIN_PATH_PROBE_H_ */
