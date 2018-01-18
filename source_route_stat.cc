/*
 * source_route_trace.cc
 *
 *  Created on: Jan 16, 2018
 *      Author: bhshin
 */

#include "source_route_stat.h"
#include <sstream>
#include "string_tokenizer.h"
#include <cstdlib> // strtod
#include <boost/format.hpp> // format()

namespace std {

SourceRouteStat::SourceRouteStat() {
	this->bandwidth = 0.0;
}

SourceRouteStat::~SourceRouteStat() {
}

void SourceRouteStat::addTrace(uint32_t nodeId) {
	this->trace.push_back(nodeId);
}

const string SourceRouteStat::serialize() {
	stringstream ss;
	for(size_t i=0; i<this->trace.size(); i++){
		if(i + 1 < this->trace.size())
			ss << this->trace[i] << ",";
		else
			ss << this->trace[i];
	}
	ss << boost::format("/%.2f") % this->bandwidth;

	return ss.str();
}

void SourceRouteStat::parseTrace(string str){
	vector<string> tokens;
	tokenizeString(str, tokens, ",");
	for(string nodeIdStr : tokens){
		addTrace(atoi(nodeIdStr.c_str()));
	}
}

void SourceRouteStat::parse(string str) {
	vector<string> tokens;
	tokenizeString(str, tokens, "/");
	parseTrace(tokens[0]);
	this->bandwidth = strtod(tokens[1].c_str(), NULL);
}

} /* namespace std */


