/*
 * route_path_quality_report.h
 *
 *  Created on: Jun 14, 2018
 *      Author: bhshin
 */

#ifndef SCRATCH_P2P_BHSHIN_ROUTE_PATH_QUALITY_REPORT_H_
#define SCRATCH_P2P_BHSHIN_ROUTE_PATH_QUALITY_REPORT_H_

#include "flow.h"
#include "qos_requirement.h"
#include "link_quality.h"

using namespace std;

class PathQualityReport {
private:
	Flow flow;
	int hopCount;
	QoSRequirement qosReq;
	LinkQuality endToEndQuality;

public:
	PathQualityReport(Flow flow, int hopCount);
	virtual ~PathQualityReport();

	string serialize();
	static PathQualityReport parse(string str);

	const LinkQuality& getEndToEndQuality() const;
	void setEndToEndQuality(const LinkQuality& endToEndQuality);
	const Flow& getFlow() const;
	void setFlow(const Flow& flow);
	int getHopCount() const;
	void setHopCount(int hopCount);
	const QoSRequirement& getQosReq() const;
	void setQosReq(const QoSRequirement& qosReq);
};

#endif /* SCRATCH_P2P_BHSHIN_ROUTE_PATH_QUALITY_REPORT_H_ */
