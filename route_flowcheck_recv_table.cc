/*
 * flowcheck_recv_table.cc
 *
 *  Created on: Jan 16, 2018
 *      Author: bhshin
 */

#include "route_flowcheck_recv_table.h"
#include "my_config.h"
#include <boost/foreach.hpp>
#include <algorithm>
#include "ns3/core-module.h"

FlowCheckRecvTable::FlowCheckRecvTable(int seqNo) {
	this->nextHopToSrc = NODEID_NOT_FOUND;
	this->prevNextHop = NODEID_NOT_FOUND;
	this->flowSeqNo = seqNo;
}

FlowCheckRecvTable::~FlowCheckRecvTable() {
	map<uint32_t, FlowCheck*>::iterator itr;
	for(itr = table.begin(); itr != table.end(); itr++){
		delete itr->second;
	}
}

void FlowCheckRecvTable::addFlowCheckReply(uint32_t nodeId, FlowCheck* flowCheck) {
	if(flowCheck->getMsgType() == ROUTE_FLOWCHECK_REPLY){
		this->table[nodeId] = flowCheck;
	}
}

const LinkQuality& FlowCheckRecvTable::getEndToEndQuality() const {
	return endToEndQuality;
}

void FlowCheckRecvTable::setEndToEndQuality(
		const LinkQuality& endToEndQuality) {
	this->endToEndQuality = endToEndQuality;
}

const Flow& FlowCheckRecvTable::getFlow() const {
	return flow;
}

void FlowCheckRecvTable::setFlow(const Flow& flow) {
	this->flow = flow;
}

uint32_t FlowCheckRecvTable::getNextHopToSrc() const {
	return nextHopToSrc;
}

void FlowCheckRecvTable::setNextHopToSrc(uint32_t nextHopToSrc) {
	this->nextHopToSrc = nextHopToSrc;
}

uint32_t FlowCheckRecvTable::getPrevNextHop() const {
	return prevNextHop;
}

void FlowCheckRecvTable::setPrevNextHop(uint32_t prevNextHop) {
	this->prevNextHop = prevNextHop;
}

const QoSRequirement& FlowCheckRecvTable::getQosReq() const {
	return qosReq;
}

void FlowCheckRecvTable::setQosReq(const QoSRequirement& qosReq) {
	this->qosReq = qosReq;
}

bool FlowCheckRecvTable::containsNodeId(uint32_t nodeId) {
	if(find(this->srcRoute.begin(), this->srcRoute.end(), nodeId) != this->srcRoute.end()){
		return true;
	} else {
		return false;
	}
}

