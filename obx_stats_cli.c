// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2023 NXP

#include<sys/socket.h>
#include<netdb.h>
#include<string.h>
#include<stdlib.h>
#include<stdio.h>
#include<unistd.h>
#include "parameters.h"
#include<stdbool.h>
#include<arpa/inet.h>
//clear macro has been used to reset the screen to normal (white) font
#define clear() printf("\033[H\033[J")
//set_cursor macro takes x and y point values and prints the given text at the given coordinates
#define set_cursor(x, y) printf("\033[%d;%dH", (y), (x))
//MAX macro has been used to define the maximum input buffer that is used to send as a request to daemon server
#define MAX 80
//PORT macro has been used to define the port number for socket address for communication with server
#define PORT 8080
//The colour macros have been defined to set different colours for different parameters in dynamic mode
#define SA struct sockaddr
#define NORMAL  "\x1B[0m"
#define RED  "\x1B[31m"
#define GREEN  "\x1B[32m"
#define YELLOW  "\x1B[33m"
#define MAGENTA  "\x1B[35m"
#define CYAN  "\x1B[36m"
#define ORANGE "\033[39m"
#define BLUE "\x1B[34m"
char buff[MAX], dynamic_buff[MAX];
int sockfd, len, n;
struct sockaddr_in servaddr;
struct orange_box_stats obox;
void display_acc_stats(int lvl)
{
	printf("\nAccelerometer: ");
	if (lvl > 1) {
		printf("\nThe accelermeter connected to the i.MX8DXL chip: FXLS8967\nProtocol used: I2C");
		printf("\nFXLS8967AF is a compact 3-axis MEMS accelerometer designed for use in a wide range of automotive security and convenience applications that require ultra-low-power wakeup on motion.\nThe part supports both high-performance and low-power operating modes, allowing maximum flexibility to meet the resolution and power needs for various unique use cases.");
	}
	printf("\n\tX axis: %f", obox.acc.x_axis);
	if (lvl > 2)
		printf("\n\tThis indicates the left-right orientation of Orangebox");
	printf("\n\tY axis: %f", obox.acc.y_axis);
	if (lvl > 2)
		printf("\n\tThis indicates the front-back orientation of Orangebox");
	printf("\n\tZ axis: %f", obox.acc.z_axis);
	if (lvl > 2)
		printf("\n\tThis indicates the top-down orientation of Orangebox");
}

void display_ble_stats(int lvl)
{
	printf("\nBluetooth: ");
	if (lvl > 1) {
		printf("\nThe Bluetooth Chip connected to the i.MX8DXL chip: 88Q9098\nProtocol used: PCIe 3.0");
		printf("\nThe 88Q9098 family of automotive wireless SoCs is the industry\'s first Wi-Fi 6 solution based on the latest IEEE 802.11ax standards supporting 2x2 plus 2x2 concurrent dual Wi-Fi and dual-mode Bluetooth 5.3.");
	}
	printf("\nNumber of Bluetooth controllers: %d", obox.num_ble_ifaces);
	int x = obox.num_ble_ifaces;

	for (int i = 0; i < x; i++) {
		printf("\nController %d", i+1);
		printf("\n\tInterface Name: %s", obox.bluetooth[i].iname);
		if (lvl > 2)
			printf("\n\tThis indicates the bluetooth controller name");
		if (lvl > 1) {
			printf("\n\tMAC Address: %s", obox.bluetooth[i].macaddr);
			printf("\tClass: %s", obox.bluetooth[i].class);
			if (lvl > 2)
				printf("\tThis indicates the class address to which controller belongs to");
			printf("\n\tStatus: %s", obox.bluetooth[i].status);
			if (lvl > 2)
				printf("\tThis indicates the status of bluetooth service");
			printf("\n\tName: %s", obox.bluetooth[i].name);
			if (lvl > 2)
				printf("\n\tThis indicates the name of device");
			printf("\n\tBus Type: %s", obox.bluetooth[i].type);
			if (lvl > 2)
				printf("\tThis indicates the communication protocol followed by bus");
			printf("\n\tProcess ID: %d", obox.bluetooth[i].pid);
		}
		printf("\n\tRX bytes: %d", obox.bluetooth[i].rxbytes);
		if (lvl > 2)
			printf("\n\tThis indicates the number of received bytes");
		printf("\n\tTX bytes: %d", obox.bluetooth[i].txbytes);
		if (lvl > 2)
			printf("\n\tThis indicates the number of transmitted bytes");
		printf("\n\tRX errors: %d", obox.bluetooth[i].rxerrors);
		if (lvl > 2)
			printf("\n\tThis indicates the number of received packet errors");
		printf("\n\tTX errors: %d", obox.bluetooth[i].txerrors);
		if (lvl > 2)
			printf("\n\tThis indicates the number of transmitted packet errors");
		printf("\n\tRX rate: %f", obox.bluetooth[i].rxrate);
		if (lvl > 2)
			printf("\n\tThis indicates the number of received kilobytes per second");
		printf("\n\tTX rate: %f", obox.bluetooth[i].txrate);
		if (lvl > 2)
			printf("\n\tThis indicates the number of transmitted kilobytes per second");
		if (lvl > 2) {
			printf("\n\tNumber of available devices: %d", obox.bluetooth[i].num_available_devices);
			printf("\n\tThis indicates the name and addresses of bluetooth enabled devices that are available to pair");
			printf("\n\tAvailable Devices: ");
			for (int j = 0; j < obox.bluetooth[i].num_available_devices; j++)
				printf("\n\tDevice Name: %s   \tMAC Address: %s", obox.bluetooth[i].devices[j].name, obox.bluetooth[i].devices[j].address);
			printf("\n\tPaired Device: ");
			printf("\n\tThis indicates the name and address of the paired device");
			printf("\n\tDevice Name: %s   MAC Address: %s", obox.bluetooth[i].pair.name, obox.bluetooth[i].pair.address);
		}
	}
}

