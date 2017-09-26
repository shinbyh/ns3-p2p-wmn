/*
 * route_arerr.h
 *
 *  Created on: Jan 18, 2017
 *      Author: bhshin
 */

#ifndef SCRATCH_BHSHIN_ROUTE_ARERR_H_
#define SCRATCH_BHSHIN_ROUTE_ARERR_H_

#include "flow.h"
#include "qos_requirement.h"
#include <string>

class ARERR {
private:
	Flow flow;
	uint32_t originator;
	QoSRequirement appReq;
public:
	ARERR();
	ARERR(Flow flow, uint32_t originator, QoSRequirement appReq);
	virtual ~ARERR();
	const QoSRequirement getAppReq() const;
	void setAppReq(const QoSRequirement appReq);
	const Flow getFlow() const;
	void setFlow(const Flow flow);
	uint32_t getOriginator() const;
	void setOriginator(uint32_t originator);
	std::string serialize();
	static ARERR parse(std::string str);
};

#endif /* SCRATCH_BHSHIN_ROUTE_ARERR_H_ */
