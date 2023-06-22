# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2023 NXP
#!/bin/bash 
cd `dirname $0` 
pulseaudio -k
pulseaudio --start --log-target=syslog
# paplay -vvv /unit_tests/ASRC/audio8k16S.wav 

# the module below is loaded in /etc/modules-load.d/moal.conf
# modprobe moal mod_para=nxp/wifi_mod_para.conf 
hciattach /dev/ttyLP1 any -s 3000000 3000000 flow
sleep 1
hciconfig hci0 up
hciconfig hci0 piscan
hciconfig hci0 noencrypt
hcitool dev