void display_eth_stats(int lvl, int interface)
{
	printf("\nEthernet Switches: ");
	if (lvl > 1) {
		printf("\nThe Ethernet Switch connected to the i.MX8DXL chip: SJA0111D\nProtocol used: RGMII");
		printf("\nSJA1110 is a family of four pin-compatible and software-compatible automotive Ethernet Switch SoC offering a scalable solution for all automotive applications.");
		printf("\nSJA1110 is further optimized to work with the S32G vehicle networking processor to enable distributed firewall and Intrusion-Detection/Prevention Systems (IDPS) reaching unmatched levels of scalability and performance.");
	}
	printf("\nNumber of Ethernet switches: %d", obox.num_eth_ifaces);
	int x = obox.num_eth_ifaces, start = 0;
	char buffer4[INET_ADDRSTRLEN], buffer6[INET6_ADDRSTRLEN];

	if (interface > -1) {
		start = interface;
		x = interface + 1;
	}

	for (int i = start; i < x; i++) {
		printf("\nSwitch %d", i+1);
		printf("\n\tInterface Name: %s", obox.eth_faces[i].ifname);
		if (lvl > 2)
			printf("\n\tThis indicates the name of the ethernet interface");
		memset(&buffer4, '\0', sizeof(buffer4));
		inet_ntop(AF_INET, &obox.eth_faces[i].inet4_addr, buffer4, INET_ADDRSTRLEN);
		printf("\n\tIPv4 address: %s", buffer4);
		memset(&buffer6, '\0', sizeof(buffer6));
		inet_ntop(AF_INET6, &obox.eth_faces[i].inet6_addr, buffer6, INET6_ADDRSTRLEN);
		printf("\n\tIPv6 address: %s", buffer6);
		printf("\n\tMAC address: %s", obox.eth_faces[i].macaddr);
		if (lvl > 1) {
			if (obox.eth_faces[i].up)
				printf("\n\tInterface UP");
			else
				printf("\n\tInterface DOWN");
			if (obox.eth_faces[i].running)
				printf("\n\tInterface RUNNING: True");
			else
				printf("\n\tInterface RUNNING: False");
			if (obox.eth_faces[i].multicast)
				printf("\n\tMulticasting: Enabled");
			else
				printf("\n\tMulticasting: Disabled");
			if (obox.eth_faces[i].broadcast)
				printf("\n\tBroadcasting: Enabled");
			else
				printf("\n\tBroadcasting: Disabled");
		}
		printf("\n\tRX bytes: %d", obox.eth_faces[i].rxbytes);
		if (lvl > 2)
			printf("\n\tThis indicates the number of received bytes");
		printf("\n\tTX bytes: %d", obox.eth_faces[i].txbytes);
		if (lvl > 2)
			printf("\n\tThis indicates the number of transmitted bytes");
		printf("\n\tRX dropped: %d", obox.eth_faces[i].rxdropped);
		if (lvl > 2)
			printf("\n\tThis indicates the number of dropped received packets");
		printf("\n\tRX errors: %d", obox.eth_faces[i].rxerrors);
		if (lvl > 2)
			printf("\n\tThis indicates the number of packet errors in received packets");
		printf("\n\tTX dropped: %d", obox.eth_faces[i].txdropped);
		if (lvl > 2)
			printf("\n\tThis indicates the number of dropped transmitted packets");
		printf("\n\tTX errors: %d", obox.eth_faces[i].txerrors);
		if (lvl > 2)
			printf("\n\tThis indicates the number of packet errors in transmitted packets");
		printf("\n\tRX rate: %f", obox.eth_faces[i].rxrate);
		if (lvl > 2)
			printf("\n\tThis indicates the rate of received bytes in kBps");
		printf("\n\tTX rate: %f", obox.eth_faces[i].txrate);
		if (lvl > 2)
			printf("\n\tThis indicates the rate of transmitted bytes in kBps");
		printf("\n");
	}
}

