/*
 * hello.h
 *
 *  Created on: Dec 7, 2016
 *      Author: bhshin
 */

#ifndef NETCTX_HELLO_H_
#define NETCTX_HELLO_H_

#include <string>
#include <sstream>
#include <vector>

class Hello {
private:
	int nodeId;
	int seqNo;
	int numOfFlows;
	double occBW;
	double allocBW;
	double avgOccBW; // Scheme 1 (2017.11.16)
	bool isRouter;
	// std::string[] reachableIPs;
	std::vector<int> neighbors; // neighbor IDs, 2017.11.16

	std::string serializeNeighbors();
	void parseTrace(std::string str);
public:
	Hello();
	Hello(int nodeId, int seqNo, int numOfFlows, double occBW, double allocBW, bool isRouter);

	int getNodeId() const;
	void setNodeId(int nodeId);
	double getAllocBw() const;
	void setAllocBw(double allocBw);
	bool isIsRouter() const;
	void setIsRouter(bool isRouter);
	int getNumOfFlows() const;
	void setNumOfFlows(int numOfFlows);
	double getOccBw() const;
	void setOccBw(double occBw);
	int getSeqNo() const;
	void setSeqNo(int seqNo);
	const std::string serialize();
	void parse(std::string line);
	double getAvgOccBw() const;
	void setAvgOccBw(double avgOccBw);
	const std::vector<int>& getNeighbors() const;
	void setNeighbors(const std::vector<int>& neighbors);
	void addNeighbor(int nodeId);
};


#endif /* NETCTX_HELLO_H_ */
