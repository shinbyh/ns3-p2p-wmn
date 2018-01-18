#!/bin/bash

EXP_NAME=$1
NODES="12" # extended_diamond2
APPS="3apps"
PKTS="150pkts"
SCHEMES="0 1"
NODE_POSITION="extended_diamond2"
DIR_PREFIX=$HOME"/exp/ns-3"
APPCONFIG_DIR="./apps_config"

mkdir $DIR_PREFIX/$EXP_NAME

for scheme in $SCHEMES
do
	mkdir $DIR_PREFIX/$EXP_NAME/$scheme

	# Debug
	echo ""
	echo "@@@@@@ Scheme = "$scheme" @@@@@@"

	for node in $NODES
	do
		# Debug
		echo " #### "$node" nodes ####"

		for app in $APPS
		do
			for pkt in $PKTS
			do
				# Remove previous data files.
				rm flowinfo_*.txt flowlog_*.txt ovinfo_*.txt ovlog_*.txt nodelog_*.txt temp_*.txt newtemp*.txt res_*.txt nodestat*.txt *.pcap 2> /dev/null

				# Select an app config file.
				appConfig=$app'_'$pkt'_ed'
				appConfigFile=$APPCONFIG_DIR'/'$appConfig'.txt'

				# Debug
				echo "Simulating "$appConfigFile

				# Run an ns-3 simulation.
				./waf --run "p2p-bhshin --apps=$appConfig --nodes=$node --scheme=$scheme --npos=$NODE_POSITION" &> output.txt

				# Make statistics and copy to the dedicated location.
				python stat.py $appConfigFile
				python avg_thp.py $appConfigFile > nodestat_avg.txt
				./copy_results2.sh $appConfig $node $EXP_NAME $scheme
			done
		done
	done
done

./slack_webhook.sh 'Your ns-3 simulation has finished: '$EXP_NAME