uint32_t FlowCheckRecvTable::getOptimalDetourNode(int hopCount) {
	// debug
	stringstream ss;
	for(uint32_t srcId : this->srcRoute){
		ss << srcId << ",";
	}
	NS_LOG_UNCOND(" -srcRoute = " << ss.str());
	NS_LOG_UNCOND(" -flow = " << this->flow.toString());
	NS_LOG_UNCOND(" -prevNextHop = " << this->prevNextHop);

	if(table.size() == 0){
		return NODEID_NOT_FOUND;
	}

	if(hopCount == 1){
		uint32_t optimalNode = NODEID_NOT_FOUND;
		//double occupiedBW = 99999999.0;
		double availableBW = 0.0; // 180826
		std::pair<uint32_t, FlowCheck*> p;
		BOOST_FOREACH(p, table){
			// Discard the information of the previous nextHop.
			if(p.first == this->prevNextHop) continue;

			// Discard the neighbor node which is already in the source route trace.
			if(containsNodeId(p.first)) continue;

			FlowCheck* fc = p.second;
			if(fc->getStats().size() == 0){
				// The link between candidate and prevNextHop is empty.
				optimalNode = p.first;
			} else {
				if(availableBW < fc->getAvgAvailableBw()){
					availableBW = fc->getAvgAvailableBw();
					optimalNode = p.first;
				}

				/*for(SourceRouteStat stat : fc->getStats()){
					int prevNextHopIdx = stat.getNodeIdPosition(this->prevNextHop);
					vector<uint32_t> trace = stat.getTrace();

					NS_LOG_UNCOND("  (candidate: " << p.first << ")");
					NS_LOG_UNCOND("    - prevNextHopIdx: " << prevNextHopIdx);
					stringstream ss2;
					int tempCount = 0;
					for(uint32_t item : trace){
						if(tempCount == prevNextHopIdx){
							ss2 << "[" << item << "],";
						} else {
							ss2 << item << ",";
						}
						tempCount++;
					}
					NS_LOG_UNCOND("    - trace: " << ss2.str());

					if(prevNextHopIdx < 0){
						// prevNextHop not found in the trace.
					} else if(prevNextHopIdx == 0){
						if(trace[1] == p.first){
							// candidate
							if(stat.getBandwidth() < occupiedBW){
								optimalNode = p.first;
								occupiedBW = stat.getBandwidth();
							}
						}
					} else if(prevNextHopIdx == (int)trace.size()-1){
						if(trace[prevNextHopIdx-1] == p.first){
							// candidate
							if(stat.getBandwidth() < occupiedBW){
								optimalNode = p.first;
								occupiedBW = stat.getBandwidth();
							}
						}
					} else {
						if(trace[prevNextHopIdx-1] == p.first || trace[prevNextHopIdx+1] == p.first){
							// candidate
							if(stat.getBandwidth() < occupiedBW){
								optimalNode = p.first;
								occupiedBW = stat.getBandwidth();
							}
						}
					}
				}*/
			}
		}

		return optimalNode;
	} else {
		uint32_t optimalNode = NODEID_NOT_FOUND;
		double occupiedBW = 99999999.0;
		std::pair<uint32_t, FlowCheck*> p;
		BOOST_FOREACH(p, table){
			// Discard the information of the previous nextHop.
			if(p.first == this->prevNextHop) continue;

			// Discard the neighbor node which is already in the source route trace.
			if(containsNodeId(p.first)) continue;

			FlowCheck* fc = p.second;
			for(SourceRouteStat stat : fc->getStats()){
				int prevNextHopIdx = stat.getNodeIdPosition(this->prevNextHop);
				vector<uint32_t> trace = stat.getTrace();

				NS_LOG_UNCOND("  (candidate: " << p.first << ")");
				NS_LOG_UNCOND("    - prevNextHopIdx: " << prevNextHopIdx);
				stringstream ss2;
				int tempCount = 0;
				for(uint32_t item : trace){
					if(tempCount == prevNextHopIdx){
						ss2 << "[" << item << "],";
					} else {
						ss2 << item << ",";
					}
					tempCount++;
				}
				NS_LOG_UNCOND("    - trace: " << ss2.str());

				if(prevNextHopIdx < 0){
					// prevNextHop not found in the trace.
				} else if(prevNextHopIdx == 0){
					if(trace[1] == p.first){
						// candidate
						if(stat.getBandwidth() < occupiedBW){
							optimalNode = p.first;
							occupiedBW = stat.getBandwidth();
						}
					}
				} else if(prevNextHopIdx == (int)trace.size()-1){
					if(trace[prevNextHopIdx-1] == p.first){
						// candidate
						if(stat.getBandwidth() < occupiedBW){
							optimalNode = p.first;
							occupiedBW = stat.getBandwidth();
						}
					}
				} else {
					if(trace[prevNextHopIdx-1] == p.first || trace[prevNextHopIdx+1] == p.first){
						// candidate
						if(stat.getBandwidth() < occupiedBW){
							optimalNode = p.first;
							occupiedBW = stat.getBandwidth();
						}
					}
				}
			}
		}

		return optimalNode;
	}
}

int FlowCheckRecvTable::getFlowSeqNo() const {
	return flowSeqNo;
}

void FlowCheckRecvTable::setFlowSeqNo(int flowSeqNo) {
	this->flowSeqNo = flowSeqNo;
}
