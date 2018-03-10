import commands
import sys
import re
import subprocess
import flow

#
# Strip directory and extensions and return the name of the configuration file.
# e.g. "./apps_config/7apps_250pkts-3.txt" returns "7apps_250pkts-3".
#
def get_app_config(fileName):
    items = re.split('/',fileName)
    for item in items:
        if('txt' in item):
            iterations = re.split('[.]', item)
            return iterations[0]
    return '.'

#
# Use if the appConfigFile's flow profile uses IPv4 addresses (x.x.x.x).
# Note that all nodes in a simulation topology must be in the same subnet.
#
def get_dst_nodeid_from_ipaddr(addrPort):
    items = re.split('[.:]',addrPort)
    return int(items[3]) - 1

#
# Use if the appConfigFile's flow profile uses node IDs (int).
#
def get_dst_nodeid_from_dstinfo(nodeIdPort):
    items = re.split(':', nodeIdPort)
    return int(items[0])

def read_line(path, start=1, length=1):
    for line in (commands.getoutput('head -%s %s | tail -%s' % ((start + (length -1)), path, length))).split("\n"):
        yield(line)

#
# Calculate the avarage bandwidth of the flow by the destination.
#
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


if __name__ == "__main__":
    # Read flow configuration file: 'Xapps_Ypkts.txt'.
    # X: number of app flows
    # Y: packet sending rate (pkts/s)
    appConfigFile = sys.argv[1]
    acf = open(appConfigFile, 'r')

    output_str_bw = '{}\t{}'.format(get_app_config(appConfigFile), 'bandwidth')
    #output_str_qos = '{}\t{}'.format(get_app_config(appConfigFile), 'qos_violation')

    # Check the flowLog of each destination node in appConfigFile.
    lines = acf.readlines()
    for line in lines:
        items = line.split('\t')
        ac_flow = flow.Flow()
        ac_flow.parse('{} {} {}'.format(items[1], items[2], items[3]))
        dstId = get_dst_nodeid_from_dstinfo(items[2])
        startTime = int(float(items[6])) + 2
        duration = int(float(items[7]))
        bw_requirement = float(int(items[4]) * int(items[5]) * 8)

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
                avg = get_average('temp_{}_{}.txt'.format(dstId, lineNo), startTime, duration)
                output_str_bw += '\t{}'.format(avg)
                #qos_violation_ratio = get_qos_violation_ratio('temp_{}_{}.txt'.format(dstId, lineNo), bw_requirement, startTime, duration)
                #output_str_qos += '\t{}'.format(qos_violation_ratio)
                #print('requirement: {}, qos violation ratio: {}'.format(bw_requirement*0.9, qos_violation_ratio))

    print(output_str_bw)
    #print(output_str_qos)
