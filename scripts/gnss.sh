# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2023 NXP
#!/bin/bash

cd `dirname $0`

dev=/dev/ttyLP3

ANT_EN="\xb5\x62\x06\x8a\x1d\x00\x00\x01\x00\x00\x2e\x00\xa3\x10\x01\x2f\x00\xa3\x10\x01\x31\x00\xa3\x10\x01\x33\x00\xa3\x10\x01\x35\x00\xa3\x10\x01\x28\x0f"

stty -F $dev 38400 raw -echo

echo -ne $ANT_EN > $dev

#socat tcp-l:8889,reuseaddr,fork file:/dev/ttyLP3,b38400,raw,nonblock,waitlock=/var/run/ttyLP3.lock
