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
