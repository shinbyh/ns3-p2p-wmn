######################################################
#
# Incrememtal random flows generator.
# This code refers to all "20apps" config files.
# Then it creates smaller flow sets from 10 to 19 flows.
# If there is '20apps_250pkts-0.txt',
# then it will create '10apps_250pkts-0.txt', '11apps_250pkts-0.txt', ...,
# and '19apps_250pkts-0.txt' respectively.
#
######################################################
import sys

config_dir = 'apps_config'

def generate_flows_incremental(max_num_of_apps, num_of_flowsets):
    # Index from 0 to 49
    for n in range(0, num_of_flowsets):
        # A base config file name to refer to.
        config_file_n = '{}/{}apps_250pkts-{}.txt'.format(config_dir, max_num_of_apps, n)

        f = open(config_file_n, 'r')
        lines = f.readlines()
        if(len(lines) >= max_num_of_apps):
            for flows in range(1, max_num_of_apps):
                # Create a new config file.
                new_config_file = '{}/{}apps_250pkts-{}.txt'.format(config_dir, flows, n)
                fw = open(new_config_file, 'w')

                line_count = 0
                for line in lines:
                    # Check if line_count smaller than flows.
                    if(line_count >= flows):
                        continue

                    # Copy string and write to a new file.
                    fw.write(line)
                    line_count += 1
                fw.close()
        f.close()

if __name__ == "__main__":
    max_num_of_apps = int(sys.argv[1])
    num_of_flowsets = int(sys.argv[2])
    generate_flows_incremental(max_num_of_apps, num_of_flowsets)
