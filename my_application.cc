/*
 * my_application.cc
 *
 *  Created on: Apr 24, 2018
 *      Author: bhshin
 */

#include "my_application.h"
#include "my_config.h"
#include "my_statistics.h"
#include <sstream>

MyApplication::MyApplication(std::string name, FlowRequest flowReq) {
	this->name = name;
	this->totalNumberOfPkts = 0;
	this->totalRecvBytes = 0;
	this->tempNumberOfPkts = 0;
	this->tempRecvBytes = 0;
	this->flowReq = flowReq;
	this->dataRateBytes = flowReq.getPktSize() * flowReq.getSendingRate();
	this->videoBuffer = 0;
	this->minBufSizeToPlay = this->dataRateBytes * atoi(MyConfig::instance().getValue("AppMinimumBufferingTime").c_str());
	this->buffering = true;
	this->pktConsumptionStarted = false;

	stringstream ss;
	ss << "applog_" << flowReq.getFlow().getDst() << "-" << flowReq.getFlow().getDstPort() << ".txt";
	this->appDataOut.open(ss.str().c_str(), ofstream::out);
}

MyApplication::~MyApplication() {
	this->appDataOut.close();
}

int MyApplication::getDataRate() const {
	return dataRateBytes;
}

void MyApplication::setDataRate(int dataRateBytes) {
	this->dataRateBytes = dataRateBytes;
}

const FlowRequest& MyApplication::getFlowReq() const {
	return flowReq;
}

void MyApplication::setFlowReq(const FlowRequest& flowReq) {
	this->flowReq = flowReq;

	// Apply dataRate based on the FlowRequest.
	this->dataRateBytes = this->flowReq.getPktSize() * this->flowReq.getSendingRate();
}

const std::string& MyApplication::getName() const {
	return name;
}

const Time& MyApplication::getLastUpdateTime() const {
	return lastUpdateTime;
}

void MyApplication::setLastUpdateTime(const Time& lastUpdateTime) {
	this->lastUpdateTime = lastUpdateTime;
}

int MyApplication::getTotalNumberOfPkts() const {
	return totalNumberOfPkts;
}

void MyApplication::setTotalNumberOfPkts(int totalNumberOfPkts) {
	this->totalNumberOfPkts = totalNumberOfPkts;
}

int MyApplication::getTotalRecvBytes() const {
	return totalRecvBytes;
}

int MyApplication::getDataRateBytes() const {
	return dataRateBytes;
}

void MyApplication::setDataRateBytes(int dataRateBytes) {
	this->dataRateBytes = dataRateBytes;
}

int MyApplication::getMinBufSizeToPlay() const {
	return minBufSizeToPlay;
}

void MyApplication::setMinBufSizeToPlay(int minBufSizeToPlay) {
	this->minBufSizeToPlay = minBufSizeToPlay;
}

int MyApplication::getVideoBuffer() const {
	return videoBuffer;
}

bool MyApplication::isBuffering() const {
	return buffering;
}

void MyApplication::setBuffering(bool buffering) {
	this->buffering = buffering;
}

void MyApplication::setVideoBuffer(int videoBuffer) {
	this->videoBuffer = videoBuffer;
}

void MyApplication::setTotalRecvBytes(int totalRecvBytes) {
	this->totalRecvBytes = totalRecvBytes;
}

void MyApplication::setName(const std::string& name) {
	this->name = name;
}

void MyApplication::handleApplicationPacket(Ptr<MyNS3Packet> myPkt) {
	//NS_LOG_UNCOND(" [handleApplicationPacket] ");
	this->totalNumberOfPkts++;
	this->totalRecvBytes += myPkt->getDataSize();
	this->tempNumberOfPkts++;
	this->tempRecvBytes += myPkt->getDataSize();
	this->videoBuffer += myPkt->getDataSize();

	// The first triggering point of packet consumption.
	if(this->buffering && !this->pktConsumptionStarted){
		if(this->videoBuffer >= this->minBufSizeToPlay){
			this->buffering = false;
			this->pktConsumptionStarted = true;
			_consumeDataPackets();
		}
	}
}

void MyApplication::updateStatistics() {
	// Debug
	NS_LOG_UNCOND(" [updateStatistics]");
	NS_LOG_UNCOND("   - flow: " << this->flowReq.getFlow().toString());
	NS_LOG_UNCOND("   - numOfPkts = " << this->tempNumberOfPkts);
	NS_LOG_UNCOND("   - recvBytes = " << this->tempRecvBytes);
	NS_LOG_UNCOND("   - totalNumOfPkts = " << this->totalNumberOfPkts);
	NS_LOG_UNCOND("   - videoBuffer = " << this->videoBuffer);
	NS_LOG_UNCOND("   - buffering? " << this->buffering);

	// Reset temporary counter variables.
	this->tempNumberOfPkts = 0;
	this->tempRecvBytes = 0;
	this->lastUpdateTime = Simulator::Now();
}

void MyApplication::_consumeDataPackets() {
	int consumedData = 0;

	if(!this->buffering){
		if(this->videoBuffer > this->dataRateBytes){
			consumedData = this->dataRateBytes;
			this->videoBuffer -= this->dataRateBytes;
		} else {
			// If buffer is empty, wait for the buffer
			// to be filled with a minimum playback size.
			consumedData = this->dataRateBytes - this->videoBuffer;
			this->videoBuffer = 0;
			this->buffering = true;
		}
	}

	// Debug
	//NS_LOG_UNCOND(" [consumeDataPackets] " << consumedData << " bytes!! (buf = " << this->videoBuffer << ")");

	// Write stat of consumed data.
	this->appDataOut << (int)(Simulator::Now().GetSeconds()) << "\t" << consumedData << "\n";
	MyStatistics::instance().addGoodput(this->flowReq.getFlow(), (double)consumedData);

	// Schedule the next data consumption.
	if(Simulator::Now() < this->flowReq.getEndTime() + Seconds(atoi(MyConfig::instance().getValue("AppMinimumBufferingTime").c_str()))){
		Time nextTime = Seconds(1.0);
		Simulator::Schedule(nextTime, &MyApplication::consumeDataPackets, this);
	}
}

void MyApplication::consumeDataPackets(MyApplication* myApp) {
	myApp->_consumeDataPackets();
}
