import flow

#appConfigFile = 'apps_config/9apps_250pkts-9.txt'

def parse_apps_config(appConfigFile):
    apps = []
    f = open(appConfigFile, 'r')
    lines = f.readlines()
    for line in lines:
        items = line.split('\t')
        ac_flow = flow.Flow()
        ac_flow.parse('{} {} {}'.format(items[1], items[2], items[3]))
        pktSize = int(items[4])
        sendingRate = int(items[5])
        startTime = int(float(items[6]))
        duration = int(float(items[7]))
        endTime = startTime + duration
        app = {
            'src':int(ac_flow.src),
            'dst':int(ac_flow.dst),
            'sport':ac_flow.sport,
            'dport':ac_flow.dport,
            'flowType':ac_flow.flowType,
            'pktSize':pktSize,
            'sendingRate':sendingRate,
            'startTime':startTime,
            'duration':duration,
            'endTime':endTime
        }
        apps.append(app)
        #print(ac_flow.src, ac_flow.dst, startTime, endTime)
    return apps


