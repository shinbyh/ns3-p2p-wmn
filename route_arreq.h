/*
 * route_arreq.h
 *
 *  Created on: Jan 17, 2017
 *      Author: bhshin
 */

#ifndef SCRATCH_BHSHIN_ROUTE_ARREQ_H_
#define SCRATCH_BHSHIN_ROUTE_ARREQ_H_

#include "flow.h"
#include "qos_requirement.h"
#include "link_quality.h"
#include "ns3/ipv4-address.h"
#include <vector>
#include <string>

class ARREQ {
private:
	Flow flow;
	int seqNo;
	int ttl;
	QoSRequirement qosReq;
	std::vector<uint32_t> trace;
	LinkQuality linkQuality;
	std::string serializeTrace();
	void parseTrace(std::string str);

public:
	ARREQ();
	ARREQ(Flow flow, int seqNo, int ttl, QoSRequirement qosReq);
	virtual ~ARREQ();
	const Flow getFlow() const;
	void setFlow(const Flow flow);
	QoSRequirement getQosReq();
	void setQosReq(const QoSRequirement qosReq);
	int getSeqNo() const;
	void setSeqNo(int seqNo);
	int getTTL() const;
	void setTTL(int ttl);
	void decrementTTL();
	const std::vector<uint32_t> getTrace() const;
	void setTrace(const std::vector<uint32_t> trace);
	std::string serialize();
	void parse(std::string str);
	void addTrace(uint32_t nodeId);
	void removeLastTrace();
	LinkQuality* getLinkQuality();
	void setLinkQuality(const LinkQuality linkQuality);
};

#endif /* SCRATCH_BHSHIN_ROUTE_ARREQ_H_ */
