/*
 * flow_request.h
 *
 *  Created on: Feb 6, 2017
 *      Author: bhshin
 */

#ifndef SCRATCH_BHSHIN_FLOW_REQUEST_H_
#define SCRATCH_BHSHIN_FLOW_REQUEST_H_

#include "flow.h"
#include "qos_requirement.h"
#include "ns3/core-module.h"

class FlowRequest {
private:
	Flow flow;
	QoSRequirement qosReq;
	int pktSize;
	int sendingRate;
	ns3::Time startTime;
	ns3::Time duration;
	std::string name;

public:
	FlowRequest();
	FlowRequest(Flow flow, QoSRequirement qosReq, int pktSize, int sendingRate, ns3::Time startTime, ns3::Time duration, string name);
	virtual ~FlowRequest();
	const ns3::Time& getDuration() const;
	void setDuration(const ns3::Time& duration);
	const ns3::Time getEndTime() const;
	const Flow& getFlow() const;
	void setFlow(const Flow& flow);
	int getPktSize() const;
	void setPktSize(int pktSize);
	int getSendingRate() const;
	void setSendingRate(int sendingRate);
	QoSRequirement getQosReq() const;
	void setQosReq(const QoSRequirement& qosReq);
	const ns3::Time& getStartTime() const;
	void setStartTime(const ns3::Time& startTime);
	const std::string& getName() const;
	void setName(const std::string& name);
};

#endif /* SCRATCH_BHSHIN_FLOW_REQUEST_H_ */
