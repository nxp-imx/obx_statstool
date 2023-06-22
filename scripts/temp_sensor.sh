# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2023 NXP
#!/bin/bash

hex_temp=$(i2cget -y 12 0x49 0 w)
hex_temp_byte_swapped=$(echo $(( ((hex_temp & 0xFF) << 8) | (hex_temp >> 8) )))
temp=$(echo $((hex_temp_byte_swapped >> 5)))
if [ $((temp & 0x400)) -eq 0 ]
then
        printf "%.3f\n" $(((temp * 125)))e-3
else
        printf "%.3f\n" $((((temp ^ 0x7FF) + 1) * 125))e-3
fi
