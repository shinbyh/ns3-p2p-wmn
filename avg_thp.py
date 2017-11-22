import commands
import sys
import re
import subprocess
import flow


def get_app_config(fileName):
    items = re.split('[_.]',fileName)
    return items[1]

def get_dst_nodeid_from_ipaddr(addrPort):
    items = re.split('[.:]',addrPort)
    return int(items[3]) - 1 

def read_line(path, start=1, length=1):  
    for line in (commands.getoutput('head -%s %s | tail -%s' % ((start + (length -1)), path, length))).split("\n"):
        yield(line)

def get_average(txtFile, startLineNum=1, numOfLines=1):
    total = 0 
    for i in read_line(txtFile, startLineNum, numOfLines):
        if i is None:
            return 0
        elif i is '': 
            return 0
        else:
            total += float(i)
    return total/numOfLines

# Read 'Xapps_Ypkts.txt'.
# X: number of app flows
# Y: packet sending rate (pkts/s)
appConfigFile = sys.argv[1]
acf = open(appConfigFile, 'r')

outputStr = get_app_config(appConfigFile)

# Check the flowLog of each destination node in appConfigFile.
lines = acf.readlines()
for line in lines:
    items = line.split('\t')
    ac_flow = flow.Flow()
    ac_flow.parse('{} {} {}'.format(items[1], items[2], items[3]))
    dstId = get_dst_nodeid_from_ipaddr(items[2])
    startTime = int(float(items[6])) + 2
    duration = int(float(items[7]))

    # One flowlog file may contain multiple columns.
    lineNo = 0
    flowInfoFile = 'flowinfo_{}.txt'.format(dstId)
    fif = open(flowInfoFile, 'r')
    fifLines = fif.readlines()
    for fifLine in fifLines:
        lineNo += 1
        fitems = re.split(r'[\s :]', fifLine.strip())
        if not (len(fitems) == 6):
            continue
        fi_flow = flow.Flow(fitems[1], fitems[2], fitems[3], fitems[4], fitems[5])
        
        # Handle flows only matching with appConfigFile
        if(ac_flow == fi_flow):
            avg = get_average('newtemp_{}_{}.txt'.format(dstId, lineNo), startTime, duration)
            outputStr += '\t{}'.format(avg)

print(outputStr)
