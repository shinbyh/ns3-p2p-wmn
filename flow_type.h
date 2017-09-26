/*
 * flow_type.h
 *
 *  Created on: Oct 19, 2016
 *      Author: bhshin
 */

#ifndef SRC_FLOW_TYPE_H_
#define SRC_FLOW_TYPE_H_

#include <string>

namespace FlowType
{
	enum Type
	{
		Head = 0,
		UDP = 0,
		TCP,
		ICMP,
		IGMP,
		BOOTP_DHCP,
		UNKNOWN,
		ANY_TYPE, // This must be at the last position of Type.
		Tail = ANY_TYPE // This must be at the last position of Type.
	};
}

extern std::string typeStrs[];

FlowType::Type checkType(std::string str);
std::string flowTypeStr(FlowType::Type type);

#endif /* SRC_FLOW_TYPE_H_ */