void display_gps_stats(int lvl)
{
	printf("\nGlobal Navigation Satellite System: ");
	if (lvl > 1) {
		printf("\nThe navigation system connected to the i.MX8DXL chip: MOSAIC-X5 or ZED-F9K\nProtocol used: UART");
		printf("\nThis describes any satellite constellation that provides positioning, navigation, and timing (PNT) services on a global or regional basis.");
	}
	printf("\n\tGNGGA  : %s", obox.gps.gngga);
	if (lvl > 2)
		printf("\tThis refers to the Global Positioning System Fix Data");
	printf("\n\tGNGSA 1: %s", obox.gps.gngsa1);
	if (lvl > 2)
		printf("\tThis refers to the GNSS DOP and Active Satellite 1");
	printf("\n\tGNGSA 2: %s", obox.gps.gngsa2);
	if (lvl > 2)
		printf("\tThis refers to the GNSS DOP and Active Satellite 2");
	printf("\n\tGNGSA 3: %s", obox.gps.gngsa3);
	if (lvl > 2)
		printf("\tThis refers to the GNSS DOP and Active Satellite 3");
	printf("\n\tGNGSA 4: %s", obox.gps.gngsa4);
	if (lvl > 2)
		printf("\tThis refers to the GNSS DOP and Active Satellite 4");
	printf("\n\tGNGLL  : %s", obox.gps.gngll);
	if (lvl > 2)
		printf("\tThis refers to the  latitude and longitude of present vessel position, time of position fix and status");
	printf("\n\tGPGSV  : %s", obox.gps.gpgsv);
	if (lvl > 2)
		printf("\tThis refers to the GNSS Satellites in View - GPS only Solutions");
	printf("\n\tGAGSV  : %s", obox.gps.gagsv);
	if (lvl > 2)
		printf("\tThis refers to the GNSS Satellites in View - Galileo only Solutions");
	printf("\n\tGBGSV  : %s", obox.gps.gbgsv);
	if (lvl > 2)
		printf("\tThis refers to the GNSS Satellites in View - BeiDou only Solutions");
	printf("\n\tGLGSV  : %s", obox.gps.glgsv);
	if (lvl > 2)
		printf("\tThis refers to the GNSS Satellites in View - GLONASS only Solutions");
	printf("\n\tGNRMC  : %s", obox.gps.gnrmc);
	if (lvl > 2)
		printf("\tThis refers to the Recommended Minimum Specific GNSS Data");
	printf("\n\tGNVTG  : %s", obox.gps.gnvtg);
	if (lvl > 2)
		printf("\tThis refers to the Course over Ground and Ground Speed");
	printf("\n\tRSSI   : %s", obox.gps.rssi);
	if (lvl > 2)
		printf("\n\t This indicates the GNSS Talker ID, Satellite ID and Signal Strength");
}

void display_wifi_stats(int lvl)
{
	printf("\nWiFi: ");
	if (lvl > 1) {
		printf("\nThe Wireless Fidelity Chip connected to the i.MX8DXL chip: 88Q9098\nProtocol used: PCIe 3.0");
		printf("\nThe 88Q9098 family of automotive wireless SoCs is the industry\'s first Wi-Fi 6 solution based on the latest IEEE 802.11ax standards supporting 2x2 plus 2x2 concurrent dual Wi-Fi and dual-mode Bluetooth 5.3.");
	}
	printf("\nNumber of WiFi controllers: %d", obox.num_wifi_ifaces);
	int x = obox.num_wifi_ifaces;
	char buffer4[INET_ADDRSTRLEN], buffer6[INET6_ADDRSTRLEN];

	for (int i = 0; i < x; i++) {
		printf("\nController %d", i+1);
		printf("\n\tInterface Name: %s", obox.wifi[i].ifname);
		if (lvl > 2)
			printf("\n\tThis indicates the name of the connected interface");
		if (lvl > 1) {
			printf("\n\tDescription: %s", obox.wifi[i].description);
			printf("\n\tState: %s", obox.wifi[i].state);
			if (lvl > 2)
				printf("\tThis indicates the results of WPA supplicant scan");
		}
		memset(&buffer4, '\0', sizeof(buffer4));
		inet_ntop(AF_INET, &obox.wifi[i].inet4_addr, buffer4, INET_ADDRSTRLEN);
		printf("\n\tIPv4 address: %s", buffer4);
		memset(&buffer6, '\0', sizeof(buffer6));
		inet_ntop(AF_INET6, &obox.wifi[i].inet6_addr, buffer6, INET6_ADDRSTRLEN);
		printf("\n\tIPv6 address: %s", buffer6);
		printf("\n\tMAC address: %s", obox.wifi[i].macaddress);
		if (lvl > 1) {
			printf("\tUnique Identifier: %s", obox.wifi[i].uuid);
			if (obox.wifi[i].up)
				printf("\n\tInterface UP");
			else
				printf("\n\tInterface DOWN");
			if (obox.wifi[i].running)
				printf("\n\tInterface RUNNING: True");
			else
				printf("\n\tInterface RUNNING: False");
			if (obox.wifi[i].multicast)
				printf("\n\tMulticasting: Enabled");
			else
				printf("\n\tMulticasting: Disabled");
			if (obox.wifi[i].broadcast)
				printf("\n\tBroadcasting: Enabled");
			else
				printf("\n\tBroadcasting: Disabled");
		}
		printf("\n\tRX bytes: %d", obox.wifi[i].rxbytes);
		if (lvl > 2)
			printf("\n\tThis indicates the number of received bytes");
		printf("\n\tTX bytes: %d", obox.wifi[i].txbytes);
		if (lvl > 2)
			printf("\n\tThis indicates the number of transmitted bytes");
		printf("\n\tRX dropped: %d", obox.wifi[i].rxdropped);
		if (lvl > 2)
			printf("\n\tThis indicates the number of dropped received packets");
		printf("\n\tRX errors: %d", obox.wifi[i].rxerrors);
		if (lvl > 2)
			printf("\n\tThis indicates the number of errors in received packets");
		printf("\n\tTX dropped: %d", obox.wifi[i].txdropped);
		if (lvl > 2)
			printf("\n\tThis indicates the number of dropped transmitted packets");
		printf("\n\tTX errors: %d", obox.wifi[i].txerrors);
		if (lvl > 2)
			printf("\n\tThis indicates the number of errors in transmitted packets");
		printf("\n\tFrequency: %d", obox.wifi[i].frequency);
		if (lvl > 2)
			printf("\n\tThis indicates the frequency of connected interface's signal in Mega Hertz");
		printf("\n\tNoise: %d", obox.wifi[i].noise);
		if (lvl > 2)
			printf("\n\tThis indicates the energy interference in dBm format");
		printf("\n\tRSSI: %d", obox.wifi[i].rssi);
		if (lvl > 2)
			printf("\n\tThis indicates the value of Received Signal Strength Indicator");
		printf("\n\tLinkspeed: %d", obox.wifi[i].linkspeed);
		if (lvl > 2)
			printf("\n\tThis indicates the maximum speed in bits/second this device can communicate with the device it is linked to");
		printf("\n\tRX rate: %f", obox.wifi[i].rxrate);
		if (lvl > 2)
			printf("\n\tThis indicates the number of kbytes received per second");
		printf("\n\tTX rate: %f", obox.wifi[i].txrate);
		if (lvl > 2)
			printf("\n\tThis indicates the number of kbytes transmitted per second");
	}
}

