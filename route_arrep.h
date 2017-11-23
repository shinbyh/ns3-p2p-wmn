/*
 * route_arrep.h
 *
 *  Created on: Jan 18, 2017
 *      Author: bhshin
 */

#ifndef SCRATCH_BHSHIN_ROUTE_ARREP_H_
#define SCRATCH_BHSHIN_ROUTE_ARREP_H_

#include "flow.h"
#include "route_arreq.h"
#include "qos_requirement.h"
#include "link_quality.h"
#include <vector>
#include <string>

class ARREP {
private:
	Flow flow;
	int seqNo;
	QoSRequirement qosReq;
	std::vector<uint32_t> trace;
	LinkQuality linkQuality;
	std::string serializeTrace();
	void parseTrace(std::string str);
public:
	ARREP();
	ARREP(Flow flow, int seqNo, QoSRequirement qosReq, LinkQuality linkQuality, std::vector<uint32_t> trace);
	virtual ~ARREP();
	Flow getFlow();
	void setFlow(const Flow& flow);
	const LinkQuality getLinkQuality() const;
	void setLinkQuality(const LinkQuality linkQuality);
	const QoSRequirement getQosReq() const;
	void setQosReq(const QoSRequirement qosReq);
	int getSeqNo() const;
	void setSeqNo(int seqNo);
	const std::vector<uint32_t> getTrace() const;
	void setTrace(const std::vector<uint32_t> trace);
	void addTrace(uint32_t nodeId);
	void removeLastTrace();
	static ARREP deriveFrom(ARREQ arreq);
	void parse(std::string str);
	std::string serialize();
	int getTraceIndex(uint32_t addr);

};

#endif /* SCRATCH_BHSHIN_ROUTE_ARREP_H_ */
