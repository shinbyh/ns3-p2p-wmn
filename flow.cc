#include "flow.h"

#include <boost/functional/hash.hpp>
#include <sstream>

Flow::Flow() {
	this->src = 0;
	this->dst = 0;
	this->srcPort = 0;
	this->dstPort = 0;
	this->type = FlowType::TCP;
}

Flow::Flow(uint32_t src, int srcPort, uint32_t dst, int dstPort, FlowType::Type flowType) {
	this->src = src;
	this->srcPort = srcPort;
	this->dst = dst;
	this->dstPort = dstPort;
	this->type = flowType;
}

const uint32_t Flow::getDst() const {
	return dst;
}

void Flow::setDst(const uint32_t dst) {
	this->dst = dst;
}

int Flow::getDstPort() const {
	return dstPort;
}

void Flow::setDstPort(int dstPort) {
	this->dstPort = dstPort;
}

const uint32_t Flow::getSrc() const {
	return src;
}

void Flow::setSrc(const uint32_t src) {
	this->src = src;
}

int Flow::getSrcPort() const {
	return srcPort;
}

void Flow::setSrcPort(int srcPort) {
	this->srcPort = srcPort;
}

FlowType::Type Flow::getType() const {
	return type;
}

std::string Flow::getTypeStr() const {
	return flowTypeStr(type);
}

void Flow::setType(FlowType::Type type) {
	this->type = type;
}

const std::string Flow::toString() const {
	std::stringstream sstm2;
	sstm2 << src << ":" << srcPort
			<< " --> "
			<< dst << ":" << dstPort
			<< ", " << type;

	return sstm2.str();
}

const std::string Flow::toFormattedString() const {
	std::stringstream sstm2;
	sstm2 << std::fixed;
	sstm2 << src << ":" << srcPort << "\t"
			<< dst << ":" << dstPort << "\t"
			<< type;

	return sstm2.str();
}

Flow::~Flow() {
}

bool Flow::operator ==(const Flow& ref) const {
	if(this->src == ref.getSrc()
			&& this->srcPort == ref.getSrcPort()
			&& this->dst == ref.getDst()
			&& this->dstPort == ref.getDstPort()
			&& this->type == ref.getType()){
		return true;
	} else {
		return false;
	}
}

bool Flow::operator <(const Flow& ref) const {
	return ((this->src + this->srcPort + this->dst + this->dstPort) < (ref.getSrc() + ref.getSrcPort() + ref.getDst() + ref.getDstPort()));
}

int Flow::hashCode() {
	boost::hash<std::string> string_hash;

	return string_hash(toString());
}