void display_v2x_stats(int lvl)
{
	printf("\nV2X: ");
	if (lvl > 1) {
		printf("\nVehicle-to-Everything (V2X) technology enables cars to communicate with their surroundings and makes driving safer and more efficient for everyone.");
		printf("\nThere are 2 supported devices for the same: SAF5400 (Single Chip Modem) and SXF1800 (Secure Element)");
	}
	printf("\nNumber of V2X controllers: %d", obox.num_vtox_ifaces);
	int x = obox.num_vtox_ifaces;
	char buffer6[INET6_ADDRSTRLEN];

	for (int i = 0; i < x; i++) {
		printf("\nInterface %d", i);
		printf("\n\tInterface Name: %s", obox.vtox[i].ifname);
		if (lvl > 2)
			printf("\n\tThis indicates the name of the connected interface");
		if (lvl > 1) {
			printf("\n\tDevice ID: %s", obox.vtox[i].deviceid);
			if (lvl > 2)
				printf("\tThis describes the chip name of the primary SAF firmware");
			printf("\n\tInterface: %s", obox.vtox[i].interface);
			if (lvl > 2)
				printf("\tThis indicates the protocol used for communication between i.MX chip and V2X chip");
			printf("\n\tChannel: %s", obox.vtox[i].channel);
			if (lvl > 2)
				printf("\tThis describes the channel used for communication in primary radio");
		}
		printf("\n\tMAC address: %s", obox.vtox[i].macaddr);
		printf("\n\tRX bytes: %d", obox.vtox[i].rxbytes);
		if (lvl > 2)
			printf("\n\tThis indicates the number of received bytes");
		printf("\n\tTX bytes: %d", obox.vtox[i].txbytes);
		if (lvl > 2)
			printf("\n\tThis indicates the number of transmitted bytes");
		printf("\n\tRX dropped: %d", obox.vtox[i].rxdropped);
		if (lvl > 2)
			printf("\n\tThis indicates the number of dropped received packets");
		printf("\n\tRX errors: %d", obox.vtox[i].rxerrors);
		if (lvl > 2)
			printf("\n\tThis indicates the number of errors in received packets");
		printf("\n\tTX dropped: %d", obox.vtox[i].txdropped);
		if (lvl > 2)
			printf("\n\tThis indicates the number of dropped transmitted packets");
		printf("\n\tTX errors: %d", obox.vtox[i].txerrors);
		if (lvl > 2)
			printf("\n\tThis indicates the number of errors in transmitted packets");
		if (lvl > 1) {
			if ((obox.vtox[i].channelnum > 167) && (obox.vtox[i].channelnum < 185)) {
				//Valid channel number for V2X communication lies between [168, 184]
				//If channel config is not set, erroneous channel numbers can be generated which shouldn't be displayed.
				printf("\n\tChannel number: %d", obox.vtox[i].channelnum);
				if (lvl > 2)
					printf("\n\tThis indicates the channel number used in primary radio");
			}
			printf("\n\tFrequency: %d", obox.vtox[i].frequency);
			if (lvl > 2)
				printf("\n\tThis indicates the Frequency of signal FCS Pass value in communication with other V2X enabled board");
			printf("\n\tRSSI Antenna 1: %d", obox.vtox[i].rssi1);
			if (lvl > 2)
				printf("\n\tThis indicates the RSSI of signal FCS Pass value (Antenna 1) in communication with other V2X enabled board");
			printf("\n\tRSSI Antenna 2: %d", obox.vtox[i].rssi2);
			if (lvl > 2)
				printf("\n\tThis indicates the RSSI of signal FCS Pass value (Antenna 2) in communication with other V2X enabled board");
			printf("\n\tNoise Antenna 1: %d", obox.vtox[i].noise1);
			if (lvl > 2)
				printf("\n\tThis indicates the Noise of signal FCS Pass value (Antenna 1) in communication with other V2X enabled board");
			printf("\n\tNoise Antenna 1: %d", obox.vtox[i].noise2);
			if (lvl > 2)
				printf("\n\tThis indicates the Noise of signal FCS Pass value (Antenna 2) in communication with other V2X enabled board");
		}
		printf("\n\tRX Rate in kBps: %f", obox.vtox[i].rxrate);
		if (lvl > 2)
			printf("\n\tThis indicates the number of kbytes received per second");
		printf("\n\tTX Rate in kBps: %f", obox.vtox[i].txrate);
		if (lvl > 2)
			printf("\n\tThis indicates the number of kbytes transmitted per second");
		printf("\n\tCPU Temperature in °C: %f", obox.vtox[i].cputemp);
		if (lvl > 1) {
			printf("\n\tAntenna 1 Temperature in °C: %f", obox.vtox[i].tempant1);
			printf("\n\tAntenna 2 Temperature in °C: %f", obox.vtox[i].tempant2);
			printf("\n\n\tIEEE 802.11P supports multichannel communication. V2X uses two channels namely SCH and CCH");
			printf("\n\n\tService Channel (SCH) Interval is used for two way communication for specific applications. Following are the values of SCH interval: ");
			printf("\n\t\tMAC address: %s", obox.vtox[i].sch.macaddr);
			printf("\n\t\tRX Bytes: %d", obox.vtox[i].sch.rxbytes);
			printf("\n\t\tTX Bytes: %d", obox.vtox[i].sch.txbytes);
			memset(&buffer6, '\0', sizeof(buffer6));
			inet_ntop(AF_INET6, &obox.vtox[i].sch.inet6_addr, buffer6, INET6_ADDRSTRLEN);
			printf("\n\t\tIPv6 address: %s", buffer6);
			printf("\n\n\tControl Channel (CCH) Interval is used for broadcast communication; dedicated to short, high priority, data and management frames. Following are the values of CCH interval: ");
			printf("\n\t\tMAC address: %s", obox.vtox[i].cch.macaddr);
			printf("\n\t\tRX Bytes: %d", obox.vtox[i].cch.rxbytes);
			printf("\n\t\tTX Bytes: %d", obox.vtox[i].cch.txbytes);
			memset(&buffer6, '\0', sizeof(buffer6));
			inet_ntop(AF_INET6, &obox.vtox[i].cch.inet6_addr, buffer6, INET6_ADDRSTRLEN);
			printf("\n\t\tIPv6 address: %s", buffer6);
		}
	}
}

