#ifndef FLOW_FLOW_H_
#define FLOW_FLOW_H_

#include <iostream>

#include "ns3/ipv4-address.h"
#include "flow_type.h"

class Flow {

private:
	uint32_t src;
	int srcPort;
	uint32_t dst;
	int dstPort;
	FlowType::Type type;

public:
	Flow();
	Flow(uint32_t src, int srcPort, uint32_t dst, int dstPort, FlowType::Type flowType);
	~Flow();

	const uint32_t getDst() const;
	void setDst(const uint32_t dst);
	int getDstPort() const;
	void setDstPort(int dstPort);
	const uint32_t getSrc() const;
	void setSrc(const uint32_t src);
	int getSrcPort() const;
	void setSrcPort(int srcPort);
	FlowType::Type getType() const;
	std::string getTypeStr() const;
	void setType(FlowType::Type type);

	bool operator == (const Flow &ref) const;
	bool operator < (const Flow &ref) const;

	const std::string toString() const;
	const std::string toFormattedString() const;
	int hashCode();

};

#endif /* FLOW_FLOW_H_ */
