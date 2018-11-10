import math
import random
import sys

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

    else:
        x = int(sys.argv[1])
        y = int(sys.argv[2])
        n = int(sys.argv[3])
        output_file = sys.argv[4]
        coordinates = get_uniform_random_distribution(x, y, n)
        print(len(coordinates))
        print(coordinates)
        write_coordinates_to_file(coordinates, output_file)