void display_temp_stats(int lvl)
{
	printf("\nTemperature: ");
	if (lvl > 1)
		printf("\nThe temperature Sensor connected to I2C Bus: PCT2075GV\nProtocol used: I2C");
	printf("\n\tTemperature Sensor: %f °C", obox.temp_sense);
	if (lvl > 2)
		printf("\nThis indicates the values of bit manipulated temperature sensor on I2C bus");
}

void display_help(void)
{
	printf("\nSyntax: obox_cli [-options] [verbosity level]\nThe options stand for the services whose stats can be displayed. The available options are as follows:\n-w : Wi-Fi parameters\n-b : Bluetooth parameters\n-g : GPS parameters\n-a : Accelerometer parameters\n-s : Ethernet switches parameters\n-v: V2X parameters\n-t: Temperature sensor\n-h : Help manual\n-d: Dynamic view\nThese options can be further viewed in verbosity levels of 1,2 and 3 with 1 being the brief description and 3 being the most detailed description of the selected option.");
}

/*      dynamic_view() function works like htop command in linux
 *      It displays all radio frequency parameters on a single screen and updates repeatedly
 */
void dynamic_view(void)
{
	//Sends a character to server and receives updated obox structure infinitely.
	//The user cannot exit the dynamic mode after entering it
	while (1) {
		strcpy(dynamic_buff, "d");
		sendto(sockfd, dynamic_buff, sizeof(dynamic_buff), 0, (SA *)&servaddr, len);
		recvfrom(sockfd, &obox, sizeof(obox), 0, (SA *)&servaddr, &len);
		clear();
		set_cursor(1, 1);
		printf("\n%sService          \t | \tParameter                                       | \tValue", NORMAL);
		//Dynamic view of Accelerometer
		set_cursor(1, 2);
		printf("\n%s/dev/accel       \t | \tX axis                                          | \t %f  \t\tm/s²", RED, obox.acc.x_axis);
		set_cursor(1, 3);
		printf("\n%s/dev/accel       \t | \tY axis                                          | \t %f  \t\tm/s²", RED, obox.acc.y_axis);
		set_cursor(1, 4);
		printf("\n%s/dev/accel       \t | \tZ axis                                          | \t %f  \t\tm/s²", RED, obox.acc.z_axis);
		//Dynamic view of Bluetooth
		set_cursor(1, 5);
		printf("\n%sBluetooth        \t | \tNumber of interfaces                            | \t %d", GREEN, obox.num_ble_ifaces);
		int y = 6;

		for (int i = 0; i < obox.num_ble_ifaces; i++) {
			set_cursor(1, y++);
			printf("\n%s%s               \t | \tRX Bytes                                        | \t %d \t\tBytes", GREEN, obox.bluetooth[i].iname, obox.bluetooth[i].rxbytes);
			set_cursor(1, y++);
			printf("\n%s%s               \t | \tTX Bytes                                        | \t %d \t\tBytes", GREEN, obox.bluetooth[i].iname, obox.bluetooth[i].txbytes);
			set_cursor(1, y++);
			printf("\n%s%s               \t | \tRX Errors                                       | \t %d \t\tPackets", GREEN, obox.bluetooth[i].iname, obox.bluetooth[i].rxerrors);
			set_cursor(1, y++);
			printf("\n%s%s               \t | \tTX Errors                                       | \t %d \t\tPackets", GREEN, obox.bluetooth[i].iname, obox.bluetooth[i].txerrors);
			set_cursor(1, y++);
			printf("\n%s%s               \t | \tRX Rate                                         | \t %f \tkB/s", GREEN, obox.bluetooth[i].iname, obox.bluetooth[i].rxrate);
			set_cursor(1, y++);
			printf("\n%s%s               \t | \tTX Rate                                         | \t %f \tkB/s", GREEN, obox.bluetooth[i].iname, obox.bluetooth[i].txrate);
		}
		//Dynamic view of GNSS RSSI
		set_cursor(1, y++);
		printf("\n%sGNSS               \t | \tRSSI (Talker ID, Satellite ID, Signal Strength) | \t %s", BLUE, obox.gps.rssi);

		//Dynamic view of Eth
		set_cursor(1, y++);
		printf("\n%sEthernet Interfaces\t | \tNumber of interfaces                            | \t %d", MAGENTA, obox.num_eth_ifaces);
		for (int i = 0; i < obox.num_eth_ifaces; i++) {
			set_cursor(1, y++);
			if (obox.eth_faces[i].rxbytes == 0)
				printf("\n%s%s             \t | \tRX Bytes                                        | \t %d \t\t Bytes", MAGENTA, obox.eth_faces[i].ifname, obox.eth_faces[i].rxbytes);
			else
				printf("\n%s%s             \t | \tRX Bytes                                        | \t %d \t Bytes", MAGENTA, obox.eth_faces[i].ifname, obox.eth_faces[i].rxbytes);
			set_cursor(1, y++);
			printf("\n%s%s             \t | \tTX Bytes                                        | \t %d \t\t Bytes", MAGENTA, obox.eth_faces[i].ifname, obox.eth_faces[i].txbytes);
			set_cursor(1, y++);
			printf("\n%s%s             \t | \tRX Dropped                                      | \t %d \t\t Packets", MAGENTA, obox.eth_faces[i].ifname, obox.eth_faces[i].rxdropped);
			set_cursor(1, y++);
			printf("\n%s%s             \t | \tRX Errors                                       | \t %d \t\t Packets", MAGENTA, obox.eth_faces[i].ifname, obox.eth_faces[i].rxerrors);
			set_cursor(1, y++);
			printf("\n%s%s             \t | \tTX Dropped                                      | \t %d \t\t Packets", MAGENTA, obox.eth_faces[i].ifname, obox.eth_faces[i].txdropped);
			set_cursor(1, y++);
			printf("\n%s%s             \t | \tTX Errors                                       | \t %d \t\t Packets", MAGENTA, obox.eth_faces[i].ifname, obox.eth_faces[i].txerrors);
			set_cursor(1, y++);
			printf("\n%s%s             \t | \tRX Rate                                         | \t %f \t kB/s", MAGENTA, obox.eth_faces[i].ifname, obox.eth_faces[i].rxrate);
			set_cursor(1, y++);
			printf("\n%s%s             \t | \tTX Rate                                         | \t %f \t kB/s", MAGENTA, obox.eth_faces[i].ifname, obox.eth_faces[i].txrate);
		}
		//Dynamic view of WiFi
		set_cursor(1, y++);
		printf("\n%sWiFi             \t | \tNumber of interfaces                            | \t %d", YELLOW, obox.num_wifi_ifaces);
		for (int i = 0; i < obox.num_wifi_ifaces; i++) {
			set_cursor(1, y++);
			printf("\n%s%s            \t | \tRX Bytes                                         | \t %d \t\t Bytes", YELLOW, obox.wifi[i].ifname, obox.wifi[i].rxbytes);
			set_cursor(1, y++);
			printf("\n%s%s            \t | \tTX Bytes                                         | \t %d \t\t Bytes", YELLOW, obox.wifi[i].ifname, obox.wifi[i].txbytes);
			set_cursor(1, y++);
			printf("\n%s%s            \t | \tRX Dropped                                       | \t %d \t\t Packets", YELLOW, obox.wifi[i].ifname, obox.wifi[i].rxdropped);
			set_cursor(1, y++);
			printf("\n%s%s            \t | \tRX Errors                                        | \t %d \t\t Packets", YELLOW, obox.wifi[i].ifname, obox.wifi[i].rxerrors);
			set_cursor(1, y++);
			printf("\n%s%s            \t | \tTX Dropped                                       | \t %d \t\t Packets", YELLOW, obox.wifi[i].ifname, obox.wifi[i].txdropped);
			set_cursor(1, y++);
			printf("\n%s%s            \t | \tTX Errors                                        | \t %d \t\t Packets", YELLOW, obox.wifi[i].ifname, obox.wifi[i].txerrors);
			set_cursor(1, y++);
			printf("\n%s%s            \t | \tNoise                                            | \t %d \t\t dBm", YELLOW, obox.wifi[i].ifname, obox.wifi[i].noise);
			set_cursor(1, y++);
			printf("\n%s%s            \t | \tLinkspeed                                        | \t %d \t\t Mb/s", YELLOW, obox.wifi[i].ifname, obox.wifi[i].linkspeed);
			set_cursor(1, y++);
			printf("\n%s%s            \t | \tRSSI                                             | \t %d \t\t dBm", YELLOW, obox.wifi[i].ifname, obox.wifi[i].rssi);
			set_cursor(1, y++);
			printf("\n%s%s            \t | \tFrequency                                        | \t %d \t\t MHz", YELLOW, obox.wifi[i].ifname, obox.wifi[i].frequency);
			set_cursor(1, y++);
			printf("\n%s%s            \t | \tRX Rate                                          | \t %f \t\t kB/s", YELLOW, obox.wifi[i].ifname, obox.wifi[i].rxrate);
			set_cursor(1, y++);
			printf("\n%s%s            \t | \tTX Rate                                          | \t %f \t kB/s", YELLOW, obox.wifi[i].ifname, obox.wifi[i].txrate);
		}
		//Dynamic view of V2X
		set_cursor(1, y++);
		printf("\n%sV2X              \t | \tNumber of interfaces                            | \t %d", CYAN, obox.num_vtox_ifaces);
		for (int i = 0; i < obox.num_vtox_ifaces; i++) {
			set_cursor(1, y++);
			printf("\n%s%s          \t | \tRX Bytes                                         | \t %d \t\t Bytes", CYAN, obox.vtox[i].ifname, obox.vtox[i].rxbytes);
			set_cursor(1, y++);
			printf("\n%s%s          \t | \tTX Bytes                                         | \t %d \t\t Bytes", CYAN, obox.vtox[i].ifname, obox.vtox[i].txbytes);
			set_cursor(1, y++);
			printf("\n%s%s          \t | \tRX Dropped                                       | \t %d \t\t Packets", CYAN, obox.vtox[i].ifname, obox.vtox[i].rxdropped);
			set_cursor(1, y++);
			printf("\n%s%s          \t | \tRX Errors                                        | \t %d \t\t Packets", CYAN, obox.vtox[i].ifname, obox.vtox[i].rxerrors);
			set_cursor(1, y++);
			printf("\n%s%s          \t | \tTX Dropped                                       | \t %d \t\t Packets", CYAN, obox.vtox[i].ifname, obox.vtox[i].txdropped);
			set_cursor(1, y++);
			printf("\n%s%s          \t | \tTX Errors                                        | \t %d \t\t Packets", CYAN, obox.vtox[i].ifname, obox.vtox[i].txerrors);
			set_cursor(1, y++);
			printf("\n%s%s          \t | \tRX Rate                                          | \t %f \t\t kB/s", CYAN, obox.vtox[i].ifname, obox.vtox[i].rxrate);
			set_cursor(1, y++);
			printf("\n%s%s          \t | \tTX Rate                                          | \t %f \t kB/s", CYAN, obox.vtox[i].ifname, obox.vtox[i].txrate);
		}
		//Dynamic View of Temperature
		set_cursor(1, y++);
		printf("\n%sTemperature Sensor\t| \tTemperature                                     | \t %f \t °C", ORANGE, obox.temp_sense);
		printf("\n");
		//sleep function for 0.5 seconds is called so that parameters can be viewed at ease rather than refreshing abruptly.
		sleep(0.5);
	}
	//Current Time Consumption: Refresh rate is 1.3 seconds
}

