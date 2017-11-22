#!/bin/bash

cd /home/bhshin/mercurial/ns-allinone-3.26/ns-3.26
REMOTE_DIR="/home/cdsn/repos/ns-allinone-3.26/ns-3.26/"

rsync -av -e "ssh -p 20006" *.sh cdsn@143.248.53.233:$REMOTE_DIR
rsync -av -e "ssh -p 20006" flow.py cdsn@143.248.53.233:$REMOTE_DIR
rsync -av -e "ssh -p 20006" stat.py cdsn@143.248.53.233:$REMOTE_DIR
rsync -av -e "ssh -p 20006" avg_thp.py cdsn@143.248.53.233:$REMOTE_DIR
rsync -av -e "ssh -p 20006" *apps_*.txt cdsn@143.248.53.233:$REMOTE_DIR

