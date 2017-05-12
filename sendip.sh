#!/bin/bash
sudo systemctl restart postfix
echo "subject:$(hostname -I)" | sendmail example@example.com
#~/Wave/wave  // uncomment to run on startup (TODO: fix running multiple times)