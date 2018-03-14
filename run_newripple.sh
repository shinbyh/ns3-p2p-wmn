#!/bin/bash

BASE_DIR=$(pwd)
EXP_NAME=$1
APPS="6apps 7apps 8apps"
PKTS="250pkts"
FLOWSETS="0 1 2 3 4 5 6 7 8 9"

#
# Scheme Numbers
# 0: baseline (route error and re-dicovery)
# 1: partial route discovery from intermediate nodes
# 2: local repair with 1-hop detours
# 3: local repair with 2-hop detours
# 4: local repair with average bandwidth
#
SCHEMES="0 2"

DIR_PREFIX=$HOME"/exp/ns-3"
APPCONFIG_DIR="./apps_config"
topologyConfigFile="topology.config"
numOfNodes=`python nodes_topology.py $topologyConfigFile`

mkdir $DIR_PREFIX/$EXP_NAME

#trap "exit" INT
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

				# Debug
				#echo "Number of nodes: "$numOfNodes
				echo "Simulating "$appConfigFile

				# Run an ns-3 simulation.
				./waf --run "p2p-bhshin --apps=$appConfig --scheme=$scheme --topology=$topologyConfigFile" &> output.txt

				# Make statistics and copy to the dedicated location.
				python stat.py $appConfigFile
				python stat_ov.py $numOfNodes
				python avg_thp.py $appConfigFile > nodestat_avg.txt
				python avg_qos_violation.py $appConfigFile > nodestat_avg_qos_vio.txt
				python avg_src_rt.py $appConfigFile > nodestat_rt_dscv_cnt.txt
				./copy_results2.sh $appConfig $numOfNodes $EXP_NAME $scheme
			done
		done
	done
done


######################################################################
# Export necessary environment variables for "integrate_stat.sh".
######################################################################
export EXP_NAME
export APPS
export PKTS
export FLOWSETS
export SCHEMES
export DIR_PREFIX
export APPCONFIG_DIR
export topologyConfigFile
export numOfNodes

######################################################################
# Integrate statistics.
######################################################################
./integrate_stat.sh > $DIR_PREFIX/$EXP_NAME/integrated_stat_thp.txt
./integrate_stat_qos_vio.sh > $DIR_PREFIX/$EXP_NAME/integrated_stat_qos_vio.txt
./integrate_ovstat.sh > $DIR_PREFIX/$EXP_NAME/integrated_stat_overhead.txt
./integrate_stat_src_rt.sh > $DIR_PREFIX/$EXP_NAME/integrated_stat_rt_dscv_cnt.txt

######################################################################
# Notify the end of simulation.
######################################################################
$BASE_DIR/slack_webhook.sh 'Your ns-3 simulation has finished: '$EXP_NAME
