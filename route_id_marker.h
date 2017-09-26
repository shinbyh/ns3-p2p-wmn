/*
 * route_id_marker.h
 *
 *  Created on: Nov 1, 2016
 *      Author: bhshin
 */

#ifndef ROUTING_ROUTE_ID_MARKER_H_
#define ROUTING_ROUTE_ID_MARKER_H_

class RouteIDMarker {
private:
	int markID;
	RouteIDMarker();
	~RouteIDMarker();

public:
	static RouteIDMarker& instance(){
		static RouteIDMarker instance;
		return instance;
	}
	int getNextMarkID();
};

#endif /* ROUTING_ROUTE_ID_MARKER_H_ */