/*      cli_switch function redirects the flow according to which option the user has asked to display.
 *      char option and int lvl are input parameters for this function which is for service selected and verbosity level selected respectively.
 */
void cli_switch(char option, int lvl, int interface)
{
	switch (option) {
	case 'a':
		display_acc_stats(lvl);
		break;
	case 'b':
		display_ble_stats(lvl);
		break;
	case 'g':
		display_gps_stats(lvl);
		break;
	case 's':
		display_eth_stats(lvl, interface);
		break;
	case 'w':
		display_wifi_stats(lvl);
		break;
	case 'v':
		display_v2x_stats(lvl);
		break;
	case 't':
		display_temp_stats(lvl);
		break;
	case 'h':
		display_help();
		break;
	case 'd':
		dynamic_view();
		break;
	default:
		//If an option other than the given options (can be viewed by obox_cli -h) is selected then Incorrect Option selected warning is given.
		printf("\nIncorrect option selected.\nTry obox_cli -h for more information.");
		break;
	}
}

void interactive_cli(void)
{
	printf("\n>");
	n = 0;
	//Reads the input till enter is encountered.
	while ((buff[n++] = getchar()) != '\n')
		;

	//In case of interactive mode, the entire query 'obox_cli -option level' is sent to server to help distinguish between interactive and non interactive mode.
	sendto(sockfd, buff, sizeof(buff), 0, (SA *)&servaddr, len);

	//If enter is pressed, further function is not processed and the function returns to main()
	if (buff[0] == '\n')
		return;
	char buffer[10];
	int p = 0;

	//Processing the first word of input query
	for (; buff[p] != ' '; p++)
		buffer[p] = buff[p];
	buffer[p] = '\0';

	//For verification that command given is syntactically correct.
	if (strncmp(buffer, "obox_cli", 8)) {
		//If incorrect decalartion used, function is not processed further and is sent back to main()
		printf("\nIncorrect declaration, use \"obox_cli\" to access tool");
		return;
	}
	//Receives the updated obox structure
	recvfrom(sockfd, &obox, sizeof(obox), 0, (SA *)&servaddr, &len);

	char opt = buff[10], level = buff[12];
	int lvl, interface = -1;

	if (buff[14])
		interface = (buff[17] - '0');

	if (level == '2')
		lvl = 2;
	else if (level == '3')
		lvl = 3;
	else
		//If level is given then it is used directly else level of verbosity is set to 1
		lvl = 1;

	cli_switch(opt, lvl, interface);

	//bzero() function resets the input buffer to 0 after processing it for new request
	bzero(buff, sizeof(buff));
}

