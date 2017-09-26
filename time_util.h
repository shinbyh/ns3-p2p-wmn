/*
 * time_util.h
 *
 *  Created on: Oct 28, 2016
 *      Author: bhshin
 */

#ifndef UTIL_TIME_UTIL_H_
#define UTIL_TIME_UTIL_H_

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/posix_time/posix_time_io.hpp>
#include <boost/date_time.hpp>
#include <string>
#include <boost/date_time/time_duration.hpp>
#include "time_zone.h"
#include "string_tokenizer.h"

using boost::posix_time::ptime;

/**
 * Convert a time string into the unix timestamp
 * with milliseconds.
 * The time format must be: "2016-10-27 19:31:44.531092".
 * Time format is: %Y-%m-%d %H:%M:%S%f .
 */
/*inline long getTimeMillisFromStr(std::string timeStr, short timezone){
	ptime pt2;
	{
		std::istringstream iss2(timeStr);
		auto* f2 = new boost::posix_time::time_input_facet("%Y-%m-%d %H:%M:%S%f");
		std::locale loc2(std::locale(""), f2);
		iss2.imbue(loc2);
		iss2 >> pt2;
	}

	return (pt2 - ptime{{1970,1,1},{}}).total_milliseconds() + (timezone*3600000);
}*/
inline long getTimeMillisFromStr(std::string timeStr){
	std::vector<std::string> tokens;
	tokenizeString(timeStr, tokens, " -:.");
	int year, month, day, hour, min, sec, msec;
	long sum;
	struct tm timeinfo;

	if(tokens.size() < 7){
		std::cout << " invalid time format! Use format like 2016-10-28 16:14:05.531092" << std::endl;
		return 0;
	}

	year = std::stoi(tokens[0]);
	month = std::stoi(tokens[1]);
	day = std::stoi(tokens[2]);
	hour = std::stoi(tokens[3]);
	min = std::stoi(tokens[4]);
	sec = std::stoi(tokens[5]);
	msec = std::stoi(tokens[6]);

	timeinfo.tm_year = year - 1900;
	timeinfo.tm_mon = month - 1;
	timeinfo.tm_mday = day;
	timeinfo.tm_hour = hour;
	timeinfo.tm_min = min;
	timeinfo.tm_sec = sec;
	sum = mktime(&timeinfo);
	sum = (sum * 1000) + (msec/1000);

	return sum;
}

/**
 * Return a current local time in the computer as
 * unix timestamp with milliseconds.
 */
inline long getCurrentTimeMillis(short timezone){
	ptime time_t_epoch(boost::gregorian::date(1970,1,1));
	ptime now = boost::posix_time::microsec_clock::local_time();
	boost::posix_time::time_duration diff = now - time_t_epoch;
	return diff.total_milliseconds() + (timezone*3600000);
}


#endif /* UTIL_TIME_UTIL_H_ */
