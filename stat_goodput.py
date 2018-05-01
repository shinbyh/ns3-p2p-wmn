import sys
import re
import subprocess
import flow

#
# Strip directory and extensions and return the name of the configuration file.
# e.g. "./apps_config/7apps_250pkts-3.txt" returns "7apps_250pkts-3".
#
def get_app_config(fileName):
    items = re.split('/',fileName)
    for item in items:
        if('txt' in item):
            iterations = re.split('[.]', item)
            return iterations[0]
    return '.'

def get_average_of_applog(txt_file):
    total = 0.0
    count = 0

    f = open(txt_file, 'r')
    lines = f.readlines()
    for line in lines:
        total += float(line)
        count += 1
    f.close()

    if(count ==0):
        return 0.0
    else:
        return total/count

#
# Write the bandwidth of each node ID for a specific port number.
#
def write_goodput_results(flows_output_str, goodputs_output_str, result_dir='.'):
    f = open('{}/nodestat_goodput.txt'.format(result_dir), 'w')
    f.write('{}\n'.format(goodputs_output_str))
    f2 = open('{}/nodestat_goodput_flows.txt'.format(result_dir), 'w')
    f2.write('{}\n'.format(flows_output_str))
    f.close()
    f2.close()


if __name__ == "__main__":
    # Read 'Xapps_Ypkts.txt'.
    # X: number of app flows
    # Y: packet sending rate (pkts/s)
    appConfigFile = sys.argv[1]
    result_dir = sys.argv[2]
    acf = open(appConfigFile, 'r')
    array_goodputs = []

    # Check the flowLog of each destination node in appConfigFile.
    lines = acf.readlines()
    for line in lines:
        items = line.split('\t')
        ac_flow = flow.Flow()
        ac_flow.parse('{} {} {}'.format(items[1], items[2], items[3]))
        start_time = float(items[6])
        duration = float(items[7])

        applog_file = '{}/applog_{}-{}.txt'.format(result_dir, ac_flow.dst, ac_flow.dport)
        templog_file = '{}/applog_temp_{}-{}.txt'.format(result_dir, ac_flow.dst, ac_flow.dport)
        stdout_templog = open(templog_file, 'w')
        subprocess.call(['./getAColumn.sh', applog_file, '1'], stdout=stdout_templog)

        goodput = get_average_of_applog(templog_file)
        tuple_goodput = (ac_flow, goodput)
        array_goodputs.append(tuple_goodput)
        #print('Flow {}'.format(ac_flow.to_string()))
        #print(' -Goodput = {}'.format(goodput))

    goodputs_output_str = '{}\t{}'.format(get_app_config(appConfigFile), 'goodput')
    flows_output_str = '{}\t{}'.format(get_app_config(appConfigFile), 'goodput_flows')
    for tuple_goodput in array_goodputs:
        goodputs_output_str += '\t{}'.format(tuple_goodput[1])
        flows_output_str += '\t{}:{}'.format(tuple_goodput[0].dst, tuple_goodput[0].dport)

    # Write goodput result to the file: "nodestat_goodput.txt".
    write_goodput_results(flows_output_str, goodputs_output_str, result_dir)
    #print(goodputs_output_str)
    #print(flows_output_str)
