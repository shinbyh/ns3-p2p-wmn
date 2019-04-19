from pyproj import Transformer
import numpy as np
import math
from math import radians

def aaa(lat, lon, r_earth=6378):
    m_per_deg_lat = 111132.954 - 559.822 * math.cos( 2.0 * lat ) + 1.175 * math.cos( 4.0 * lat)
    m_per_deg_lon = (3.14159265359/180 ) * 6367449 * math.cos( lat )
    print(m_per_deg_lat)
    print(m_per_deg_lon)
    return m_per_deg_lat, m_per_deg_lon


def bbb(latMid, r_earth=6378):
    m_per_deg_lat = 111132.954 - 559.822 * math.cos( 2.0 * latMid) + 1.175 * math.cos( 4.0 * latMid)
    m_per_deg_lon = (3.14159265359/180 ) * 6367449 * math.cos(latMid)
    print(m_per_deg_lat)
    print(m_per_deg_lon)
    return m_per_deg_lat, m_per_deg_lon


def get_coordinate_meters(lat, lon):
    transproj = Transformer.from_proj({"proj":'geocent', "ellps":'WGS84', "datum":'WGS84'}, '+init=EPSG:4326')
    # transformer = Transformer.from_crs("epsg:4326", "epsg:3857")
    x, y, z = transproj.transform(lon, lat, 0)
    # x,y = pyproj.transform(wgs84, epsg3035, lon, lat)
    return 1000*x, 1000*y


def get_distance_meters(x1, y1, x2, y2):
    return np.sqrt((x2-x1)**2 + (y2-y1)**2)


if __name__ == "__main__":
    x1, y1 = get_coordinate_meters(45.523645, -122.660899)
    print(x1, y1)
    x2, y2 = get_coordinate_meters(45.519469, -122.648693)
    print(x2, y2)
    dist = get_distance_meters(x1, y1, x2, y2)
    print(dist)
    x3, y3 = get_coordinate_meters(45.517811, -122.6537)
    print(x3, y3)
    dx, dy = aaa(45.523645, -122.660899)
    latMid = (45.523645 + 45.519469)/2.0
    m_per_deg_lat,m_per_deg_lon = bbb(latMid)
    deltaLat = 45.523645 - 45.519469
    deltaLon = -122.660899 + 122.648693
    dist_m = math.sqrt ((deltaLat * m_per_deg_lat)**2 + (deltaLon * m_per_deg_lon)**2 )
    print('dist_m = ', dist_m)

    print('---------------------------------------')
    x4, y4 = get_coordinate_meters(45.526630, -122.648470)
    x5, y5 = get_coordinate_meters(45.520635, -122.681620)
    dist = get_distance_meters(x4, y4, x5, y5)
    print('(x4,y4) = ', x4, y4)
    print('(x5,y5) = ', x5, y5)
    print(' - dist = ', dist)
    
    latMid = (45.526630 + 45.520635) / 2.0
    print('latmid = ', latMid)
    m_per_deg_rat, m_per_deg_lon = bbb(latMid)
    deltaLat = 45.526630 - 45.520635
    deltaLon = -122.648470 + 122.681620
    dist_m = math.sqrt ((deltaLat * m_per_deg_lat)**2 + (deltaLon * m_per_deg_lon)**2 )
    print('new dist_m = ', dist_m)

    print('---------------------------------------')
    gps1 = (radians(45.526630), radians(-122.648470))
    gps2 = (radians(45.520635), radians(-122.681620))
    x4, y4 = get_coordinate_meters(gps1[0], gps1[1])
    x5, y5 = get_coordinate_meters(gps2[0], gps2[1])
    dist = get_distance_meters(x4, y4, x5, y5)
    print('(x4,y4) = ', x4, y4)
    print('(x5,y5) = ', x5, y5)
    print(' - dist = ', dist)
    
    latMid = (gps1[0] + gps2[0]) / 2.0
    print('latmid = ', latMid)
    m_per_deg_rat, m_per_deg_lon = bbb(latMid)
    deltaLat = gps1[0] - gps2[0]
    deltaLon = gps1[1] + gps2[1]
    dist_m = math.sqrt ((deltaLat * m_per_deg_lat)**2 + (deltaLon * m_per_deg_lon)**2 )
    print('new dist_m = ', dist_m)
