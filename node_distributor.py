import math
import random

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
# Test Code for the function
#
if __name__ == "__main__":
    coordinates = get_uniform_random_distribution(500, 500, 30)
    print(len(coordinates))
    print(coordinates)
    write_coordinates_to_file(coordinates, 'coordinate_test.config')


