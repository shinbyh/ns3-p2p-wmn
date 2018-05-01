#!/bin/bash

BASE_DIR=$(pwd)
EXP_NAME=$1
APPS="10apps 11apps 12apps 13apps 14apps 15apps 16apps 17apps 18apps 19apps 20apps" #large30
#APPS="6apps 7apps 8apps" #large
#APPS="2apps" # smalltest
PKTS="250pkts"
#FLOWSETS=$(seq 0 0) # smalltest
FLOWSETS=$(seq 0 49) #large

#
# Scheme Numbers
# 0: baseline (route error and re-dicovery)
# 1: partial route discovery from intermediate nodes
# 2: local repair with 1-hop detours
# 3: local repair with 2-hop detours
# 4: local repair with average bandwidth
#
SCHEMES="0 3"

DIR_PREFIX=$HOME"/exp/ns-3"
APPCONFIG_DIR="./apps_config"
topologyConfigFile="topology.config"
numOfNodes=`python nodes_topology.py $topologyConfigFile`


for scheme in $SCHEMES
do
	# Debug
	echo ""
	echo "@@@@@@ Scheme = "$scheme" @@@@@@"

	for app in $APPS
	do
		for pkt in $PKTS
		do
			for flowset_no in $FLOWSETS
			do

				# Select an app config file.
				appConfig=$app'_'$pkt'-'$flowset_no
				appConfigFile=$APPCONFIG_DIR'/'$appConfig'.txt'

        echo "Calculating goodput: "$appConfigFile", dir = "$DIR_PREFIX/$EXP_NAME/$scheme/$numOfNodes'nodes_'$appConfig
        python stat_goodput.py $appConfigFile $DIR_PREFIX/$EXP_NAME/$scheme/$numOfNodes'nodes_'$appConfig

			done
		done
	done
done
