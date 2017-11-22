#!/bin/bash

WEBHOOK="https://hooks.slack.com/services/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"
msg=$1

echo "Sending WebHook msg to Slack: "$msg

curl -X POST \
 -H 'Content-Type:application/json' \
 --data '{"text":"'"$msg"'"}' $WEBHOOK

