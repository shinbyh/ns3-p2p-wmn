/*
 * time_zone.h
 *
 *  Created on: Oct 28, 2016
 *      Author: bhshin
 */

#ifndef UTIL_TIME_ZONE_H_
#define UTIL_TIME_ZONE_H_

/*
 * UTC time offsets
 * (TODO: NOTE) Due to the variable type(short),
 * 30 or 15 minutes zones are not supported yet.
 */
#define TIMEZONE_UTC_MINUS_1200	12
#define TIMEZONE_UTC_MINUS_1100	11
#define TIMEZONE_UTC_MINUS_1000	10
#define TIMEZONE_UTC_MINUS_0900	9
#define TIMEZONE_UTC_MINUS_0800	8
#define TIMEZONE_UTC_MINUS_0700	7
#define TIMEZONE_UTC_MINUS_0600	6
#define TIMEZONE_UTC_MINUS_0500	5
#define TIMEZONE_UTC_MINUS_0400	4
#define TIMEZONE_UTC_MINUS_0300	3
#define TIMEZONE_UTC_MINUS_0200	2
#define TIMEZONE_UTC_MINUS_0100	1
#define TIMEZONE_UTC_0000		0
#define TIMEZONE_UTC_PLUS_0100	(-1)
#define TIMEZONE_UTC_PLUS_0200	(-2)
#define TIMEZONE_UTC_PLUS_0300	(-3)
#define TIMEZONE_UTC_PLUS_0400	(-4)
#define TIMEZONE_UTC_PLUS_0500	(-5)
#define TIMEZONE_UTC_PLUS_0600	(-6)
#define TIMEZONE_UTC_PLUS_0700	(-7)
#define TIMEZONE_UTC_PLUS_0800	(-8)
#define TIMEZONE_UTC_PLUS_0900	(-9)
#define TIMEZONE_UTC_PLUS_1000	(-10)
#define TIMEZONE_UTC_PLUS_1100	(-11)
#define TIMEZONE_UTC_PLUS_1200	(-12)
#define TIMEZONE_UTC_PLUS_1300	(-13)
#define TIMEZONE_UTC_PLUS_1400	(-14)

/*
 * Time offsets by countries
 * (NOTE) not all countries are added.
 */
#define TIMEZONE_KST		(-9)



#endif /* UTIL_TIME_ZONE_H_ */

