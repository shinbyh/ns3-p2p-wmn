/*
 * route_src_rt_update.h
 *
 *  Created on: Dec 4, 2017
 *      Author: bhshin
 */

#ifndef SCRATCH_P2P_BHSHIN_ROUTE_SRC_RT_UPDATE_H_
#define SCRATCH_P2P_BHSHIN_ROUTE_SRC_RT_UPDATE_H_

#include "flow.h"
#include <vector>

class SourceRouteUpdate {
private:
	Flow flow;
	std::vector<uint32_t> srcRoute;
	std::string serializeTrace();
	void parseTrace(std::string str);

public:
	SourceRouteUpdate();
	SourceRouteUpdate(Flow flow);
	SourceRouteUpdate(Flow flow, const std::vector<uint32_t>& srcRoute);
	virtual ~SourceRouteUpdate();

	std::string serialize();
	void parse(std::string str);
	const Flow& getFlow() const;
	void setFlow(const Flow& flow);
	const std::vector<uint32_t>& getSrcRoute() const;
	void setSrcRoute(const std::vector<uint32_t>& srcRoute);
};

#endif /* SCRATCH_P2P_BHSHIN_ROUTE_SRC_RT_UPDATE_H_ */
