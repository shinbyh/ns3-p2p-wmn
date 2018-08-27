/*
 * route_local_repair_request.h
 *
 *  Created on: Mar 23, 2018
 *      Author: bhshin
 */

#ifndef SCRATCH_P2P_BHSHIN_ROUTE_LOCAL_REPAIR_REQUEST_H_
#define SCRATCH_P2P_BHSHIN_ROUTE_LOCAL_REPAIR_REQUEST_H_

#include "flow.h"
#include "qos_requirement.h"
#include "link_quality.h"
#include <vector>

class LocalRepairRequest {
private:
	Flow flow;
	int seqNo;
	QoSRequirement qosReq;
	LinkQuality endToEndQuality;
	uint32_t nextHopToSrc;
	uint32_t previousNextHop;
	std::vector<uint32_t> srcRoute;

	std::string serializeTrace();

public:
	LocalRepairRequest();
	virtual ~LocalRepairRequest();
	const LinkQuality& getEndToEndQuality() const;
	void setEndToEndQuality(const LinkQuality& endToEndQuality);
	const Flow& getFlow() const;
	void setFlow(const Flow& flow);
	uint32_t getNextHopToSrc() const;
	void setNextHopToSrc(uint32_t nextHopToSrc);
	uint32_t getPreviousNextHop() const;
	void setPreviousNextHop(uint32_t previousNextHop);
	const QoSRequirement& getQosReq() const;
	void setQosReq(const QoSRequirement& qosReq);
	const std::vector<uint32_t>& getSrcRoute() const;
	void setSrcRoute(const std::vector<uint32_t>& srcRoute);

	std::string serialize();
	static vector<uint32_t> parseTrace(std::string str);
	static LocalRepairRequest parse(std::string str);
	int getSeqNo() const;
	void setSeqNo(int seqNo);
};

#endif /* SCRATCH_P2P_BHSHIN_ROUTE_LOCAL_REPAIR_REQUEST_H_ */
