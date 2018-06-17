/*
 * route_local_repair_reply.h
 *
 *  Created on: Jun 16, 2018
 *      Author: bhshin
 */

#ifndef SCRATCH_P2P_BHSHIN_ROUTE_LOCAL_REPAIR_REPLY_H_
#define SCRATCH_P2P_BHSHIN_ROUTE_LOCAL_REPAIR_REPLY_H_

#include "flow.h"
#include "qos_requirement.h"

class LocalRepairReply {
private:
	Flow flow;
	//QoSRequirement qosReq;
	bool repaired; // Whether the local repair is successful or not

public:
	LocalRepairReply(Flow flow, bool repaired);
	virtual ~LocalRepairReply();
	const Flow& getFlow() const;
	void setFlow(const Flow& flow);
	//const QoSRequirement& getQosReq() const;
	//void setQosReq(const QoSRequirement& qosReq);
	bool isRepaired() const;
	void setRepaired(bool repaired);
	std::string serialize();
	static LocalRepairReply parse(std::string str);
};

#endif /* SCRATCH_P2P_BHSHIN_ROUTE_LOCAL_REPAIR_REPLY_H_ */
