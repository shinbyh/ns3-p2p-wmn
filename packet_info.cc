#include "packet_info.h"
#include "string_tokenizer.h"
#include "time_util.h"
#include "flow_type.h"

#include <boost/functional/hash.hpp>
#include <sstream>

PacketInfo::PacketInfo(std::string time, const Flow flow, int ifIdx, int bytes)
{
	init(getTimeMillisFromStr(time), flow, ifIdx, bytes);
}

PacketInfo::PacketInfo(long time, const Flow flow, int ifIdx, int bytes)
{
	init(time, flow, ifIdx, bytes);
}

PacketInfo::~PacketInfo() {
}

void PacketInfo::init(long time, const Flow flow, int ifIdx, int bytes) {
	this->time = time;
	this->flow = flow;
	this->ifIdx = ifIdx;
	this->bytes = bytes;
	this->tcpFlowStart = false;
	this->tcpPackets = 0;
	this->tcpAvgFlowRate = 0.0;
}

int PacketInfo::getBytes() const {
	return bytes;
}

void PacketInfo::setBytes(int bytes) {
	this->bytes = bytes;
}

const Flow PacketInfo::getFlow() const {
	return flow;
}

const int PacketInfo::getIfIdx() const {
	return ifIdx;
}

void PacketInfo::setIfIdx(const int ifIdx) {
	this->ifIdx = ifIdx;
}

long PacketInfo::getTime() const {
	return time;
}

double PacketInfo::getTcpAvgFlowRate() const {
	return tcpAvgFlowRate;
}

void PacketInfo::setTcpAvgFlowRate(double tcpAvgFlowRate) {
	this->tcpAvgFlowRate = tcpAvgFlowRate;
}

bool PacketInfo::isTcpFlowStart() const {
	return tcpFlowStart;
}

void PacketInfo::setTcpFlowStart(bool tcpFlowStart) {
	this->tcpFlowStart = tcpFlowStart;
}

int PacketInfo::getTcpPackets() const {
	return tcpPackets;
}

void PacketInfo::setTcpPackets(int tcpPackets) {
	this->tcpPackets = tcpPackets;
}

void PacketInfo::setTime(long time) {
	this->time = time;
}

void PacketInfo::parse(std::string line) {
	std::vector<std::string> tokens;

	tokenizeString(line, tokens, ";");

	for(size_t i=0; i<tokens.size(); i++){
		//tokens[i]
	}
}

int PacketInfo::hashCode() {
	boost::hash<std::string> string_hash;

	return string_hash(toString());
}

bool PacketInfo::operator ==(const PacketInfo &pktInfo) const {
	if(this->flow == (pktInfo.getFlow())
			&& this->ifIdx == pktInfo.getIfIdx()){
		return true;
	} else {
		return false;
	}

}

std::string PacketInfo::toString() {
	std::stringstream sstm;
	sstm << this->time << " "
			<< this->flow.toString() << " "
			<< this->ifIdx << " "
			<< this->bytes;
	return sstm.str();
}
