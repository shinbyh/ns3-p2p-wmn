import sys
import subprocess

#
# DEBUG: Print portBwMap on the shell.
#
def print_portBwMap(portBwMap):
    for portKey in portBwMap.keys():
        for nodeId in portBwMap[portKey].keys():
            print(' portBwMap[{}][{}] = {}'.format(portKey, nodeId, portBwMap[portKey][nodeId]))

#
# DEBUG: Print portBwSumMap on the shell.
#
def print_portBwSumMap(portBwSumMap):
    for portKey in portBwSumMap.keys():
        print(' bw[{}] = {}'.format(portKey, portBwSumMap[portKey]))

#
# Write the total bandwidth of all port numbers to the file (fileName).
# The port numbers indicate the types of control messages.
#
def write_portBwSum(portBwSumMap):
    f = open('ovstat_sum.txt', 'w')
    #f.write('Port\tBandwidth\n')
    for portNum in portBwSumMap.keys():
        f.write('{}\t'.format(portBwSumMap[portNum]))
        #f.write('{}\t{}\n'.format(portNum, portBwSumMap[portNum]))
    f.write('\n')
    f.close()

def write_portOrder(portBwSumMap):
    f = open('ovstat_ports.txt', 'w')
    for portNum in portBwSumMap.keys():
        f.write('{}\t'.format(portNum))
    f.write('\n')
    f.close()

#
# Write the bandwidth of each node ID for a specific port number.
#
def write_portBw(portBwMap, portNum):
    f = open('ovstat_{}.txt'.format(portNum), 'w')
    f.write('Port {}\n'.format(portNum))
    f.write('NodeID\tBandwidth\n')
    for nodeId in portBwMap[portNum].keys():
        f.write('{}\t{}\n'.format(nodeId, portBwMap[portNum][nodeId]))
    f.close()

def write_portBwPerNode(portBwMap):
    for portNum in portBwMap.keys():
        write_portBw(portBwMap, portNum)

#
# Returns the sum of all numbers in a column of the target text file.
# The text file must have numbers where each column is separated with
# another column by delimiters such as a tab character ('\t').
#
def get_sum_of_column(fileName, columnNo):
    try:
        columnOut = subprocess.Popen(['./getAColumn.sh', fileName, '{}'.format(columnNo)], stdout=subprocess.PIPE)
        formulaOut = subprocess.Popen(['paste', '-sd+', '-'], stdin=columnOut.stdout, stdout=subprocess.PIPE)
        columnOut.stdout.close()
        bcOut = subprocess.Popen(['bc'], stdin=formulaOut.stdout, stdout=subprocess.PIPE)
        formulaOut.stdout.close()
        bcResult = bcOut.stdout.read()
        bcOut.stdout.close()
        return int(bcResult)
    except:
        return 0

if __name__ == "__main__":
    # Get number of nodes as arguments
    numOfNodes = int(sys.argv[1])
    portBwMap = {}
    portBwSumMap = {}

    # Debug
    #print('numOfNodes = {}'.format(numOfNodes))

    for i in range(0, numOfNodes):
        try:
            # Read ovinfo_X.txt and get port numbers
            with open('ovinfo_{}.txt'.format(i), 'r') as f:
                line = f.readline()
                portStrs = line.split('\t')
                count = 0

                for portStr in portStrs:
                    count += 1
                    try:
                        portNum = int(portStr)
                        ovlogFile = 'ovlog_{}.txt'.format(i)
                        bcResult = get_sum_of_column(ovlogFile, count)

                        # Add value to the maps.
                        if portNum not in portBwMap.keys():
                            portBwMap[portNum] = {}
                            portBwSumMap[portNum] = 0
                        portBwMap[portNum][i] = bcResult
                        portBwSumMap[portNum] += bcResult
                    except:
                        print('Error analyzing ovlog_{}.txt (portStr: {})'.format(i, portStr))
                        pass
        except:
            # Error reading a file.
            print('Cannot read ovinfo_{}.txt'.format(i))
            pass

    #print_portBwMap(portBwMap)
    #print_portBwSumMap(portBwSumMap)
    write_portBwSum(portBwSumMap)
    write_portOrder(portBwSumMap)
    write_portBwPerNode(portBwMap)
