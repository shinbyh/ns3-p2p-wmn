import re

class Flow:
    def __init__(self, src='', sport=0, dst='', dport=0, flowType=0):
        self.src = src
        self.sport = int(sport)
        self.dst = dst
        self.dport = int(dport)
        if(flowType == 'UDP'):
            self.flowType == 0
        elif(flowType == 'TCP'):
            self.flowType == 1
        else:
            self.flowType = int(flowType)
    def __eq__(self, flow):
        if(self.src == flow.src and
              self.sport == flow.sport and
              self.dst == flow.dst and
              self.dport == flow.dport and
              self.flowType == flow.flowType):
            return True
        else:
            return False
    def __ne__(self, flow):
        return not self.__eq__(flow)
    def parse(self, string):
        items = re.split(r'[\s :]', string)
        self.src = items[0]
        self.sport = int(items[1])
        self.dst = items[2]
        self.dport = int(items[3])
        if(items[4] == 'UDP'):
            self.flowType = 0
        elif(items[4] == 'TCP'):
            self.flowType = 1
        else:
            self.flowType = int(items[4])

    def to_string(self):
        out = 'flow = {}:{} {}:{} {}'.format(self.src, self.sport, self.dst, self.dport, self.flowType)
        return out
#
# Testcode
#
#f1 = Flow('10.1.1.1', 6677, '10.1.1.4', 5004, 0)
#f2 = Flow('10.1.1.10', 6677, '10.1.1.5', 5004, 0)
#f3 = Flow('10.1.1.1', 6677, '10.1.1.4', 5004, 0)
#print f1 == f2
#print f1 == f3
#f3.parse('10.1.1.5:6655 10.1.1.7:443 1')
#print f3.dst
#f4 = Flow()
#print f4.sport
#f4.parse('10.1.1.5:6655 10.1.1.7:443 1')
#print f4.sport
