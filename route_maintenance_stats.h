/*
 * route_maintenance_stats.h
 *
 *  Created on: Jun 25, 2018
 *      Author: bhshin
 */

#ifndef SCRATCH_P2P_BHSHIN_ROUTE_MAINTENANCE_STATS_H_
#define SCRATCH_P2P_BHSHIN_ROUTE_MAINTENANCE_STATS_H_

#include "ns3/core-module.h"
#include "flow.h"
#include <map>

using namespace std;

class QoSViolationStat {
private:
	ns3::Time qvStart;
	ns3::Time qvEnd;
	uint32_t qvFoundNode;
	bool resolved; // flag whether QoS violation is resolved or not

public:
	QoSViolationStat();
	virtual ~QoSViolationStat();
	const ns3::Time& getQvEnd() const;
	void setQvEnd(const ns3::Time& qvEnd);
	uint32_t getQvFoundNode() const;
	void setQvFoundNode(uint32_t qvFoundNode);
	const ns3::Time& getQvStart() const;
	void setQvStart(const ns3::Time& qvStart);
	ns3::Time getRtMtncDuration();
	bool isResolved() const;
	void setResolved(bool resolved);
};

class RouteMaintenanceStat {
private:
	map<int, QoSViolationStat*> qvMap; // seqNo and stat mapping

public:
	RouteMaintenanceStat();
	virtual ~RouteMaintenanceStat();
	void addQVStartTime(int seqNo, ns3::Time qvStart);
	void addQVEndTime(int seqNo, ns3::Time qvEnd);
	void setQVFoundNodeId(int seqNo, uint32_t nodeId);
	void setQVResolved(int seqNo, bool resolved);
	QoSViolationStat* getQVStat(int seqNo);
	string generateFormattedOut(string header);
	static string generateHeaderOut();
};

class RouteMaintenanceStats {
private:
	map<Flow, RouteMaintenanceStat*> rtMtncMap;

public:
	RouteMaintenanceStats();
	virtual ~RouteMaintenanceStats();
	void addQVStartTime(Flow flow, int seqNo, ns3::Time qvStart);
	void addQVEndTime(Flow flow, int seqNo, ns3::Time qvEnd);
	void setQVFoundNodeId(Flow flow, int seqNo, uint32_t nodeId);
	void setQVResolved(Flow flow, int seqNo, bool resolved);
	void writeStatsToFile(string fileName);
};

#endif /* SCRATCH_P2P_BHSHIN_ROUTE_MAINTENANCE_STATS_H_ */
