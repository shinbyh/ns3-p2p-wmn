#!/bin/bash

FILE_PREFIXES="applog_ flowinfo_ flowlog_ flow_qos flow_goodput srcrtlog ovinfo_ ovlog_ ovstat_ nodelog_ nodestat integrated_stat_ output temp_"

for prefix in $FILE_PREFIXES
do
  rm $prefix*.txt 2> /dev/null
done

rm *.pcap 2> /dev/null
