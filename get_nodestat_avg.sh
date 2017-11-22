#!/bin/bash

# EXP_DIR must be an absolute path.
EXP_ROOT_DIR=$1

cd $EXP_ROOT_DIR

NODES="25 36 49"
APPS="3 4 5"
PKTS="10 30 50 70 100"

for node in $NODES
do
	for app in $APPS
	do
		echo $node"nodes_"$app"apps"
		for pkt in $PKTS
		do
			dir_name=$node"nodes_"$app"apps_"$pkt"pkts"
			cd $dir_name
			cat nodestat_avg.txt
			cd ..
		done
		echo ""
	done
done




#FILES=`ls`
#for dir in $FILES
#do
#	echo "dir: "$dir
#	cd $dir
#	cat nodestat_avg.txt
#	cd ..
#done
