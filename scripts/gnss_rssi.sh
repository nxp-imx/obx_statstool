# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2023 NXP
#!/bin/bash
cd `dirname $0`

display_rssi() {
        cat gnsslogfile | grep GSV | sed 's/,,/,0,/g' | rev | cut --complement -d"," -f1 | rev | while read i
        do
                IFS=',' read -r -a array <<< $i
                for (( j=4; j<$(( ${#array[@]} - 1 )); j=j+4 ))
                do
                        echo -n "${array[0]:1}, " | sed 's/GSV//g'
                        echo -n "${array[j]}, "
                        echo "${array[j+3]}"
                done
        done
}

