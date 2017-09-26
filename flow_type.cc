/*
 * flow_type.cc
 *
 *  Created on: Oct 27, 2016
 *      Author: bhshin
 */

#include "flow_type.h"

std::string typeStrs[] = {
	"UDP",
	"TCP",
	"ICMP",
	"IGMP",
	"BOOTP_DHCP",
	"UNKNOWN",
	"ANY_TYPE"
};

FlowType::Type checkType(std::string str){
	for(int i=0; i<FlowType::Tail; i++){
		if(typeStrs[i] == str) return (FlowType::Type)i;
	}
	return FlowType::UNKNOWN;
}

std::string flowTypeStr(FlowType::Type type){
	return typeStrs[type];
}


