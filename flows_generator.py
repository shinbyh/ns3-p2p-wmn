import sys
import random

target_dir = 'apps_config'
node_id_min = 1
start_time_min = 10
start_time_max = 35
duration_min = 50
duration_max = 65
flow_type = 'UDP'
pkt_size = 1024
pkts_per_second = 250

qos_delay = 100.0
qos_jitter = 100.0
qos_lossrate = 0.3

def generate_flow_requirements(node_id_max, num_of_flows, num_of_flowsets):
    for i in range(0, num_of_flowsets):
        filename = '{}/{}apps_{}pkts-{}.txt'.format(target_dir, num_of_flows, pkts_per_second, i)
        f = open(filename, 'w')
        for j in range(0, num_of_flows):
            # src:sport --> dst:dport
            src = random.randint(node_id_min, node_id_max)
            dst = random.randint(node_id_min, node_id_max)
            while src == dst:
                dst = random.randint(node_id_min, node_id_max)
            sport = random.randint(50000, 65535)
            dport = random.randint(1, 5000)

            # Start & end times
            startTime = random.randint(start_time_min, start_time_max)
            duration = random.randint(duration_min, duration_max)

            flow_config_str = '{}\t{}:{}\t{}:{}\tUDP\t{}\t{}\t{}.0\t{}.0\t{}\t{}\t{}'.format(src, src, sport, dst, dport, pkt_size, pkts_per_second, startTime, duration, qos_delay, qos_jitter, qos_lossrate)

            #print(flow_config_str)
            f.write(flow_config_str)
            f.write('\n')
        f.close()
        print('generated {} flows (iteration {})'.format(num_of_flows, i))



if __name__ == "__main__":
    node_id_max = int(sys.argv[1])
    num_of_flows = int(sys.argv[2])
    num_of_flowsets = int(sys.argv[3])

    generate_flow_requirements(node_id_max, num_of_flows, num_of_flowsets)
