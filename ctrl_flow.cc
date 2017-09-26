/*
 * ctrl_flow.cc
 *
 *  Created on: Feb 2, 2017
 *      Author: bhshin
 */

#include "ctrl_flow.h"

ControlFlow::ControlFlow(int port) {
	this->port = port;
	this->totalRecvBytes = 0;
	this->totalRecvPackets = 0;
	this->totalTxBytes = 0;
	this->totalTxPackets = 0;
	this->realTimeBandwidth = 0;
}

int ControlFlow::getPort() const {
	return port;
}

void ControlFlow::setPort(int port) {
	this->port = port;
}

int ControlFlow::getRealTimeBandwidth() const {
	return realTimeBandwidth;
}

void ControlFlow::setRealTimeBandwidth(int realTimeBandwidth) {
	this->realTimeBandwidth = realTimeBandwidth;
}

int ControlFlow::getTotalRecvBytes() const {
	return totalRecvBytes;
}

void ControlFlow::setTotalRecvBytes(int totalRecvBytes) {
	this->totalRecvBytes = totalRecvBytes;
}

int ControlFlow::getTotalRecvPackets() const {
	return totalRecvPackets;
}

void ControlFlow::setTotalRecvPackets(int totalRecvPackets) {
	this->totalRecvPackets = totalRecvPackets;
}

int ControlFlow::getTotalTxBytes() const {
	return totalTxBytes;
}

void ControlFlow::setTotalTxBytes(int totalTxBytes) {
	this->totalTxBytes = totalTxBytes;
}

int ControlFlow::getTotalTxPackets() const {
	return totalTxPackets;
}

void ControlFlow::setTotalTxPackets(int totalTxPackets) {
	this->totalTxPackets = totalTxPackets;
}

ControlFlow::~ControlFlow() {
}

void ControlFlow::addPacketInfo(bool isTx, PacketInfo* pktInfo) {
	if(isTx){
		this->totalTxBytes += pktInfo->getBytes();
		this->totalTxPackets++;
	} else {
		this->totalRecvBytes += pktInfo->getBytes();
		this->totalRecvPackets++;
	}
	this->realTimeBandwidth += pktInfo->getBytes() * 8;
}

void ControlFlow::resetRealTimeBandwidth() {
	this->realTimeBandwidth = 0;
}
