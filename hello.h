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
#include <stdint.h>

class Hello {
private:
	uint32_t nodeId;
	int seqNo;
	int numOfFlows;
	double occBW;
	double allocBW;
	double avgOccBW; // Scheme 1 (2017.11.16)
	bool isRouter;
	// std::string[] reachableIPs;
	std::vector<uint32_t> neighbors; // neighbor IDs, 2017.11.16

	std::string serializeNeighbors();
	void parseTrace(std::string str);
public:
	Hello();
	Hello(uint32_t nodeId, int seqNo, int numOfFlows, double occBW, double allocBW, bool isRouter);

	uint32_t getNodeId() const;
	void setNodeId(uint32_t nodeId);
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
	const std::vector<uint32_t>& getNeighbors() const;
	void setNeighbors(const std::vector<uint32_t>& neighbors);
	void addNeighbor(uint32_t nodeId);
};


#endif /* NETCTX_HELLO_H_ */
