#!/bin/bash

EXP_NAME=$1
APPS="7apps"
PKTS="250pkts"
FLOWSETS="0 1 2 3 4 5 6 7 8 9"
SCHEMES="2"
DIR_PREFIX=$HOME"/exp/ns-3"
APPCONFIG_DIR="./apps_config"

mkdir $DIR_PREFIX/$EXP_NAME

for scheme in $SCHEMES
do
	mkdir $DIR_PREFIX/$EXP_NAME/$scheme

	# Debug
	echo ""
	echo "@@@@@@ Scheme = "$scheme" @@@@@@"

	for app in $APPS
	do
		for pkt in $PKTS
		do
			for flowset_no in $FLOWSETS
			do
				# Remove previous data files.
				rm flowinfo_*.txt flowlog_*.txt ovinfo_*.txt ovlog_*.txt ovstat_*.txt nodelog_*.txt temp_*.txt newtemp*.txt res_*.txt nodestat*.txt *.pcap 2> /dev/null

				# Select an app config file.
				appConfig=$app'_'$pkt'-'$flowset_no
				appConfigFile=$APPCONFIG_DIR'/'$appConfig'.txt'
				topologyConfigFile="topology.config"
				numOfNodes=`python nodes_topology.py $topologyConfigFile`

				# Debug
				echo "Number of nodes: "$numOfNodes
				echo "Simulating "$appConfigFile

				# Run an ns-3 simulation.
				./waf --run "p2p-bhshin --apps=$appConfig --scheme=$scheme --topology=$topologyConfigFile" &> output.txt

				# Make statistics and copy to the dedicated location.
				python stat.py $appConfigFile
				python stat_ov.py $numOfNodes
				python avg_thp.py $appConfigFile > nodestat_avg.txt
				./copy_results2.sh $appConfig $numOfNodes $EXP_NAME $scheme
			done
		done
	done
done

./slack_webhook.sh 'Your ns-3 simulation has finished: '$EXP_NAME
