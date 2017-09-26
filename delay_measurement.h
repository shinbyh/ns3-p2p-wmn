/*
 * delay_measurement.h
 *
 *  Created on: Dec 19, 2016
 *      Author: bhshin
 */

#ifndef SCRATCH_BHSHIN_DELAY_MEASUREMENT_H_
#define SCRATCH_BHSHIN_DELAY_MEASUREMENT_H_

#include "ns3/nstime.h"

class DelayMeasurement {
private:
	std::string header;
	std::string type; // request, reply
	int seqNo;
	ns3::Time timeDifference;

public:
	DelayMeasurement();
	DelayMeasurement(std::string type, int seqNo, ns3::Time time);
	virtual ~DelayMeasurement();
	std::string serialize();
	void parse(std::string str);
	const std::string& getHeader() const;
	void setHeader(const std::string& header);
	const ns3::Time& getTimeDifference() const;
	void setTimeDifference(const ns3::Time& time);
	int getSeqNo() const;
	void setSeqNo(int seqNo);
	const std::string& getType() const;
	void setType(const std::string& type);
};



#endif /* SCRATCH_BHSHIN_DELAY_MEASUREMENT_H_ */
