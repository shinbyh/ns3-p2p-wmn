#!/bin/bash

logFileName=$1
numColumns=$2

numColumns=$(($numColumns+1))

cat $logFileName | awk -v colNum="$numColumns" -F '\t' '
{
if (NF == 1) {
	printf("0\n");
} else if (NF < colNum) {
	printf("0\n");
} else {
	printf("%s\n", $colNum);
}
}'
