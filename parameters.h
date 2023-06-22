/* SPDX-License-Identifier: BSD-3-Clause
 *  Copyright 2023 NXP
 */

#include<stdio.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<stdbool.h>
#include<netdb.h>
#include<time.h>
//MAX_IFACES macro has been used to define an upper limit for interfaces of wi-fi, bluetooth, v2x and ethernet
#define MAX_IFACES 5
//MAXAVAIL macro has been used to define maximum number of devices available for bluetooth connection
#define MAXAVAIL 20
struct wifi {
	char ifname[16], description[24], state[16];
	struct sockaddr_in6 inet6_addr;
	struct sockaddr_in inet4_addr;
	char macaddress[24];
	char uuid[40];
	int frequency, rxbytes, txbytes, rxdropped, rxerrors, txdropped, txerrors, noise, rssi, linkspeed;
	float rxrate, txrate;
	bool up, running, broadcast, multicast;
};

struct available_devices {
	char name[24];
	char address[24];
};

struct paired_device {
	char name[24];
	char address[24];
};

struct ble {
	char iname[8], macaddr[24], class[16], status[16], name[24], type[8];
	int pid;
	int rxbytes, txbytes, rxerrors, txerrors;
	float rxrate, txrate;
	int num_available_devices;
	struct available_devices devices[MAXAVAIL];
	struct paired_device pair;
};

struct eth {
	char ifname[8];
	char macaddr[24];
	int rxbytes, txbytes, rxdropped, rxerrors, txdropped, txerrors;
	float rxrate, txrate;
	bool up, running, multicast, broadcast;
	struct sockaddr_in6 inet6_addr;
	struct sockaddr_in inet4_addr;
};

struct accelerometer {
	float x_axis, y_axis, z_axis;
};

struct gnss {
	char gnrmc[50], gnvtg[50], gngga[50], gngll[50], gpgsv[50], gagsv[50], gbgsv[50], glgsv[50], gngsa1[50], gngsa2[50], gngsa3[50], gngsa4[50], rssi[20];
};

struct vtox_sch {
	char macaddr[24];
	int rxbytes, txbytes;
	struct sockaddr_in6 inet6_addr;
};

struct vtox_cch {
	char macaddr[24];
	int rxbytes, txbytes;
	struct sockaddr_in6 inet6_addr;
};

struct vtox {
	char ifname[8], deviceid[16], interface[8], channel[8], macaddr[24];
	int rxbytes, txbytes, rxdropped, rxerrors, txdropped, txerrors, channelnum, frequency, rssi1, rssi2, noise1, noise2;
	float rxrate, txrate, cputemp, tempant1, tempant2;
	struct vtox_sch sch;
	struct vtox_cch cch;
};

struct orange_box_stats {
	struct accelerometer acc;
	int num_ble_ifaces;
	struct ble bluetooth[MAX_IFACES];
	struct gnss gps;
	int num_eth_ifaces;
	struct eth eth_faces[MAX_IFACES];
	int num_wifi_ifaces;
	struct wifi wifi[MAX_IFACES];
	int num_vtox_ifaces;
	struct vtox vtox[MAX_IFACES];
	float temp_sense;
	clock_t time_stamp;
};
