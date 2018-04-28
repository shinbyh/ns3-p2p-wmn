/*
 * app_buffered_video_stream.cc
 *
 *  Created on: Apr 23, 2018
 *      Author: bhshin
 */

#include "app_buffered_video_stream.h"

AppBufferedVideoStream::AppBufferedVideoStream() {
	this->minBufSizeToPlay = 0;
	this->playbackBuffer = 0;
	this->totalGoodput = 0;
}

int AppBufferedVideoStream::getMinBufSizeToPlay() const {
	return minBufSizeToPlay;
}

void AppBufferedVideoStream::setMinBufSizeToPlay(int minBufSizeToPlay) {
	this->minBufSizeToPlay = minBufSizeToPlay;
}

int AppBufferedVideoStream::getPlaybackBuffer() const {
	return playbackBuffer;
}

void AppBufferedVideoStream::setPlaybackBuffer(int playbackBuffer) {
	this->playbackBuffer = playbackBuffer;
}

int AppBufferedVideoStream::getTotalGoodput() const {
	return totalGoodput;
}

void AppBufferedVideoStream::setTotalGoodput(int totalGoodput) {
	this->totalGoodput = totalGoodput;
}

AppBufferedVideoStream::~AppBufferedVideoStream() {
	// TODO Auto-generated destructor stub
}
