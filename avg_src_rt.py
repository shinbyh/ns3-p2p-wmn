import sys
import re
import flow
import avg_thp
import commands

if __name__ == "__main__":
    # Read flow configuration file: 'Xapps_Ypkts.txt'.
    # X: number of app flows
    # Y: packet sending rate (pkts/s)
    appConfigFile = sys.argv[1]
    acf = open(appConfigFile, 'r')
    output_str = '{}\t{}'.format(avg_thp.get_app_config(appConfigFile), 'rt_dscv_count')

    # Generate a set of source node IDs from appConfigFile.
    src_set = set([])
    lines = acf.readlines()
    for line in lines:
        items = line.split('\t')
        ac_flow = flow.Flow()
        ac_flow.parse('{} {} {}'.format(items[1], items[2], items[3]))
        ac_src_id = avg_thp.get_nodeid_from_idport(items[1])
        src_set.add(ac_src_id)

    log_file = open('nodestat_src_rt.txt', 'w')
    total_rt_dscv_count = 0
    for src_id in src_set:
        src_rtlog_file = 'srcrtlog_{}.txt'.format(src_id)
        srf = open(src_rtlog_file, 'r')
        srf_lines = srf.readlines()

        rt_dscv_count = 0
        for srf_line in srf_lines:
            items = re.split(r'[\t]', srf_line.strip())
            rt_dscv_count += int(items[1])

        #print('{}\t{}'.format(src_id, rt_dscv_count))
        log_file.write('{}\t{}\n'.format(src_id, rt_dscv_count))
        total_rt_dscv_count += rt_dscv_count

    #print('total\t{}'.format(total_rt_dscv_count))
    log_file.write('total\t{}\n'.format(total_rt_dscv_count))
    log_file.close()
    print('{}\t{}'.format(output_str, total_rt_dscv_count))
