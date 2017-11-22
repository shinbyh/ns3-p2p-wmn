import sys
import re
import subprocess
import flow

def get_dst_nodeid_from_ipaddr(addrPort):
    items = re.split('[.:]',addrPort)
    return int(items[3]) - 1 

# Read 'Xapps_Ypkts.txt'.
# X: number of app flows
# Y: packet sending rate (pkts/s)
appConfigFile = sys.argv[1]
acf = open(appConfigFile, 'r')

prevNode = -1

# Check the flowLog of each destination node in appConfigFile.
lines = acf.readlines()
for line in lines:
    items = line.split('\t')
    ac_flow = flow.Flow()
    ac_flow.parse('{} {} {}'.format(items[1], items[2], items[3]))
    print('analyzing {}'.format(ac_flow.toString()))
    dstId = get_dst_nodeid_from_ipaddr(items[2])
    
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
            flowLogFile = 'flowlog_{}.txt'.format(dstId)
            tempLogFile = 'newtemp_{}_{}.txt'.format(dstId, lineNo)
            stdOutTempLog = open(tempLogFile, 'w')
            subprocess.call(['./getAColumn.sh', flowLogFile, '{}'.format(lineNo)], stdout=stdOutTempLog)
            subprocess.call(['sed', '-i', '1in{}_f{}'.format(dstId, lineNo), tempLogFile])

            # Paste each column to the result file: nodestat.txt
            stdOutRes = open('res_{}.txt'.format(dstId), 'w')
            if(prevNode == -1):
                prevNode = dstId
                subprocess.call(['paste', tempLogFile], stdout=stdOutRes)
            else:
                subprocess.call(['paste', 'res_{}.txt'.format(prevNode), tempLogFile], stdout=stdOutRes)
                prevNode = dstId

subprocess.call(['mv', 'res_{}.txt'.format(prevNode), 'nodestat.txt'])
