import sys

def get_number_of_nodes(topology_file):
    maxNodeId = 0
    with open(topology_file, 'r') as f:
        lines = f.readlines()
        for line in lines:
            linkItems = line.split(',')
            if(len(linkItems) >= 4):
                if(maxNodeId < int(linkItems[0])):
                    maxNodeId = int(linkItems[0])
                if(maxNodeId < int(linkItems[1])):
                    maxNodeId = int(linkItems[1])

    maxNodeId += 1
    return maxNodeId

if __name__ == "__main__":
    # argv[1] = topology configuration file (e.g. topology.config)
    topology_file = sys.argv[1]
    maxNodeId = get_number_of_nodes(topology_file)
    print(maxNodeId)
