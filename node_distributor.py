import math
import random
import sys
import csv
from geopy.distance import geodesic

#####################################################
# (Node Distributor)
# It creates a (pseudo) uniform random distribution
# of nodes in the specified square area of
# X * Y square meters.
#####################################################

def get_uniform_random_distribution(width, height, num_nodes):
    coordinates = []

    x_range = width / math.sqrt(num_nodes)
    y_range = height / math.sqrt(num_nodes)
    x_num = width / x_range
    y_num = height / y_range
    #print('x_range = {}, y_range = {}'.format(x_range, x_range))
    #print('x_num = ',x_num)

    counter = 0
    ys = 0
    for yc in range(int(y_num)):
        xs = 0
        for xc in range(int(x_num)):
            counter += 1
            x = random.randint(0, int(x_range))
            y = random.randint(0, int(y_range))
            coordinates.append((xs+x, ys+y))
            #print(' {} - ({},{})'.format(counter, xs+x, ys+y))
            xs += x_range
        ys += y_range

    for n in range(num_nodes - counter):
        x = random.randint(0, int(x_range))
        y = random.randint(0, int(y_range))
        xs = random.randint(0, int(x_num))
        ys = random.randint(0, int(y_num))
        coordinates.append((int(xs*x_range+x),int(ys*y_range+y)))
        #print(' {} - ({},{})'.format(n, xs*x_range+x, ys*y_range+y))

    return coordinates


def write_coordinates_to_file(coordinates, file_name):
    f = open(file_name,'w')
    counter = 1
    for co in coordinates:
        f.write('{}\tR\t{},{}\n'.format(counter,int(co[0]),int(co[1])))
        counter += 1
    f.close()


#
# Loads a set of GPS coordinates from a text file.
# Format:
# BSSID\tLATITUDE\tLONGITUDE\n
#
def load_ap_coordinates_from_file(file_name):
    ap_topology_set = []
    f = open(file_name, 'r')
    lines = f.readlines()
    for line in lines:
        items = line.split()
        node_info = {}
        node_info['bssid'] = items[0]
        node_info['lat'] = float(items[1])
        node_info['lon'] = float(items[2])
        ap_topology_set.append(node_info)
    f.close()
    # print(ap_topology_set)
    print('Total', len(ap_topology_set), 'APs are loaded from ', file_name)
    return ap_topology_set


def load_ap_interaction_data_from_csv(csv_file_name):
    ap_data_set = []
    with open(csv_file_name) as csvfile:
        ap_reader = csv.reader(csvfile, delimiter=',')
        line_count = 0
        for row in ap_reader:
            if line_count == 0:
                line_count += 1
                continue
            else:
                try:
                    ap_data = {}
                    ap_data['timestamp'] = int(row[0])
                    ap_data['state'] = int(row[2])
                    ap_data['tget1'] = float(row[7])
                    ap_data['tget5'] = float(row[8])
                    ap_data['tput'] = float(row[9])
                    ap_data['ping_avg'] = float(row[11])
                    ap_data['loss_rate'] = float(row[12])
                    ap_data['rss'] = int(row[14])
                    ap_data['bssid'] = row[17]
                    ap_data['channel'] = int(row[19])
                    ap_data['distance'] = float(row[22])
                    ap_data_set.append(ap_data)
                    line_count += 1
                except ValueError:
                    pass
    print(ap_data_set)
    print('Total', len(ap_data_set), 'AP measurement items')
    return ap_data_set


def get_comparison_point(ap_topology_set):
    smallest_lat = 91.0
    smallest_lon = 181.0
    for ap_info in ap_topology_set:
        if ap_info['lat'] < smallest_lat:
            smallest_lat = ap_info['lat']
        if ap_info['lon'] < smallest_lon:
            smallest_lon = ap_info['lon']
    return smallest_lat, smallest_lon


def get_coordinate_meters(smallest_lat, smallest_lon, target_lat, target_lon):
    x_meters = geodesic((target_lat, smallest_lon), (target_lat, target_lon)).meters
    y_meters = geodesic((smallest_lat, target_lon), (target_lat, target_lon)).meters
    return x_meters, y_meters


def get_distance_meters(lat_1, lon_1, lat_2, lon_2):
    return geodesic((lat_1, lon_1), (lat_2, lon_2)).meters


def convert_gps_to_meter_coordinates(ap_topology_set):
    s_lat, s_lon = get_comparison_point(ap_topology_set)

    for ap_info in ap_topology_set:
        x, y = get_coordinate_meters(s_lat, s_lon, ap_info['lat'], ap_info['lon'])
        ap_info['x'] = x
        ap_info['y'] = y


def print_usage():
    print('Usage: $ python node_distributor.py [X] [Y] [N] [output_file]')
    print(' - X: width of a square area')
    print(' - Y: height of a square area')
    print(' - N: the number of nodes to be distributed')
    print(' - output_file: an output file name of node coordinates')
    print('e.g. $ python node_distributor.py 500 500 30 coordinates.config')


if __name__ == "__main__":
    if len(sys.argv) < 4:
        print_usage()
        ap_topology_set = load_ap_coordinates_from_file('metrofi/aps.txt')

        convert_gps_to_meter_coordinates(ap_topology_set)
        print('----------------')
        print(ap_topology_set)

        #s_lat, s_lon = get_comparison_point(ap_topology_set)
        #print('(s_lat, s_lon) = ({}, {})'.format(s_lat, s_lon))
        #x, y = get_coordinate_meters(s_lat, s_lon, ap_topology_set[0]['lat'], ap_topology_set[0]['lon'])
        #dist = get_distance_meters(s_lat, s_lon, ap_topology_set[0]['lat'], ap_topology_set[0]['lon'])
        #print('(x, y) = ({}, {}) meters'.format(x, y))
        #print('  - dist[0] = ', dist)
        load_ap_interaction_data_from_csv('metrofi/random.csv')

    else:
        x = int(sys.argv[1])
        y = int(sys.argv[2])
        n = int(sys.argv[3])
        output_file = sys.argv[4]
        coordinates = get_uniform_random_distribution(x, y, n)
        print(len(coordinates))
        print(coordinates)
        write_coordinates_to_file(coordinates, output_file)
