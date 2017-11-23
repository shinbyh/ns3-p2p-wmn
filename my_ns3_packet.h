/*
 * my_ns3_packet.h
 *
 *  Created on: Jan 11, 2017
 *      Author: bhshin
 */

#ifndef SCRATCH_BHSHIN_MY_NS3_PACKET_H_
#define SCRATCH_BHSHIN_MY_NS3_PACKET_H_

#include "ns3/object.h"

class MyNS3Packet : public ns3::Object
{
private:
	uint32_t src;
	int appSrcPort;
	uint32_t dst;
	int appDstPort;
	std::string msg;

public:
	MyNS3Packet(uint32_t src, int appSrcPort, uint32_t dst, int appDstPort, std::string msg);
	MyNS3Packet();
	virtual ~MyNS3Packet();
	const uint32_t getSrc() const;
	void setSrc(uint32_t src);
	const uint32_t getDst() const;
	void setDst(uint32_t dst);
	const std::string getMsg() const;
	void setMsg(const std::string msg);
	const std::string serialize();
	static ns3::Ptr<MyNS3Packet> parse(std::string str);
	const int getAppDstPort() const;
	void setAppDstPort(int appDstPort);
	const int getAppSrcPort() const;
	void setAppSrcPort(int appSrcPort);
	const int getPktSize() const;
};



#endif /* SCRATCH_BHSHIN_MY_NS3_PACKET_H_ */
