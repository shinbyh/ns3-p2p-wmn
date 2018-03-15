#!/bin/bash

FILE_PREFIXES="flowinfo_ flowlog_ srcrtlog ovinfo_ ovlog_ ovstat_ nodelog_ nodestat integrated_stat_ output temp_"

for prefix in $FILE_PREFIXES
do
  rm $prefix*.txt 2> /dev/null
done

rm *.pcap 2> /dev/null
