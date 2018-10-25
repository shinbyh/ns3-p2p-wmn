import sys
import math
import json
import node_distributor

#target_file = 'coordinates.config'
tx_range = 100.0

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
                #print(node1['id'], node2['id'], distance)
                p2p_link_str = '{},{},5Mbps,2ms'.format(node1['id'], node2['id'])
                p2p_links.append(p2p_link_str)
        idx = idx+1
    return p2p_links


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
    print(p2p_links)

    f_p2p = open(output_topology_file, 'w')
    for p2p_link in p2p_links:
        f_p2p.write('{}\n'.format(p2p_link))
    f_p2p.close()

