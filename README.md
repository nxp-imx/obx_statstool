<License: SPDX-License-Identifier: BSD-3-Clause
Copyright 2023 NXP>
 
# OrangeBox Stats Tool
The OrangeBox Stats Tool is a customized command line interface tool designed to display the parameters of services like WiFi, Bluetooth, GNSS, V2X, Accelerometer, Temperature and Ethernet switches present on the OrangeBox Automotive Connectivity Domain Controller Development Platform.

## Description
The command line interface tool should capture information about V2X, WiFi, Bluetooth, accelerometer, gps, temperature and ethernet switches SJA1110D and print it in user space. There are preexisting cli tools present on the Orange Box such as wpa cli and bluetoothctl which help in capturing information about Wi-Fi and Bluetooth respectively. This cli tool will use the present existing tools and provide the unified statistics at any instance to the user. If no existing cli tool or system command exists, then IOCTL calls will be used.

C Files:
* obx_system_stats_d.c
* obx_stats_cli.c

Header Files:
* parameters.h

Script Files:
* gnss.sh
* bluetooth.sh
* gnss_rssi.sh
* temp_sensor.sh

## Getting Started
### Dependencies
=============================
* 5G modem card for Wi-Fi and Bluetooth and V2X card for V2X communication
* Modprobe moal module for WiFi activation
* Pulseaudio package and modprobe moal module for Bluetooth
* V2X modules present

### Build and Installation
=============================
* obx_statstool can be compiled on orangebox board itself or cross-compiled on host machine and then executables can be transferred to Orangebox board.

* Build and Installation steps for compiling on Orangebox board
---------------------------------------------------------------
* Copy/Get/Clone the obx_statstool source code folder with (C files, header file and scripts) on OrangeBox board.
* Change directory to this obx_statstool folder on Orangebox board.
~~~
	# cd obx_statstool/
~~~
* Compile using gcc toolchian:
~~~
	# gcc -o obx_system_stats_d obx_system_stats_d.c
	# gcc -o obx_stats_cli obx_stats_cli.c
~~~
* Ensure all scripts in script folder are in execution mode (Use chmod +x _filename_ to give execution access).
~~~
	# ls scripts/
	# chmod +x scripts/*
~~~

### Execution
=============================

On reboot, before autobooting begins (timer of 3 seconds is given), press Enter and give the following commands
~~~
	# setenv fdt_file imx8dxl-orangebox-sd.dtb; saveenv
	# boot
~~~
After booting, follwowing commands need to be run
~~~
	# cd /usr/bin
	# sxf1800_check --update
	# llc chconfig -s -c 176
	# ifconfig llc-cch-ipv6 fc00::1/64 up
	# ifconfig llc-sch-ipv6 fc00::1/64 up
~~~
* Ensure /usr/bin/sxf1800/v2xApplet_2.12.3_NCK.apdu exists
* Ensure /usr/bin/sxf1800/v2xscppalutil.bin exists

* Change directory to the obx_statstool folder
~~~
	# cd obx_statstool/
~~~

Start the WiFi service
* Ensure /etc/wpa_supplicant/wpa_supplicant-mlan0.conf exists
* Ensure /etc/systemd/network/25-wlan.network exists
~~~
	# systemctl enable wpa_supplicant@mlan0.service
	# systemctl restart systemd-networkd.service
	# systemctl restart wpa_supplicant@mlan0.service
	# modprobe moal
	# systemctl start wpa_supplicant@mlan0.service
~~~
load bluetotth module and run the Bluetooth script File
~~~
	# modprobe btnxpuart
	# ./scripts/bluetooth.sh
~~~
Load the accelerometer modules (if not already done).
~~~
	# insmod /lib/modules/6.1.1+g7dea0c7bec6b/kernel/drivers/iio/accel/fxls8962af-core.ko
	# insmod /lib/modules/6.1.1+g7dea0c7bec6b/kernel/drivers/iio/accel/fxls8962af-i2c.ko
~~~
Run the GNSS script file
~~~
	# ./scripts/gnss.sh
	# stty -F /dev/ttyLP3 38400
~~~
The daemon server executes in the background while the client keeps querying the server.
~~~
	# obx_system_stats_d &
	# obx_stats_cli
	>obox_cli -d (cli options for dynamic stats display)
	>obox_cli -h (cli options to display help)
~~~

### To Do
* Take care of different outputs of different OBX boards
* De-init for all modules

## Version History
* v1.0
* This version is compatible with i.MX Matter 2023Q3 release (base on i.MX Yocto 6.1.36-2.1.0)
