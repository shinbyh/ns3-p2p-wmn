import sys
import re
import flow
import avg_thp

#
# Calculate QoS violation ratio in time by the destination.
#
def get_qos_violation_ratio(txt_file, bw_requirement, start_line_no=1, num_of_lines=1):
    total = 0
    count = 0
    for i in avg_thp.read_line(txt_file, start_line_no, num_of_lines):
        if i is None:
            return 0
        elif i is '':
            return 0
        else:
            if float(bw_requirement)*0.9 > float(i):
                count += 1
            total += 1
    #print('count: {}, total: {}'.format(count, total))
    return float(count)/float(total)


# Read flow configuration file: 'Xapps_Ypkts.txt'.
# X: number of app flows
# Y: packet sending rate (pkts/s)
appConfigFile = sys.argv[1]
acf = open(appConfigFile, 'r')
output_str_qos = '{}\t{}'.format(avg_thp.get_app_config(appConfigFile), 'qos_violation')

# Check the flowLog of each destination node in appConfigFile.
lines = acf.readlines()
for line in lines:
    items = line.split('\t')
    ac_flow = flow.Flow()
    ac_flow.parse('{} {} {}'.format(items[1], items[2], items[3]))
    dstId = avg_thp.get_dst_nodeid_from_dstinfo(items[2])
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
            qos_violation_ratio = get_qos_violation_ratio('temp_{}_{}.txt'.format(dstId, lineNo), bw_requirement, startTime, duration)
            output_str_qos += '\t{}'.format(qos_violation_ratio)
            #print('requirement: {}, qos violation ratio: {}'.format(bw_requirement*0.9, qos_violation_ratio))

print(output_str_qos)