int main(int argc, char **argv)
{
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd == -1) {
		printf("Socket creation failed...\n");
		close(sockfd);
		exit(0);
	}
	bzero(&servaddr, sizeof(len));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	servaddr.sin_port = htons(PORT);
	len = sizeof(servaddr);
	/*      There are 2 ways to communicate with daemon server through stats client tool: Interactive and non-interactive.
	 *      1) While using interactive mode the first command given is 'obx_stats_cli' to start using the tool.
	 *         In this method, the user is using the tool in and infinite manner and can view any stat by giving the relevant command whenever '>' is displayed.
	 *         The commands are given as >obox_cli -[a/b/g/s/v/w/t] [1/2/3] to display stats.
	 *      2) While using non interactive mode the params can be accessed without using the tool indefinitely.
	 *         In this method, the command is given as obx_stats_cli obox_cli -[a/b/g/s/v/w/t] [1/2/3] to display stats at root prompt.
	 */
	if (argc == 1) {
		//In case of interactive mode, the passed arguments to main is a single argument (obx_stats_cli) which triggers the tool to run in an infinite loop
		//Hence the client side calls the interactive_cli() function infinitely.
		for (;;)
			interactive_cli();
	} else {
		//In case of non interactive mode, the passed arguments to main can be 3-4 arguments, hence we only process the stats tool once for the queried parameter.
		//cli_request string will be used to send the user query to daemon server
		char cli_request[10];

		/* argv holds the arguments passed to main by user.
		 * argv[0] holds 'obx_stats_cli' indicating to run this tool
		 * argc[1] holds 'obox_cli' to maintain uniformity between interactive and non interactive modes
		 * argv[2] holds the option -[a/b/g/s/v/w/t] which will be copied to input query sent to server
		 */
		strcpy(cli_request, argv[2]);
		if (argv[3]) {
			//It is optional for the user if verbosity level has to be defined.
			//In case user decides to send the level, it will be stored in argv[3] which is appended to the query
			strcat(cli_request, " ");
			strcat(cli_request, argv[3]);
		}
		sendto(sockfd, cli_request, sizeof(cli_request), 0, (SA *)&servaddr, len);
		recvfrom(sockfd, &obox, sizeof(obox), 0, (SA *)&servaddr, &len);
		if (argv[3])    {
			//If user has defined the verbosity level, the level along with option selected is sent to cli_switch() function
			if (argv[4])
				cli_switch(argv[2][1], atoi(argv[3]), (argv[4][3] - '0'));
			else
				cli_switch(argv[2][1], atoi(argv[3]), -1);
		} else {
			//In case no level is selected, level 1 verbosity is given as the standard to cli_switch() function
			if (argv[4])
				cli_switch(argv[2][1], 1, (argv[4][3] - '0'));
			else
				cli_switch(argv[2][1], 1, -1);
		}
	}
	close(sockfd);
	return 0;
}

