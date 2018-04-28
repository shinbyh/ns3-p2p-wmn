/*
 * app_buffered_video_stream.h
 *
 *  Created on: Apr 23, 2018
 *      Author: bhshin
 */

#ifndef SCRATCH_P2P_BHSHIN_APP_BUFFERED_VIDEO_STREAM_H_
#define SCRATCH_P2P_BHSHIN_APP_BUFFERED_VIDEO_STREAM_H_

class AppBufferedVideoStream {
	int playbackBuffer; // current playback buffer (bytes)
	int totalGoodput; // total goodput (bps)
	int minBufSizeToPlay; // minimum buffer size to play (bytes)

public:
	AppBufferedVideoStream();
	virtual ~AppBufferedVideoStream();

	int getMinBufSizeToPlay() const;
	void setMinBufSizeToPlay(int minBufSizeToPlay);
	int getPlaybackBuffer() const;
	void setPlaybackBuffer(int playbackBuffer);
	int getTotalGoodput() const;
	void setTotalGoodput(int totalGoodput);
};

#endif /* SCRATCH_P2P_BHSHIN_APP_BUFFERED_VIDEO_STREAM_H_ */
