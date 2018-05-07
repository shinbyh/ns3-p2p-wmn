#!/bin/bash

######################################################################
# All variables below will be provided by environment variables.
######################################################################
#EXP_NAME=$1
#APPS="7apps"
#PKTS="250pkts"
#FLOWSETS="0 1 2 3 4 5 6 7 8 9"
#SCHEMES="0 1 2"
#DIR_PREFIX=$HOME"/exp/ns-3"
#APPCONFIG_DIR="./apps_config"
#topologyConfigFile="topology.config"
#numOfNodes=`python nodes_topology.py $topologyConfigFile`

######################################################################
# Integrate statistics.
######################################################################
cd $DIR_PREFIX"/"$EXP_NAME
for scheme in $SCHEMES
do
	cd $scheme
	for app in $APPS
	do
		for pkt in $PKTS
		do
			echo 'Scheme '$scheme
			for flowset_no in $FLOWSETS
			do
				configDir=$numOfNodes'nodes_'$app'_'$pkt'-'$flowset_no
				cd $configDir
				app_pkt_flowno_str=$app'_'$pkt'-'$flowset_no
				goodput_qv_sum_data=`cat flow_goodput_qv_sum.txt`
				printf '%s\t%s\n' "[$app_pkt_flowno_str, $goodput_qv_sum_data]"
				#echo -e $app"_"$pkt"-"$flowset_no"\t"$goodput_qv_sum_data
				cd ..
			done
			echo ''
			echo ''
		done
	done
	cd ..
done
