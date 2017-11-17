/*
 * config.cc
 *
 *  Created on: Oct 27, 2016
 *      Author: bhshin
 */

#include "my_config.h"

#include "string_tokenizer.h"
#include <fstream>
#include <boost/foreach.hpp>
#include <iostream> // for debug "cout"

MyConfig::MyConfig() {
	init();
}

MyConfig::~MyConfig() {

}

void MyConfig::init() {
	this->meshRouter = false;
	//this->proposedScheme = false;
	this->scheme = 0;
	this->flowUnsatisfactoryThreshold = 0;
	this->maxBandwidth = 2000000.0; // 2Mbps
}

std::pair<std::string, std::string> MyConfig::parsePairFromStr(std::string str, std::string delim) {
	std::pair<std::string, std::string> p;
	std::vector<std::string> tokens;
	tokenizeString(str, tokens, delim);

	if(tokens.size() == 2){
		//cout << "key " << tokens[0] << ", value " << tokens[1] << "\n";

		p.first = tokens[0];
		p.second = tokens[1];
	} else {
		// error: parsing failed.
		p.first = "null";
		p.second = "null";
	}
	return p;
}

void MyConfig::readConfigFromFile(std::string filepath) {
	std::cout << "[MyConfig] reading config file: " << filepath << std::endl;

	std::ifstream inStream(filepath, std::ios::in);
	std::string temp;

	while(!inStream.eof()){
		getline(inStream, temp);
		ltrim(temp);
		rtrim(temp);

		// Skip comments
		if(temp.substr(0, 1) == "#" || temp.substr(0, 2) == "//"){
			continue;
		}
		// skip blank lines
		if(temp == "" || temp.size() == 0){
			continue;
		}

		// debug
		//cout << "[MyConfig] " << temp << "\n";

		// read each line of the config file and add to confMap
		std::pair<std::string, std::string> pConf = parsePairFromStr(temp, " =");
		if(pConf.first != "null"){
			confMap[pConf.first] = pConf.second;

			// additional mapping for interface mapping
			if(pConf.first == "InterfaceMapping"){
				std::pair<std::string, std::string> pIf = parsePairFromStr(pConf.second, "/");
				if(pIf.first != "null"){
					ifToSubnetMap[pIf.first] = pIf.second;
					subnetToIfMap[pIf.second] = pIf.first;
				}
			}

			// additional mapping for port mapping
			size_t found = pConf.first.find("Port");
			if(found != std::string::npos){
				controlPortMap[pConf.first] = atoi(pConf.second.c_str());
			}

			// additional mapping for MeshSubnet
			if(pConf.first == "MeshSubnet"){
				meshSubnets.push_back(pConf.second);
			}

			// additional mapping for HostapdSubnet
			if(pConf.first == "HostapdSubnet"){
				hostapdSubnets.push_back(pConf.second);
			}

			// additional setting for isMeshRouter
			if(pConf.first == "IsMeshRouter"){
				if(pConf.second == "yes" || pConf.second == "true"){
					this->meshRouter = true;
				} else {
					this->meshRouter = false;
				}
			}

			// additional setting for selecting local repair schemes
			if(pConf.first == "Scheme"){
				this->scheme = atoi(pConf.second.c_str());
			}

			// additional mapping for FlowUnsatisfactoryThreshold
			if(pConf.first == "FlowUnsatisfactoryThreshold"){
				this->flowUnsatisfactoryThreshold = atoi(pConf.second.c_str());
			}

			// additional mapping for maximum bandwidth
			if(pConf.first == "DefaultMaxBandwidth"){
				this->maxBandwidth = atof(pConf.second.c_str());
			}
		}
	}


	// after loading ifToSubnet mappings
	this->ifaces = getKeySetFrom(this->ifToSubnetMap);
}

std::string MyConfig::getValue(std::string key) {
	return this->confMap[key];
}

std::vector<std::string> MyConfig::getKeySetFrom(std::map<std::string, std::string> m) {
	std::pair<std::string, std::string> p;
	std::vector<std::string> keys;

	BOOST_FOREACH (p, m) {
		keys.push_back(p.first);
	}
	return keys;
}

template <typename T>
std::vector<T> MyConfig::getValueSetFrom(std::map<std::string, T> m) {
	std::pair<std::string, T> p;
	std::vector<T> values;

	BOOST_FOREACH (p, m) {
		values.push_back(p.second);
	}
	return values;
}

bool MyConfig::isMeshRouter() const {
	return meshRouter;
}

int MyConfig::getScheme() const {
	return scheme;
}

std::vector<std::string> MyConfig::getIfaces() {
	return this->ifaces;
}

int MyConfig::getPortByName(std::string name){
	return controlPortMap[name];
}

std::vector<int> MyConfig::getControlPorts() {
	return getValueSetFrom(controlPortMap);
}

std::vector<std::string> MyConfig::getSubnets() {
	return getValueSetFrom(ifToSubnetMap);
}

std::string MyConfig::getBroadcastIP(std::string iface) {
	return ifToSubnetMap[iface] + "255";
}

int MyConfig::getFlowUnsatisThreshold() {
	return flowUnsatisfactoryThreshold;
}

double MyConfig::getMaxBandwidth() {
	return maxBandwidth;
}
