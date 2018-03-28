/*
 * config.h
 *
 *  Created on: Oct 27, 2016
 *      Author: bhshin
 */

#ifndef CONFIG_H_
#define CONFIG_H_

#include <map>
#include <vector>
#include <algorithm>  // for string trim
#include <functional> // for string trim
#include <cctype> // for string trim
#include <locale> // for string trim

//#define EXISTING_WORK 0
//#define PROPOSED_SCHEME 1
#define BASELINE 0
#define SCHEME_PATIAL_DISCOVERY 1
#define SCHEME_LOCAL_REPAIR_1HOP 2
#define SCHEME_LOCAL_REPAIR_2HOPS 3
#define SCHEME_LOCAL_REPAIR_AVG_BW 4 // not used for now (180221)

#define ROUTE_ARREQ 501
#define ROUTE_ARREP 502
#define ROUTE_ARERR 503
#define ROUTE_SETUP 504
#define PATH_PROBE	505
#define ROUTE_SRC_ROUTE_UPDATE 506
#define ROUTE_FLOWCHECK_REQUEST 507
#define ROUTE_FLOWCHECK_REPLY 508
#define ROUTE_FLOW_ACCEPT_REQUEST 509
#define ROUTE_FLOW_ACCEPT_REPLY 510
#define ROUTE_LOCAL_REPAIR_REQUEST 511

#define NODEID_NOT_FOUND 999999
#define NODEID_BROADCAST 255255

#define CIRCULAR_BUFFER_SIZE_ETX 10
#define CIRCULAR_BUFFER_SIZE_NC 10
#define CIRCULAR_BUFFER_SIZE_FLOWSTAT 3

class MyConfig {
private:
	std::map<std::string, int> controlPortMap;
	std::map<std::string, std::string> ifToSubnetMap;
	std::map<std::string, std::string> subnetToIfMap;
	std::map<std::string, std::string> confMap;
	std::vector<std::string> ifaces;
	std::vector<std::string> meshSubnets;
	std::vector<std::string> hostapdSubnets;
	bool meshRouter;
	int scheme;
	int flowUnsatisfactoryThreshold;
	double maxBandwidth;

	MyConfig();
	MyConfig(const MyConfig& other); // prevent construction by copying
	~MyConfig();
	void init();
	std::vector<std::string> getKeySetFrom(std::map<std::string, std::string> m);
	template <typename T>
	std::vector<T> getValueSetFrom(std::map<std::string, T> m);
	std::pair<std::string, std::string> parsePairFromStr(std::string str, std::string delim);

public:
	static MyConfig& instance(){
		static MyConfig instance;
		return instance;
	}
	void readConfigFromFile(std::string filepath);
	std::string getValue(std::string key);
	bool isMeshRouter() const;
	int getScheme() const;
	std::vector<std::string> getIfaces();
	int getPortByName(std::string name);
	std::vector<int> getControlPorts();
	std::vector<std::string> getSubnets();
	std::string getBroadcastIP(std::string iface);
	int getFlowUnsatisThreshold();
	double getMaxBandwidth();
};


#endif /* CONFIG_H_ */
