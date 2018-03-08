/*
 * qos_requirement.h
 *
 *  Created on: Jan 17, 2017
 *      Author: bhshin
 */

#ifndef SCRATCH_BHSHIN_QOS_REQUIREMENT_H_
#define SCRATCH_BHSHIN_QOS_REQUIREMENT_H_

#include <string>
#include "link_quality.h"

using namespace std;

class QoSRequirement {
private:
	double bandwidth;
	double delay;
	double jitter;
	double lossRate;

public:
	QoSRequirement();
	QoSRequirement(double bandwidth, double delay, double jitter, double lossRate);
	virtual ~QoSRequirement();
	double getBandwidth() const;
	void setBandwidth(double bandwidth);
	double getDelay() const;
	void setDelay(double delay);
	double getJitter() const;
	void setJitter(double jitter);
	double getLossRate() const;
	void setLossRate(double lossRate);
	static QoSRequirement parse(string str);
	string serialize();
	//bool isSatisfactory(LinkQuality* lq);
	static bool isSatisfactory(const QoSRequirement& qosReq, const LinkQuality& lq);
};

#endif /* SCRATCH_BHSHIN_QOS_REQUIREMENT_H_ */
