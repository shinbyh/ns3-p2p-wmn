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

class Hello {
private:
	int nodeId;
	int seqNo;
	int numOfFlows;
	double occBW;
	double allocBW;
	bool isRouter;
	// std::string[] reachableIPs;

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
};


#endif /* NETCTX_HELLO_H_ */
