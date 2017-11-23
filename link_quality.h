/*
 * link_quality.h
 *
 *  Created on: Jan 18, 2017
 *      Author: bhshin
 */

#ifndef SCRATCH_BHSHIN_LINK_QUALITY_H_
#define SCRATCH_BHSHIN_LINK_QUALITY_H_

#include "neighbor_entry.h"
#include <string>

class LinkQuality {
private:
	double bandwidth; // available bandwidth
	double delay;
	double jitter;
	double lossRate;

public:
	LinkQuality();
	LinkQuality(double bandwidth, double delay, double jitter, double lossRate);
	LinkQuality(LinkQuality* lq);
	virtual ~LinkQuality();
	const double getBandwidth() const;
	void setBandwidth(double bandwidth);
	const double getDelay() const;
	void setDelay(double delay);
	const double getJitter() const;
	void setJitter(double jitter);
	const double getLossRate() const;
	void setLossRate(double lossRate);
	int compare(const LinkQuality target) const;
	std::string serialize();
	static LinkQuality parse(std::string str);
};

#endif /* SCRATCH_BHSHIN_LINK_QUALITY_H_ */
