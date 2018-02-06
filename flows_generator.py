import sys
import random

target_dir = 'apps_config'

nodeId_min = 1
nodeId_max = int(sys.argv[1])
startTime_min = 10
duration_min = 10
duration_max = 30

flow_type = 'UDP'
pkt_size = 1024
pkts_per_second = 250

qos_delay = 100.0
qos_jitter = 100.0
qos_lossrate = 0.3

number_of_flows = int(sys.argv[2])
number_of_flowsets = int(sys.argv[3])

for i in range(0, number_of_flowsets):
    filename = '{}/{}apps_{}pkts-{}.txt'.format(target_dir, number_of_flows, pkts_per_second, i)
    f = open(filename, 'w')
    for j in range(0, number_of_flows):
        # src:sport --> dst:dport
        src = random.randint(nodeId_min, nodeId_max)
        dst = random.randint(nodeId_min, nodeId_max)
        while src == dst:
            dst = random.randint(nodeId_min, nodeId_max)
        sport = random.randint(50000, 65535)
        dport = random.randint(1, 5000)

        # Start & end times
        startTime = random.randint(startTime_min, 13)
        duration = random.randint(duration_min, duration_max)

        flow_config_str = '{}\t{}:{}\t{}:{}\tUDP\t{}\t{}\t{}.0\t{}.0\t{}\t{}\t{}'.format(src, src, sport, dst, dport, pkt_size, pkts_per_second, startTime, duration, qos_delay, qos_jitter, qos_lossrate)

        #print(flow_config_str)
        f.write(flow_config_str)
        f.write('\n')
    f.close()
    print('generated {} flows (iteration {})'.format(number_of_flows, i))
