import sys
import math
import json
import random
import node_distributor
import apps_parser

tx_range = 120.0
max_speed = 10
pause_time = 10

#
# (Input File Format)
# ID	type	coordinate
# 1	R	12,19
# 2	R	24,48
# 3	R	403,177
# ...
#
def load_coordinates(coordinates_file):
    nodes = []
    f = open(coordinates_file, 'r')
    lines = f.readlines()
    for line in lines:
        items = line.split()
        temp = items[2].split(',')
        node_coordinate = (int(temp[0]), int(temp[1]))
        node_config = {
            'id':int(items[0]),
            'node_type':items[1],
            'coordinate':node_coordinate
        }
        nodes.append(node_config)
    return nodes

#
# Gets the Euclidean distance between two coordinates.
# @param node1: node1's coordinate as tuple (x, y)
# @param node2: node2's coordinate as tuple (x, y)
#
def get_distance(node1, node2):
    x_dist = node1[0] - node2[0]
    y_dist = node1[1] - node2[1]
    x2y2 = (x_dist*x_dist) + (y_dist*y_dist)
    return math.sqrt(x2y2)

def make_p2p_links(nodes):
    p2p_links = []
    idx = 1
    for node1 in nodes:
        for node2 in nodes[idx:]:
            distance = get_distance(node1['coordinate'], node2['coordinate'])
            if(distance < tx_range):
                p2p_links.append((node1['id'], node2['id']))
                #print(node1['id'], node2['id'], distance)
                #p2p_link_str = '{},{},5Mbps,2ms'.format(node1['id'], node2['id'])
                #p2p_links.append(p2p_link_str)
        idx = idx+1
    return p2p_links

def find_neighbors(p2p_links, node_id):
    neighbors = []
    for node_pair in p2p_links:
        if(node_pair[0] == node_id):
            neighbors.append(node_pair[1])
        elif(node_pair[1] == node_id):
            neighbors.append(node_pair[0])
    return neighbors


def get_next_app(p2p_links, src_id, dst_id, start_time, end_time, max_speed):
    next_point = {
        'src_id':src_id,
        'next_src_id':0,
        'dst_id':dst_id,
        'start_time':start_time,
        'end_time':end_time
    }

    neighbors = find_neighbors(p2p_links, src_id)
    #print('neighors of {}: {}'.format(src_id, neighbors))
    if(len(neighbors) > 0):
        neighbor_id = neighbors[random.randint(0, len(neighbors)-1)]
        if(neighbor_id != dst_id):
            d = get_distance(nodes[src_id-1]['coordinate'], nodes[neighbor_id-1]['coordinate'])
            speed = random.randint(0, max_speed)
            if(speed > 0):
                next_start_time = start_time + d / speed + pause_time
                if(next_start_time < end_time):
                    next_point['end_time'] = int(next_start_time)
                    next_point['next_src_id'] = neighbor_id
                else:
                    next_point['next_src_id'] = neighbor_id
                    #print('to {}, time={}s\n'.format(neighbor_id, next_start_time))
    return next_point

def get_mobility(p2p_links, src_id, dst_id, start_time, end_time, max_speed):
    mobility_pattern = []
    next_point = get_next_app(p2p_links, src_id, dst_id, start_time, end_time, max_speed)
    mobility_pattern.append(next_point)
    #print(next_point)
    while next_point['end_time'] < end_time:
        next_point = get_next_app(p2p_links, next_point['next_src_id'], dst_id, next_point['end_time'], end_time, max_speed)
        mobility_pattern.append(next_point)
        #print(next_point)
    return mobility_pattern


def write_topology_file(p2p_links, output_file):
    f_p2p = open(output_topology_file, 'w')
    for p2p_link in p2p_links:
        f_p2p.write('{},{},5Mbps,2ms\n'.format(p2p_link[0], p2p_link[1]))
    f_p2p.close()

def write_apps_config_mobility(p2p_links, max_speed, file_name):
    apps = apps_parser.parse_apps_config('apps_config/{}'.format(file_name))
    f_newapps = open('apps_config_mobile/{}mps-{}'.format(max_speed, file_name), 'w')
    for app in apps:
        mp = get_mobility(p2p_links, app['src'], app['dst'], app['startTime'], app['endTime'], max_speed)
        #print(mp)
        for p in mp:
            newapp = '{}\t{}:{}\t{}:{}\tUDP\t{}\t{}\t{}.0\t{}.0\t100.0\t100.0\t0.3\n'.format(p['src_id'],p['src_id'],app['sport'], p['dst_id'],app['dport'],app['pktSize'],app['sendingRate'],p['start_time'],p['end_time']-p['start_time'])
            f_newapps.write(newapp)
    f_newapps.close()


if __name__ == "__main__":
    output_topology_file = sys.argv[1]
    coordinates_file = 'coordinates.config'

    width = 500
    height = 500
    num_nodes = 30
    coordinates = node_distributor.get_uniform_random_distribution(width, height, num_nodes)
    node_distributor.write_coordinates_to_file(coordinates, coordinates_file)

    nodes = load_coordinates(coordinates_file)
    p2p_links = make_p2p_links(nodes)
    write_topology_file(p2p_links, output_topology_file)
    print(p2p_links)

    speeds = [5, 10, 15, 20]
    for speed in speeds:
        for a in range(0,49):
            write_apps_config_mobility(p2p_links, speed, '16apps_250pkts-{}.txt'.format(a))

#    src_id = 3
#    dst_id = 20
#    start_time = 14
#    end_time = 77
#    mp = get_mobility(p2p_links, src_id, dst_id, start_time, end_time)
#    print(mp)


