/*
 * route_id_marker.cc
 *
 *  Created on: Nov 1, 2016
 *      Author: bhshin
 */

#include "route_id_marker.h"

RouteIDMarker::RouteIDMarker() {
	markID = 1;
}

RouteIDMarker::~RouteIDMarker() {
}

int RouteIDMarker::getNextMarkID() {
	return markID++;
}
