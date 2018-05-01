#!/bin/bash

BASE_DIR=$(pwd)
EXP_NAME=$1
APPS="10apps 11apps 12apps 13apps 14apps 15apps 16apps 17apps 18apps 19apps 20apps" #large30
#APPS="6apps 7apps 8apps" #large
#APPS="2apps" # smalltest
#APPS="20apps" # special case
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
SCHEMES="3 0"

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
				# Remove previous simulation data for the accuracy of results.
				$BASE_DIR/remove_log_data.sh

				# Select an app config file.
				appConfig=$app'_'$pkt'-'$flowset_no
				appConfigFile=$APPCONFIG_DIR'/'$appConfig'.txt'

				# Run an ns-3 simulation.
				echo "Simulating "$appConfigFile
				./waf --run "p2p-bhshin --apps=$appConfig --scheme=$scheme --topology=$topologyConfigFile" &> output.txt

				# Make statistics results and copy them to the dedicated location.
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
$BASE_DIR/integrate_stat.sh > $DIR_PREFIX/$EXP_NAME/integrated_stat_thp.txt
$BASE_DIR/integrate_stat_qos_vio.sh > $DIR_PREFIX/$EXP_NAME/integrated_stat_qos_vio.txt
$BASE_DIR/integrate_ovstat.sh > $DIR_PREFIX/$EXP_NAME/integrated_stat_overhead.txt
$BASE_DIR/integrate_stat_src_rt.sh > $DIR_PREFIX/$EXP_NAME/integrated_stat_rt_dscv_cnt.txt
$BASE_DIR/integrate_stat_goodput.sh > $DIR_PREFIX/$EXP_NAME/integrated_stat_goodput.txt

######################################################################
# Notify the end of simulation.
######################################################################
HOSTNAME=`hostname`
$BASE_DIR/slack_webhook.sh 'Your ns-3 simulation has finished: '$EXP_NAME' (host: '$HOSTNAME')'

# Remove previous data files.
$BASE_DIR/remove_log_data.sh
