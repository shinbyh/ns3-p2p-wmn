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

#define EXISTING_WORK 0
#define PROPOSED_SCHEME 1

#define ROUTE_ARREQ 501
#define ROUTE_ARREP 502
#define ROUTE_ARERR 503
#define ROUTE_SETUP 504
#define PATH_PROBE	505

#define NODEID_NOT_FOUND 999999
#define NODEID_BROADCAST 255255

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
	bool proposedScheme;
	int flowUnsatisfactoryThreshold;

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
	bool isProposedScheme() const;
	std::vector<std::string> getIfaces();
	int getPortByName(std::string name);
	std::vector<int> getControlPorts();
	std::vector<std::string> getSubnets();
	std::string getBroadcastIP(std::string iface);
	int getFlowUnsatisThreshold();
};


#endif /* CONFIG_H_ */
