// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2023 NXP

#include<stdio.h>
#include<netinet/in.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netdb.h>
#include<string.h>
#include<stdlib.h>
#include<time.h>
#include<errno.h>
#include<stdbool.h>
#include<unistd.h>
#include<arpa/inet.h>
#include "parameters.h"
//MAX macro has been used to define the maximum input buffer that is used to receive as a request from client tool
#define MAX 80
//PORT macro has been used to define the port number for socket address for communication with client
#define PORT 8080
#define SA struct sockaddr
struct orange_box_stats obox;
/*      These boolean variables have been used as flags which will be set after processing client tool's query.
 *      -> is_interactive_cli_cmd flag: Set when client tool is used in interactive mode
 *      -> is_gui_enabled flag: Set when client tool is used in non interactive mode by Stats GUI tool's Static View Page (Verbosity Level 2 for extra data)
 *      -> is_dynamic_mode flag: Set when client tool is used in non interactive mode by Stats GUI tool's Dynamic View Page (Verbosity Level 1 for faster updation)
 */
bool is_interactive_cli_cmd, is_dynamic_mode, is_gui_enabled;

void update_acc_stats(void)
{
	FILE *accFile;

	float scale_value, x_axis, y_axis, z_axis;
	char line[10];

	//Scale value is accessed since it will be used to scale down x, y and z acis to appropriate limits
	accFile = popen("cat /sys/bus/iio/devices/iio\\:device0/in_accel_scale", "r");
	if (accFile == NULL)
		perror("File error");
	if (fgets(line, sizeof(line), accFile) == NULL)
		perror("Accelerometer scale fgets error");
	scale_value = atof(line);
	pclose(accFile);

	accFile = popen("cat /sys/bus/iio/devices/iio\\:device0/in_accel_x_raw", "r");
	if (accFile == NULL)
		perror("File error");
	//Extracts the x_axis parameter from sysfs
	if (fgets(line, sizeof(line), accFile) == NULL)
		perror("Accelerometer x fgets error");
	x_axis = atof(line);
	obox.acc.x_axis = x_axis * scale_value * (256.0 / 9.81);
	pclose(accFile);

	accFile = popen("cat /sys/bus/iio/devices/iio\\:device0/in_accel_y_raw", "r");
	if (accFile == NULL)
		perror("File error");
	//Extracts the y_axis parameter from sysfs
	if (fgets(line, sizeof(line), accFile) == NULL)
		perror("Accelerometer y fgets error");
	y_axis = atof(line);
	obox.acc.y_axis = y_axis * scale_value * (256.0 / 9.81);
	pclose(accFile);

	accFile = popen("cat /sys/bus/iio/devices/iio\\:device0/in_accel_z_raw", "r");
	if (accFile == NULL)
		perror("File error");
	//Extracts the z_axis parameter from sysfs
	if (fgets(line, sizeof(line), accFile) == NULL)
		perror("Accelerometer z fgets error");
	z_axis = atof(line);
	obox.acc.z_axis = z_axis * scale_value * (256.0 / 9.81);
	pclose(accFile);
}

void update_gps_stats(void)
{
	FILE *gpsFile;
	//line is used for extracting input from file and buff is used for storing manipulated string line
	char line[50], buff[50];
	int j = 0;

	/*      Timeout of 2 seconds has been put since the command executes infinitely and blocks the process
	 *      Storing all commands' output in log file and then reading from log.txt using File operations.
	 */
	//GNRMC
	if (system("timeout 2 cat /dev/ttyLP3 > log.txt") == -1)
		perror("System error");
	gpsFile = popen("cat log.txt | grep GNRMC", "r");
	if (gpsFile == NULL)
		perror("File error");
	if (fgets(line, sizeof(line), gpsFile) == NULL)
		perror("GNSS GNRMC fgets error");
	//Output is in the form $GNRMC,X,Y,Z...; Manipulating string to display only variable values
	for (int i = 7; line[i] != '\0'; i++, j++)
		buff[j] = line[i];
	buff[j] = '\0';
	strcpy(obox.gps.gnrmc, buff);
	strcpy(buff, "");
	j = 0;
	pclose(gpsFile);

	//GNVTG
	gpsFile = popen("cat log.txt | grep GNVTG", "r");
	if (gpsFile == NULL)
		perror("File error");
	if (fgets(line, sizeof(line), gpsFile) == NULL)
		perror("GNSS GNVTG fgets error");
	//Output is in the form $GNVTG,X,Y,Z...; Manipulating string to display only variable values
	for (int i = 7; line[i] != '\0'; i++, j++)
		buff[j] = line[i];
	buff[j] = '\0';
	strcpy(obox.gps.gnvtg, buff);
	strcpy(buff, "");
	j = 0;
	pclose(gpsFile);

	//GNGGA
	gpsFile = popen("cat log.txt | grep GNGGA", "r");
	if (gpsFile == NULL)
		perror("File error");
	//Output is in the form $GNGGA,X,Y,Z...; Manipulating string to display only variable values
	if (fgets(line, sizeof(line), gpsFile) == NULL)
		perror("GNSS GNGGA fgets error");
	for (int i = 7; line[i] != '\0'; i++, j++)
		buff[j] = line[i];
	buff[j] = '\0';
	strcpy(obox.gps.gngga, buff);
	strcpy(buff, "");
	j = 0;
	pclose(gpsFile);

	//GNGLL
	gpsFile = popen("cat log.txt | grep GNGLL", "r");
	if (gpsFile == NULL)
		perror("File error");
	if (fgets(line, sizeof(line), gpsFile) == NULL)
		perror("GNSS GNGLL fgets error");
	//Output is in the form $GNGLL,X,Y,Z...; Manipulating string to display only variable values
	for (int i = 7; line[i] != '\0'; i++, j++)
		buff[j] = line[i];
	buff[j] = '\0';
	strcpy(obox.gps.gngll, buff);
	strcpy(buff, "");
	j = 0;
	pclose(gpsFile);

	//GPGSV
	gpsFile = popen("cat log.txt | grep GPGSV", "r");
	if (gpsFile == NULL)
		perror("File error");
	if (fgets(line, sizeof(line), gpsFile) == NULL)
		perror("GNSS GPGSV fgets error");
	//Output is in the form $GPGSV,X,Y,Z...; Manipulating string to display only variable values
	for (int i = 7; line[i] != '\0'; i++, j++)
		buff[j] = line[i];
	buff[j] = '\0';
	strcpy(obox.gps.gpgsv, buff);
	strcpy(buff, "");
	j = 0;
	pclose(gpsFile);

	//GAGSV
	gpsFile = popen("cat log.txt | grep GAGSV", "r");
	if (gpsFile == NULL)
		perror("File error");
	if (fgets(line, sizeof(line), gpsFile) == NULL)
		perror("GNSS GAGSV fgets error");
	//Output is in the form $GAGSV,X,Y,Z...; Manipulating string to display only variable values
	for (int i = 7; line[i] != '\0'; i++, j++)
		buff[j] = line[i];
	buff[j] = '\0';
	strcpy(obox.gps.gagsv, buff);
	strcpy(buff, "");
	j = 0;
	pclose(gpsFile);

	//GBGSV
	gpsFile = popen("cat log.txt | grep GBGSV", "r");
	if (gpsFile == NULL)
		perror("File error");
	if (fgets(line, sizeof(line), gpsFile) == NULL)
		perror("GNSS GBGSV fgets error");
	//Output is in the form $GBGSV,X,Y,Z...; Manipulating string to display only variable values
	for (int i = 7; line[i] != '\0'; i++, j++)
		buff[j] = line[i];
	buff[j] = '\0';
	strcpy(obox.gps.gbgsv, buff);
	strcpy(buff, "");
	j = 0;
	pclose(gpsFile);

	//GLGSV
	gpsFile = popen("cat log.txt | grep GLGSV", "r");
	if (gpsFile == NULL)
		perror("File error");
	if (fgets(line, sizeof(line), gpsFile) == NULL)
		perror("GNSS GLGSV fgets error");
	//Output is in the form $GLGSV,X,Y,Z...; Manipulating string to display only variable values
	for (int i = 7; line[i] != '\0'; i++, j++)
		buff[j] = line[i];
	buff[j] = '\0';
	strcpy(obox.gps.glgsv, buff);
	strcpy(buff, "");
	j = 0;
	pclose(gpsFile);

	//GNGSA -- 4
	gpsFile = popen("cat log.txt | grep GNGSA", "r");
	if (gpsFile == NULL)
		perror("File error");
	if (fgets(line, sizeof(line), gpsFile) == NULL)
		perror("GNSS GNGSA 1 fgets error");
	//There are 4 values for GNGSA hence line is read 1 by 1 for each GNGSA
	//Output is in the form $GNGSA,X,Y,Z...; Manipulating string to display only variable values
	for (int i = 7; line[i] != '\0'; i++, j++)
		buff[j] = line[i];
	buff[j] = '\0';
	strcpy(obox.gps.gngsa1, buff);
	strcpy(buff, "");
	j = 0;
	if (fgets(line, sizeof(line), gpsFile) == NULL)
		perror("GNSS GNGSA 2 fgets error");
	//Output is in the form $GNGSA,X,Y,Z...; Manipulating string to display only variable values
	for (int i = 7; line[i] != '\0'; i++, j++)
		buff[j] = line[i];
	buff[j] = '\0';
	strcpy(obox.gps.gngsa2, buff);
	strcpy(buff, "");
	j = 0;
	if (fgets(line, sizeof(line), gpsFile) == NULL)
		perror("GNSS GNGSA 3 fgets error");
	//Output is in the form $GNGSA,X,Y,Z...; Manipulating string to display only variable values
	for (int i = 7; line[i] != '\0'; i++, j++)
		buff[j] = line[i];
	buff[j] = '\0';
	strcpy(obox.gps.gngsa3, buff);
	strcpy(buff, "");
	j = 0;
	if (fgets(line, sizeof(line), gpsFile) == NULL)
		perror("GNSS GNGSA 4 fgets error");
	//Output is in the form $GNGSA,X,Y,Z...; Manipulating string to display only variable values
	for (int i = 7; line[i] != '\0'; i++, j++)
		buff[j] = line[i];
	buff[j] = '\0';
	strcpy(obox.gps.gngsa4, buff);
	strcpy(buff, "");
	remove("log.txt");
	pclose(gpsFile);
}

void update_wifi_stats(void)
{
	FILE *wifiFile;
	char num_of_wifi_ifaces[2], line[50], ifconfig_init[100] = "ifconfig ", command_search[100], interface_name[10], wpa_init[100] = "wpa_cli -i ";
	int x, i = 0;

	//-c option extracts the number of interaces with "mlan" regex
	wifiFile = popen("ifconfig | grep -c mlan", "r");
	if (wifiFile == NULL)
		perror("File error");
	if (fgets(num_of_wifi_ifaces, sizeof(num_of_wifi_ifaces), wifiFile) == NULL)
		perror("Wifi interfaces fgets error");
	x = atoi(num_of_wifi_ifaces);
	obox.num_wifi_ifaces = x;
	pclose(wifiFile);

	//grep mlan will print the first column storing the name of the interface
	wifiFile = popen("ifconfig | grep \"mlan\" | awk \'{print $1}\'", "r");
	if (wifiFile == NULL)
		perror("File error");
	while (fgets(line, sizeof(line), wifiFile)) {
		char get_name[50];
		int p = 0;
		//The interface name is stored as 1.X\n 2.Y\n; Manipulating strings to extract the name of interfaces only.
		for (p = 0; line[p] != '\n'; p++)
			get_name[p] = line[p];
		get_name[p] = '\0';
		strcpy(obox.wifi[i++].ifname, get_name);
	}
	pclose(wifiFile);

	for (i = 0; i < x; i++) {

		//Description
		strcpy(obox.wifi[i].description, "Wireless Interface");

		//State
		/*      A series of strcpy and strcat are used to make the final system command which includes the name of interface whose details have to be fetched; to be given to popen()
		 *      Post execution the command will display as: wpa_cli -i mlan0 status | grep wpa_state
		 */
		strcpy(interface_name, obox.wifi[i].ifname);
		strcpy(wpa_init, "wpa_cli -i ");
		strncat(wpa_init, interface_name, 5);
		strcpy(command_search, " status | grep wpa_state");
		strcat(wpa_init, command_search);

		wifiFile = popen(wpa_init, "r");
		if (wifiFile == NULL)
			perror("File error");
		strcpy(line, "");
		if (fgets(line, sizeof(line), wifiFile) == NULL)
			perror("Wifi state fgets error");
		//Output is in the form of wpa_state=XYZ; Manipulating strings to get value of state only
		char temp_buffer[10];
		int j = 0, prev_rx, prev_tx;

		for (int k = 10; line[k] != '\0'; k++, j++)
			temp_buffer[j] = line[k];
		temp_buffer[j] = '\0';
		strcpy(obox.wifi[i].state, temp_buffer);
		//Re-initialising temp buffer and counter for next variable.
		strcpy(temp_buffer, "");
		j = 0;
		pclose(wifiFile);

		//INET 6 address
		strcpy(ifconfig_init, "ifconfig ");
		strncat(ifconfig_init, interface_name, 5);
		strcpy(command_search, " | grep Scope:Link | awk \'{print $3}\'");
		//Inet6 addresses of two types are available: Global and Link scope. Extracting only Link scope Inet6.
		strcat(ifconfig_init, command_search);
		//Post execution the command will display as: ifconfig mlan0 | grep Scope:Link | awk '{print $3}'
		wifiFile = popen(ifconfig_init, "r");
		if (wifiFile == NULL)
			perror("File error");
		if (fgets(line, sizeof(line), wifiFile) == NULL)
			perror("Wifi inet6 fgets error");
		if (sizeof(line) > 1) {
			char temp_address_inet6[INET6_ADDRSTRLEN];
			int l = 0;
			//Address is in the form of X:X:X:X:X/Y. Manipulating strings to extract address before '/'.
			for (l = 0; line[l] != '/'; l++)
				temp_address_inet6[l] = line[l];
			temp_address_inet6[l] = '\0';
			if (inet_pton(AF_INET6, temp_address_inet6, &obox.wifi[i].inet6_addr) != 1)
				perror("Inet6 conversion error");
		}
		pclose(wifiFile);

		//INET 4 address
		strcpy(ifconfig_init, "ifconfig ");
		strncat(ifconfig_init, interface_name, 5);
		strcpy(command_search, " | grep inet | awk \'{print $2}\'");
		strcat(ifconfig_init, command_search);
		//Post execution the command will display as: ifconfig mlan0 | grep inet | awk '{print $2}'
		wifiFile = popen(ifconfig_init, "r");
		if (wifiFile == NULL)
			perror("File error");
		if (fgets(line, sizeof(line), wifiFile) == NULL)
			perror("Wifi inet4 fgets error");
		char temp_address_inet4[INET_ADDRSTRLEN];
		int y = 0;

		//Address stored in form of addr:X:X:X:X; Manipulating strings to extract address from 5th character onwards.
		for (int q = 5; line[q] != '\0'; q++, y++)
			temp_address_inet4[y] = line[q];
		temp_address_inet4[y-1] = '\0';
		//If anything is displayed on output then only it will store in obox structure address.
		if (strlen(temp_address_inet4) > 1) {
			if (inet_pton(AF_INET, temp_address_inet4, &obox.wifi[i].inet4_addr) != 1)
				perror("Inet4 conversion error");
		}
		pclose(wifiFile);

		//MAC address
		strcpy(ifconfig_init, "ifconfig ");
		strncat(ifconfig_init, interface_name, 5);
		strcpy(command_search, " | grep HWaddr | awk \'{print $5}\'");
		strcat(ifconfig_init, command_search);
		//Post execution the command will display as: ifconfig mlan0 | grep HWaddr | awk '{print $5}'
		wifiFile = popen(ifconfig_init, "r");
		if (wifiFile == NULL)
			perror("File error");
		strcpy(line, "");
		if (fgets(line, sizeof(line), wifiFile) == NULL)
			perror("Wifi mac fgets error");
		strncpy(obox.wifi[i].macaddress, line, sizeof(obox.wifi[i].macaddress));
		pclose(wifiFile);

		//UUID
		strcpy(wpa_init, "wpa_cli -i ");
		//-i option defines the interface to be queried
		strncat(wpa_init, interface_name, 5);
		strcpy(command_search, " status | grep uuid");
		strcat(wpa_init, command_search);
		wifiFile = popen(wpa_init, "r");
		if (wifiFile == NULL)
			perror("File error");
		if (fgets(line, sizeof(line), wifiFile) == NULL)
			perror("Wifi uuid fgets error");
		//Output is in the form of uuid=XYZ; Manipulating strings to get value of uuid only
		for (int k = 5; line[k] != '\0'; k++, j++)
			temp_buffer[j] = line[k];
		temp_buffer[j] = '\0';
		strcpy(obox.wifi[i].uuid, temp_buffer);
		strcpy(temp_buffer, "");
		j = 0;
		pclose(wifiFile);

		//Frequency
		strcpy(wpa_init, "wpa_cli -i ");
		strncat(wpa_init, interface_name, 5);
		strcpy(command_search, " signal_poll | grep FREQUENCY");
		strcat(wpa_init, command_search);
		wifiFile = popen(wpa_init, "r");
		if (wifiFile == NULL)
			perror("File error");
		if (fgets(line, sizeof(line), wifiFile) == NULL)
			perror("Wifi frequency fgets error");
		//Output is in the form of FREQUENCY=XYZ; Manipulating strings to get value of frequency only
		for (int k = 10; line[k] != '\0'; k++, j++)
			temp_buffer[j] = line[k];
		temp_buffer[j] = '\0';
		obox.wifi[i].frequency = atoi(temp_buffer);
		strcpy(temp_buffer, "");
		j = 0;
		pclose(wifiFile);

		//RX bytes
		strcpy(ifconfig_init, "ifconfig ");
		strncat(ifconfig_init, interface_name, 5);
		strcpy(command_search, " | grep \"RX bytes\" | awk \'{print $2}\'");
		strcat(ifconfig_init, command_search);
		wifiFile = popen(ifconfig_init, "r");
		if (wifiFile == NULL)
			perror("File error");
		if (fgets(line, sizeof(line), wifiFile) == NULL)
			perror("WiFi RX fgets error");
		//Output is in the form of bytes:XYZ; Manipulating strings to get value of bytes only
		for (int k = 6; line[k] != '\0'; k++, j++)
			temp_buffer[j] = line[k];
		temp_buffer[j] = '\0';
		prev_rx = obox.wifi[i].rxbytes;
		obox.wifi[i].rxbytes = atoi(temp_buffer);
		strcpy(temp_buffer, "");
		j = 0;
		pclose(wifiFile);

		//TX bytes
		strcpy(ifconfig_init, "ifconfig ");
		strncat(ifconfig_init, interface_name, 5);
		strcpy(command_search, " | grep \"TX bytes\" | awk \'{print $6}\'");
		strcat(ifconfig_init, command_search);
		wifiFile = popen(ifconfig_init, "r");
		if (wifiFile == NULL)
			perror("File error");
		if (fgets(line, sizeof(line), wifiFile) == NULL)
			perror("WiFi TX fgets error");
		//Output is in the form of bytes:XYZ; Manipulating strings to get value of bytes only
		for (int k = 6; line[k] != '\0'; k++, j++)
			temp_buffer[j] = line[k];
		temp_buffer[j] = '\0';
		prev_tx = obox.wifi[i].txbytes;
		obox.wifi[i].txbytes = atoi(temp_buffer);
		strcpy(temp_buffer, "");
		j = 0;
		pclose(wifiFile);

		//RX dropped
		strcpy(ifconfig_init, "ifconfig ");
		strncat(ifconfig_init, interface_name, 5);
		strcpy(command_search, " | grep \"RX packets\" | awk '{print $4}' | cut -d ':' -f 2");
		strcat(ifconfig_init, command_search);
		wifiFile = popen(ifconfig_init, "r");
		if (wifiFile == NULL)
			perror("File error");
		if (fgets(line, sizeof(line), wifiFile) == NULL)
			perror("WiFi RX dropped fgets error");
		obox.wifi[i].rxdropped = atoi(line);
		pclose(wifiFile);

		//RX errors
		strcpy(ifconfig_init, "ifconfig ");
		strncat(ifconfig_init, interface_name, 5);
		strcpy(command_search, " | grep \"RX packets\" | awk '{print $3}' | cut -d ':' -f 2");
		strcat(ifconfig_init, command_search);
		wifiFile = popen(ifconfig_init, "r");
		if (wifiFile == NULL)
			perror("File error");
		if (fgets(line, sizeof(line), wifiFile) == NULL)
			perror("WiFi RX errors fgets error");
		obox.wifi[i].rxerrors = atoi(line);
		pclose(wifiFile);

		//TX dropped
		strcpy(ifconfig_init, "ifconfig ");
		strncat(ifconfig_init, interface_name, 5);
		strcpy(command_search, " | grep \"TX packets\" | awk '{print $4}' | cut -d ':' -f 2");
		strcat(ifconfig_init, command_search);
		wifiFile = popen(ifconfig_init, "r");
		if (wifiFile == NULL)
			perror("File error");
		if (fgets(line, sizeof(line), wifiFile) == NULL)
			perror("WiFi TX dropped fgets error");
		obox.wifi[i].txdropped = atoi(line);
		pclose(wifiFile);

		//TX errors
		strcpy(ifconfig_init, "ifconfig ");
		strncat(ifconfig_init, interface_name, 5);
		strcpy(command_search, " | grep \"TX packets\" | awk '{print $3}' | cut -d ':' -f 2");
		strcat(ifconfig_init, command_search);
		wifiFile = popen(ifconfig_init, "r");
		if (wifiFile == NULL)
			perror("File error");
		if (fgets(line, sizeof(line), wifiFile) == NULL)
			perror("WiFi TX errors fgets error");
		obox.wifi[i].txerrors = atoi(line);
		pclose(wifiFile);

		//Noise
		strcpy(wpa_init, "wpa_cli -i ");
		strncat(wpa_init, interface_name, 5);
		strcpy(command_search, " signal_poll | grep NOISE");
		strcat(wpa_init, command_search);
		wifiFile = popen(wpa_init, "r");
		if (wifiFile == NULL)
			perror("File error");
		if (fgets(line, sizeof(line), wifiFile) == NULL)
			perror("Wifi noise fgets error");
		//Output is in the form of NOISE=XYZ; Manipulating strings to get value of noise only
		for (int k = 6; line[k] != '\0'; k++, j++)
			temp_buffer[j] = line[k];
		temp_buffer[j] = '\0';
		obox.wifi[i].noise = atoi(temp_buffer);
		strcpy(temp_buffer, "");
		j = 0;
		pclose(wifiFile);

		//RSSI
		strcpy(wpa_init, "wpa_cli -i ");
		strncat(wpa_init, interface_name, 5);
		strcpy(command_search, " signal_poll | grep RSSI");
		strcat(wpa_init, command_search);
		wifiFile = popen(wpa_init, "r");
		if (wifiFile == NULL)
			perror("File error");
		if (fgets(line, sizeof(line), wifiFile) == NULL)
			perror("Wifi rssi fgets error");
		//Output is in the form of RSSI=XYZ; Manipulating strings to get value of rssi only
		for (int k = 5; line[k] != '\0'; k++, j++)
			temp_buffer[j] = line[k];
		temp_buffer[j] = '\0';
		obox.wifi[i].rssi = atoi(temp_buffer);
		strcpy(temp_buffer, "");
		j = 0;
		pclose(wifiFile);

		//Linkspeed
		strcpy(wpa_init, "wpa_cli -i ");
		strncat(wpa_init, interface_name, 5);
		strcpy(command_search, " signal_poll | grep LINKSPEED");
		strcat(wpa_init, command_search);
		wifiFile = popen(wpa_init, "r");
		if (wifiFile == NULL)
			perror("File error");
		if (fgets(line, sizeof(line), wifiFile) == NULL)
			perror("Wifi linkspeed fgets error");
		//Output is in the form of LINKSPEED=XYZ; Manipulating strings to get value of linkspeed only
		for (int k = 10; line[k] != '\0'; k++, j++)
			temp_buffer[j] = line[k];
		temp_buffer[j] = '\0';
		obox.wifi[i].linkspeed = atoi(temp_buffer);
		strcpy(temp_buffer, "");
		j = 0;
		pclose(wifiFile);

		//RX rate
		clock_t current_time = clock();
		double time_difference = (((double) (current_time - obox.time_stamp)) / CLOCKS_PER_SEC);
		int rx_bytes_diff = obox.wifi[i].rxbytes - prev_rx;

		obox.wifi[i].rxrate = ((rx_bytes_diff / time_difference) / 1000);

		//TX Rate
		int tx_bytes_diff = obox.wifi[i].txbytes - prev_tx;

		obox.wifi[i].txrate = ((tx_bytes_diff / time_difference) / 1000);

		//UP
		strcpy(ifconfig_init, "ifconfig ");
		strncat(ifconfig_init, interface_name, 5);
		//-o Option searches for the exact match of the given regex. If UP is found, it will be set true.
		strcpy(command_search, " | grep -o UP");
		strcat(ifconfig_init, command_search);
		wifiFile = popen(ifconfig_init, "r");
		if (wifiFile == NULL)
			perror("File error");
		if (fgets(line, sizeof(line), wifiFile) == NULL)
			perror("Wifi up fgets error");
		//Compare the first two characters with UP, to avoid comparison of any extra characters observed on display.
		if (strncmp("UP", line, 2) == 0)
			obox.wifi[i].up = true;
		else
			obox.wifi[i].up = false;
		pclose(wifiFile);

		//RUNNING
		strcpy(ifconfig_init, "ifconfig ");
		strncat(ifconfig_init, interface_name, 5);
		//-o Option searches for the exact match of the given regex. If RUNNING is found, it will be set true.
		strcpy(command_search, " | grep -o RUNNING");
		strcat(ifconfig_init, command_search);
		wifiFile = popen(ifconfig_init, "r");
		if (wifiFile == NULL)
			perror("File error");
		if (fgets(line, sizeof(line), wifiFile) == NULL)
			perror("Wifi running fgets error");
		//Compare the first 7 characters with RUNNING, to avoid comparison of any extra characters observed on display.
		if (strncmp("RUNNING", line, 7) == 0)
			obox.wifi[i].running = true;
		else
			obox.wifi[i].running = false;
		pclose(wifiFile);

		//BROADCAST
		strcpy(ifconfig_init, "ifconfig ");
		strncat(ifconfig_init, interface_name, 5);
		//-o Option searches for the exact match of the given regex. If BROADCAST is found, it will be set true.
		strcpy(command_search, " | grep -o BROADCAST");
		strcat(ifconfig_init, command_search);
		wifiFile = popen(ifconfig_init, "r");
		if (wifiFile == NULL)
			perror("File error");
		if (fgets(line, sizeof(line), wifiFile) == NULL)
			perror("Wifi broadcast fgets error");
		//Compare the first 9 characters with BROADCAST, to avoid comparison of any extra characters observed on display.
		if (strncmp("BROADCAST", line, 9) == 0)
			obox.wifi[i].broadcast = true;
		else
			obox.wifi[i].broadcast = false;
		pclose(wifiFile);

		//MULTICAST
		strcpy(ifconfig_init, "ifconfig ");
		strncat(ifconfig_init, interface_name, 5);
		//-o Option searches for the exact match of the given regex. If MULTICAST is found, it will be set true.
		strcpy(command_search, " | grep -o MULTICAST");
		strcat(ifconfig_init, command_search);
		wifiFile = popen(ifconfig_init, "r");
		if (wifiFile == NULL)
			perror("File error");
		if (fgets(line, sizeof(line), wifiFile) == NULL)
			perror("Wifi multicast fgets error");
		//Compare the first 9 characters with MULTICAST, to avoid comparison of any extra characters observed on display.
		if (strncmp("MULTICAST", line, 9) == 0)
			obox.wifi[i].multicast = true;
		else
			obox.wifi[i].multicast = false;
		pclose(wifiFile);
	}
}

void update_ble_stats(void)
{
	FILE *bleFile;
	char num_of_controllers[2];
	int temp_controllers;

	//Extracting number of bluetooth controllers.
	//-c option in grep will tell the number of controllers with "hci" string in them.
	bleFile = popen("hciconfig -a | grep -c hci", "r");
	if (bleFile == NULL)
		perror("File error");
	if (fgets(num_of_controllers, sizeof(num_of_controllers), bleFile) == NULL)
		perror("Bluetooth controller number fgets error");
	temp_controllers = atoi(num_of_controllers);
	obox.num_ble_ifaces = temp_controllers;
	pclose(bleFile);

	for (int i = 0; i < temp_controllers; i++) {
		FILE *bleNode;
		char line[20];

		//Interface name
		bleNode = popen("hciconfig -a | grep \"hci\" | awk \'{print $1}\'", "r");
		if (bleNode == NULL)
			perror("File error");
		if (fgets(line, sizeof(line), bleNode) == NULL)
			perror("Bluetooth name fgets error");
		//Interface Name in the form of hciX: ; Manipulating string to store name only
		char temp_ifname[30];
		int l = 0;

		for (l = 0; line[l] != ':'; l++)
			temp_ifname[l] = line[l];
		temp_ifname[l] = '\0';
		strcpy(obox.bluetooth[i].iname, temp_ifname);
		strcpy(line, "");
		pclose(bleNode);

		if (((is_gui_enabled == true) && (is_dynamic_mode == false)) || (is_interactive_cli_cmd == true)) {
			/*      These flags check is more verbosity is required or not. Two cases will be generated when these extra parameters would be required:
			 *      1) Static View Page of GUI enabled: Requires verbosity level 2 (all params get listed)
			 *      2) Interactive CLI mode: Viewing Bluetooth statically
			 */
			//MAC address
			//The MAC address is stored in the second column which is extracted with the help of awk
			bleNode = popen("hcitool dev | awk \'{print $2}\'", "r");
			if (bleNode == NULL)
				perror("File error");
			//The address value is stored in the second row hence fgets is called twice.
			if (fgets(line, sizeof(line), bleNode) == NULL)
				perror("Bluetooth MAC fgets error");
			if (fgets(line, sizeof(line), bleNode) == NULL)
				perror("Bluetooth MAC fgets error");
			strcpy(obox.bluetooth[i].macaddr, line);
			strcpy(line, "");
			pclose(bleNode);

			//Class
			//Class is stored in second column, extracted by awk
			bleNode = popen("bluetoothctl show | grep Class | awk \'{print $2}\'", "r");
			if (bleNode == NULL)
				perror("File error");
			if (fgets(line, sizeof(line), bleNode) == NULL)
				perror("Bluetooth class fgets error");
			strcpy(obox.bluetooth[i].class, line);
			strcpy(line, "");
			pclose(bleNode);

			//Status
			//Status is stored in second column, extracted by awk
			bleNode = popen("systemctl status bluetooth.service | grep Status | awk \'{print $2}\'", "r");
			if (bleNode == NULL)
				perror("File error");
			if (fgets(line, sizeof(line), bleNode) == NULL)
				perror("Bluetooth status fgets error");
			strcpy(obox.bluetooth[i].status, line);
			strcpy(line, "");
			pclose(bleNode);

			//Controller Name
			//Name stored in second column, extracted by awk
			bleNode = popen("bluetoothctl show | grep Name | awk \'{print $2}\'", "r");
			if (bleNode == NULL)
				perror("File error");
			if (fgets(line, sizeof(line), bleNode) == NULL)
				perror("Bluetooth controller fgets error");
			strcpy(obox.bluetooth[i].name, line);
			strcpy(line, "");
			pclose(bleNode);

			//Bus Type
			//Bus Type stored in fifth column, extracted by awk
			bleNode = popen("hciconfig -a | grep Bus | awk \'{print $5}\'", "r");
			if (bleNode == NULL)
				perror("File error");
			if (fgets(line, sizeof(line), bleNode) == NULL)
				perror("Bluetooth bus fgets error");
			strcpy(obox.bluetooth[i].type, line);
			strcpy(line, "");
			pclose(bleNode);

			//PID
			//PID stored in 3rd column
			bleNode = popen("systemctl status bluetooth.service | grep PID | awk \'{print $3}\'", "r");
			if (bleNode == NULL)
				perror("File error");
			if (fgets(line, sizeof(line), bleNode) == NULL)
				perror("Bluetooth pid fgets error");
			obox.bluetooth[i].pid = atoi(line);
			strcpy(line, "");
			pclose(bleNode);
		}

		//RXbytes
		//Bytes stored in second column
		bleNode = popen("hciconfig -a | grep \"RX bytes\" | awk \'{print $2}\'", "r");
		if (bleNode == NULL)
			perror("File error");
		if (fgets(line, sizeof(line), bleNode) == NULL)
			perror("Bluetooth RX fgets error");
		//RX bytes stored in the form of bytes:XYZ; Manipulating string to extract number of bytes only
		int j = 0, max_available_devices, prev_rx, prev_tx;
		char rx[10], tx[10];

		for (int i = 6; line[i] != '\0'; i++, j++)
			rx[j] = line[i];
		rx[j] = '\0';
		prev_rx = obox.bluetooth[i].rxbytes;
		obox.bluetooth[i].rxbytes = atoi(rx);
		strcpy(line, "");
		pclose(bleNode);

		//TXbytes
		//Bytes stored in second column
		bleNode = popen("hciconfig -a | grep \"TX bytes\" | awk \'{print $2}\'", "r");
		if (bleNode == NULL)
			perror("File error");
		if (fgets(line, sizeof(line), bleNode) == NULL)
			perror("Bluetooth TX fgets error");
		//TX bytes stored in the form of bytes:XYZ; Manipulating string to extract number of bytes only
		for (int i = 6, j = 0; line[i] != '\0'; i++, j++)
			tx[j] = line[i];
		tx[j] = '\0';
		prev_tx = obox.bluetooth[i].txbytes;
		obox.bluetooth[i].txbytes = atoi(tx);
		strcpy(line, "");
		pclose(bleNode);

		//RX errors
		//Packets stored in second column
		bleNode = popen("hciconfig -a | grep \"RX bytes\" | awk '{print $6}' | cut -d ':' -f 2", "r");
		if (bleNode == NULL)
			perror("File error");
		if (fgets(line, sizeof(line), bleNode) == NULL)
			perror("Bluetooth RX errors fgets error");
		obox.bluetooth[i].rxerrors = atoi(line);
		strcpy(line, "");
		pclose(bleNode);

		//TX errors
		//Packets stored in second column
		bleNode = popen("hciconfig -a | grep \"TX bytes\" | awk '{print $6}' | cut -d ':' -f 2", "r");
		if (bleNode == NULL)
			perror("File error");
		if (fgets(line, sizeof(line), bleNode) == NULL)
			perror("Bluetooth TX errors fgets error");
		obox.bluetooth[i].txerrors = atoi(line);
		strcpy(line, "");
		pclose(bleNode);

		//RX rate
		//Current time stamp is noted and is compared with the previous updated time stamp to get the time difference.
		clock_t current_time = clock();
		//The difference is divided by predefined CLOCKS_PER_SEC as the difference is in terms of CPU cycles elapsed.
		double time_difference = (((double) (current_time - obox.time_stamp)) / CLOCKS_PER_SEC);
		int rx_bytes_diff = obox.bluetooth[i].rxbytes - prev_rx;
		//The value is divided by 1000 to make the unit kBps instead of Bps.
		obox.bluetooth[i].rxrate = ((rx_bytes_diff / time_difference) / 1000);

		//TX Rate
		int tx_bytes_diff = obox.bluetooth[i].txbytes - prev_tx;

		obox.bluetooth[i].txrate = ((tx_bytes_diff / time_difference) / 1000);

		if (((is_gui_enabled == true) && (is_dynamic_mode == false)) || (is_interactive_cli_cmd == true)) {
			//Flags checked to add verbosity

			//Number of Available devices
			//Timeout used to move onto next command after 3 seconds. This scans the available bluetooth devices for 3 seconds. Output of scan wouldn't be displayed.
			if (system("timeout 3 bluetoothctl scan on &> /dev/null") == -1)
				perror("System error");
			//-c option gets number of bluetooth enabled available devices.
			bleNode = popen("bluetoothctl devices | grep -c Device", "r");
			if (bleNode == NULL)
				perror("File error");
			if (fgets(line, sizeof(line), bleNode) == NULL)
				perror("Bluetooth available devices fgets error");
			max_available_devices = atoi(line);
			if (max_available_devices > MAXAVAIL)
				max_available_devices = MAXAVAIL;
			obox.bluetooth[i].num_available_devices = max_available_devices;
			strcpy(line, "");
			pclose(bleNode);

			//Storing available devices
			int num_devices = 0;

			//Extracting MAC addresses of available bluetooth devices which is in 2nd column
			bleNode = popen("bluetoothctl devices | awk \'{print $2}\'", "r");
			if (bleNode == NULL)
				perror("File error");
			while (fgets(line, sizeof(line), bleNode) && (num_devices < max_available_devices))
				strcpy(obox.bluetooth[i].devices[num_devices++].address, line);
			strcpy(line, "");
			num_devices = 0;
			pclose(bleNode);
			//Extracting name of available bluetooth devices which is in 3rd column
			bleNode = popen("bluetoothctl devices | awk \'{print $3}\'", "r");
			if (bleNode == NULL)
				perror("File error");
			while (fgets(line, sizeof(line), bleNode) && (num_devices < max_available_devices))
				strcpy(obox.bluetooth[i].devices[num_devices++].name, line);
			strcpy(line, "");
			pclose(bleNode);

			//Paired device
			//Extracting the address of paired device which is in 2nd column
			bleNode = popen("bluetoothctl devices Paired | awk \'{print $2}\'", "r");
			if (bleNode == NULL)
				perror("File error");
			if (fgets(line, sizeof(line), bleNode) == NULL)
				perror("Bluetooth paired address fgets error");
			strcpy(obox.bluetooth[i].pair.address, line);
			strcpy(line, "");
			pclose(bleNode);
			//Extracting the name of paried device which is in 3rd column
			bleNode = popen("bluetoothctl devices Paired | awk \'{print $3}\'", "r");
			if (bleNode == NULL)
				perror("File error");
			if (fgets(line, sizeof(line), bleNode) == NULL)
				perror("Bluetooth paired name fgets error");
			strcpy(obox.bluetooth[i].pair.name, line);
			pclose(bleNode);

			if (system("bluetoothctl scan off > /dev/null") == -1)
				perror("System error");
		}
	}
}

void update_vtox_stats(void)
{
	FILE *v2xFile;
	char num_of_v2x_ifaces[2], line[50], interface_buffer[10], ifconfig_init[100] = "ifconfig", command_search[100], interface_name[10], llc_init[100] = "llc ";
	int x, i = 0;

	//-c option in grep will tell the number of interfaces with "cw-llc" string in them.
	v2xFile = popen("ifconfig | grep -c cw-llc", "r");
	if (v2xFile == NULL)
		perror("File error");

	if (fgets(num_of_v2x_ifaces, sizeof(num_of_v2x_ifaces), v2xFile) == NULL)
		perror("V2X interface number fgets error");
	x = atoi(num_of_v2x_ifaces);
	obox.num_vtox_ifaces = x;
	pclose(v2xFile);

	//Interface Name
	v2xFile = popen("ifconfig | grep \"cw-llc\" | awk '{print $1}'", "r");
	if (v2xFile == NULL)
		perror("File error");
	while (fgets(line, sizeof(line), v2xFile)) {
		//Names are in the format 1. X\n  2. Y\n 3. Z\n
		//Manipulating strings to store only the names.
		char temp_name[50];
		int p = 0;

		for (p = 0; line[p] != '\n'; p++)
			temp_name[p] = line[p];
		temp_name[p] = '\0';
		strcpy(obox.vtox[i++].ifname, temp_name);
	}
	pclose(v2xFile);

	//Loop runs for number of interfaces listed
	for (i = 0; i < x; i++) {
		if (((is_gui_enabled == true) && (is_dynamic_mode == false)) || (is_interactive_cli_cmd == true)) {
			/*      These flags check is more verbosity is required or not. Two cases will be generated when these extra parameters would be required:
			 *      1) Static View Page of GUI enabled: Requires verbosity level 2 (all params get listed)
			 *      2) Interactive CLI mode: Viewing V2X statically
			 */

			//Device ID
			strcpy(llc_init, "llc -i ");
			sprintf(interface_buffer, "%d", i);
			strcat(llc_init, interface_buffer);
			//Device ID is stored in the 5th column where it is displayed using llc version, so awk extracts the 5th column.
			strcpy(command_search, " version | grep \"Device ID\" | awk '{print $5}'");
			strcat(llc_init, command_search);
			v2xFile = popen(llc_init, "r");
			if (v2xFile == NULL)
				perror("File error");
			if (fgets(line, sizeof(line), v2xFile) == NULL)
				perror("V2X Device ID fgets error");
			strcpy(obox.vtox[i].deviceid, line);
			pclose(v2xFile);

			//Protocol Interface
			strcpy(llc_init, "llc -i ");
			sprintf(interface_buffer, "%d", i);
			strcat(llc_init, interface_buffer);
			//Protocol Interface is stored in the 4th column where it is displayed using llc version, so awk extracts the 4th column.
			strcpy(command_search, " version | grep \"Interface\" | awk '{print $4}'");
			strcat(llc_init, command_search);
			v2xFile = popen(llc_init, "r");
			if (v2xFile == NULL)
				perror("File error");
			if (fgets(line, sizeof(line), v2xFile) == NULL)
				perror("V2X Interface fgets error");
			strcpy(obox.vtox[i].interface, line);
			pclose(v2xFile);

			//Channel
			strcpy(llc_init, "llc -i ");
			sprintf(interface_buffer, "%d", i);
			strcat(llc_init, interface_buffer);
			//Channel Name is stored in the 2nd column where it is displayed using llc chconfig, so awk extracts the 2nd column.
			strcpy(command_search, " chconfig | grep Channel | awk '{print $2}'");
			strcat(llc_init, command_search);
			v2xFile = popen(llc_init, "r");
			if (v2xFile == NULL)
				perror("File error");
			if (fgets(line, sizeof(line), v2xFile) == NULL)
				perror("V2X Channel fgets error");
			strcpy(obox.vtox[i].channel, line);
			pclose(v2xFile);
		}

		//MAC Address
		strcpy(llc_init, "llc -i ");
		sprintf(interface_buffer, "%d", i);
		strcat(llc_init, interface_buffer);
		//MAC Address is stored in the 3rd column where it is displayed using llc chconfig, so awk extracts the 3rd column.
		strcpy(command_search, " chconfig | grep MAC | awk '{print $3}'");
		strcat(llc_init, command_search);
		v2xFile = popen(llc_init, "r");
		if (v2xFile == NULL)
			perror("File error");
		if (fgets(line, sizeof(line), v2xFile) == NULL)
			perror("V2X mac fgets error");
		strcpy(obox.vtox[i].macaddr, line);
		pclose(v2xFile);

		//RX Bytes
		strcpy(ifconfig_init, "ifconfig ");
		strcpy(interface_name, obox.vtox[i].ifname);
		strcat(ifconfig_init, interface_name);
		//The bytes are stored in the second column where RX bytes:XYZ is displayed.
		//Awk gives the 2nd column and cut slices the output after ':' hence bytes are received
		strcpy(command_search, " | grep \"RX bytes\" | awk '{print $2}' | cut -d ':' -f 2");
		strcat(ifconfig_init, command_search);
		v2xFile = popen(ifconfig_init, "r");
		if (v2xFile == NULL)
			perror("File error");
		if (fgets(line, sizeof(line), v2xFile) == NULL)
			perror("V2X rx fgets error");
		int prev_rx = obox.vtox[i].rxbytes;

		obox.vtox[i].rxbytes = atoi(line);
		pclose(v2xFile);

		//TX Bytes
		strcpy(ifconfig_init, "ifconfig ");
		strcat(ifconfig_init, interface_name);
		//The bytes are stored in the sixth column where TX bytes:XYZ is displayed.
		//Awk gives the 6th column and cut slices the output after ':' hence bytes are received
		strcpy(command_search, " | grep \"TX bytes\" | awk '{print $6}' | cut -d ':' -f 2");
		strcat(ifconfig_init, command_search);
		v2xFile = popen(ifconfig_init, "r");
		if (v2xFile == NULL)
			perror("File error");
		if (fgets(line, sizeof(line), v2xFile) == NULL)
			perror("V2X tx fgets error");
		int prev_tx = obox.vtox[i].txbytes;

		obox.vtox[i].txbytes = atoi(line);
		pclose(v2xFile);

		//RX Dropped
		strcpy(ifconfig_init, "ifconfig ");
		strcat(ifconfig_init, interface_name);
		//RX dropped is stored in the 4th column where it is displayed using ifconfig, so awk extracts the 4th column.
		strcpy(command_search, " | grep \"RX packets\" | awk '{print $4}' | cut -d ':' -f 2");
		strcat(ifconfig_init, command_search);
		v2xFile = popen(ifconfig_init, "r");
		if (v2xFile == NULL)
			perror("File error");
		if (fgets(line, sizeof(line), v2xFile) == NULL)
			perror("V2X RX dropped fgets error");
		obox.vtox[i].rxdropped = atoi(line);
		pclose(v2xFile);

		//RX errors
		strcpy(ifconfig_init, "ifconfig ");
		strcat(ifconfig_init, interface_name);
		//RX errors is stored in the 3rd column where it is displayed using ifconfig, so awk extracts the 3rd column.
		strcpy(command_search, " | grep \"RX packets\" | awk '{print $3}' | cut -d ':' -f 2");
		strcat(ifconfig_init, command_search);
		v2xFile = popen(ifconfig_init, "r");
		if (v2xFile == NULL)
			perror("File error");
		if (fgets(line, sizeof(line), v2xFile) == NULL)
			perror("V2X RX errors fgets error");
		obox.vtox[i].rxerrors = atoi(line);
		pclose(v2xFile);

		//TX dropped
		strcpy(ifconfig_init, "ifconfig ");
		strcat(ifconfig_init, interface_name);
		//TX dropped is stored in the 4th column where it is displayed using ifconfig, so awk extracts the 4th column.
		strcpy(command_search, " | grep \"TX packets\" | awk '{print $4}' | cut -d ':' -f 2");
		strcat(ifconfig_init, command_search);
		v2xFile = popen(ifconfig_init, "r");
		if (v2xFile == NULL)
			perror("File error");
		if (fgets(line, sizeof(line), v2xFile) == NULL)
			perror("V2X TX dropped fgets error");
		obox.vtox[i].txdropped = atoi(line);
		pclose(v2xFile);

		//TX errors
		strcpy(ifconfig_init, "ifconfig ");
		strcat(ifconfig_init, interface_name);
		//TX errors is stored in the 3rd column where it is displayed using ifconfig, so awk extracts the 3rd column.
		strcpy(command_search, " | grep \"TX packets\" | awk '{print $3}' | cut -d ':' -f 2");
		strcat(ifconfig_init, command_search);
		v2xFile = popen(ifconfig_init, "r");
		if (v2xFile == NULL)
			perror("File error");
		if (fgets(line, sizeof(line), v2xFile) == NULL)
			perror("V2X TX errors fgets error");
		obox.vtox[i].txerrors = atoi(line);
		pclose(v2xFile);

		if (((is_gui_enabled == true) && (is_dynamic_mode == false)) || (is_interactive_cli_cmd == true)) {
			//Flags checked to add verbosity

			//Channel number
			strcpy(llc_init, "llc -i ");
			sprintf(interface_buffer, "%d", i);
			strcat(llc_init, interface_buffer);
			//Channel number is stored in the 2nd column where it is displayed using llc chconfig, so awk extracts the 2nd column.
			strcpy(command_search, " chconfig | grep ChannelNumber | awk '{print $2}'");
			strcat(llc_init, command_search);
			v2xFile = popen(llc_init, "r");
			if (v2xFile == NULL)
				perror("File error");
			if (fgets(line, sizeof(line), v2xFile) == NULL)
				perror("V2X Channel number fgets error");
			obox.vtox[i].channelnum = atoi(line);
			pclose(v2xFile);

			//Frequency
			strcpy(llc_init, "llc -i ");
			sprintf(interface_buffer, "%d", i);
			strcat(llc_init, interface_buffer);
			//FCS Pass Frequency grepped (llc rxphylast)
			strcpy(command_search, " rxphylast | grep Freq | awk '{print $3}'");
			strcat(llc_init, command_search);
			v2xFile = popen(llc_init, "r");
			if (v2xFile == NULL)
				perror("File error");
			if (fgets(line, sizeof(line), v2xFile) == NULL)
				perror("V2X Frequency fgets error");
			obox.vtox[i].frequency = atoi(line);
			pclose(v2xFile);

			//RSSI 1
			strcpy(llc_init, "llc -i ");
			sprintf(interface_buffer, "%d", i);
			strcat(llc_init, interface_buffer);
			//FCS Pass RSSI for Antenna 1 grepped (llc rxphylast)
			strcpy(command_search, " rxphylast | grep \"RSSI Ant 1\" | awk '{print $4}'");
			strcat(llc_init, command_search);
			v2xFile = popen(llc_init, "r");
			if (v2xFile == NULL)
				perror("File error");
			if (fgets(line, sizeof(line), v2xFile) == NULL)
				perror("V2X RSSI Ant 1 fgets error");
			obox.vtox[i].rssi1 = atoi(line);
			pclose(v2xFile);

			//RSSI 2
			strcpy(llc_init, "llc -i ");
			sprintf(interface_buffer, "%d", i);
			strcat(llc_init, interface_buffer);
			//FCS Pass RSSI for Antenna 2 grepped (llc rxphylast)
			strcpy(command_search, " rxphylast | grep \"RSSI Ant 2\" | awk '{print $4}'");
			strcat(llc_init, command_search);
			v2xFile = popen(llc_init, "r");
			if (v2xFile == NULL)
				perror("File error");
			if (fgets(line, sizeof(line), v2xFile) == NULL)
				perror("V2X RSSI Ant 2 fgets error");
			obox.vtox[i].rssi2 = atoi(line);
			pclose(v2xFile);

			//Noise 1
			strcpy(llc_init, "llc -i ");
			sprintf(interface_buffer, "%d", i);
			strcat(llc_init, interface_buffer);
			//FCS Pass Noise for Antenna 1 grepped (llc rxphylast)
			strcpy(command_search, " rxphylast | grep \"Noise S2 Ant 1\" | awk '{print $5}'");
			strcat(llc_init, command_search);
			v2xFile = popen(llc_init, "r");
			if (v2xFile == NULL)
				perror("File error");
			if (fgets(line, sizeof(line), v2xFile) == NULL)
				perror("V2X Noise Ant 1 fgets error");
			obox.vtox[i].noise1 = atoi(line);
			pclose(v2xFile);

			//Noise 2
			strcpy(llc_init, "llc -i ");
			sprintf(interface_buffer, "%d", i);
			strcat(llc_init, interface_buffer);
			//FCS Pass Noise for Antenna 2 grepped (llc rxphylast)
			strcpy(command_search, " rxphylast | grep \"Noise S2 Ant 2\" | awk '{print $5}'");
			strcat(llc_init, command_search);
			v2xFile = popen(llc_init, "r");
			if (v2xFile == NULL)
				perror("File error");
			if (fgets(line, sizeof(line), v2xFile) == NULL)
				perror("V2X Noise Ant 2 fgets error");
			obox.vtox[i].noise2 = atoi(line);
			pclose(v2xFile);
		}

		//RX Rate
		//Current time stamp is noted and is compared with the previous updated time stamp to get the time difference.
		clock_t current_time = clock();
		//The difference is divided by predefined CLOCKS_PER_SEC as the difference is in terms of CPU cycles elapsed.
		double time_difference = (((double) (current_time - obox.time_stamp)) / CLOCKS_PER_SEC);
		int rx_bytes_diff = obox.vtox[i].rxbytes - prev_rx;
		//The value is divided by 1000 to make the unit kBps instead of Bps.
		obox.vtox[i].rxrate = ((rx_bytes_diff / time_difference) / 1000);

		//TX Rate
		int tx_bytes_diff = obox.vtox[i].txbytes - prev_tx;

		obox.vtox[i].txrate = ((tx_bytes_diff / time_difference) / 1000);

		//CPU Temperature
		strcpy(llc_init, "cat /sys/class/thermal/thermal_zone");
		sprintf(interface_buffer, "%d", i);
		strcat(llc_init, interface_buffer);
		strcpy(command_search, "/temp");
		strcat(llc_init, command_search);
		v2xFile = popen(llc_init, "r");
		if (v2xFile == NULL)
			perror("File error");
		if (fgets(line, sizeof(line), v2xFile) == NULL)
			perror("V2X CPU Temperature fgets error");
		//Value received converted to room temperature degree celsius
		float temp = (atof(line) / 1000);

		obox.vtox[i].cputemp = temp;
		pclose(v2xFile);

		if (((is_gui_enabled == true) && (is_dynamic_mode == false)) || (is_interactive_cli_cmd == true)) {
			//Flags checked to add verbosity

			//Temperature of Antenna 1
			strcpy(llc_init, "llc -i ");
			sprintf(interface_buffer, "%d", i);
			strcat(llc_init, interface_buffer);
			//Temperature of Antenna 1 is stored in the 2nd column where it is displayed using llc temp, so awk extracts the 2nd column.
			strcpy(command_search, " temp | grep TempPAAnt1 | awk '{print $2}'");
			strcat(llc_init, command_search);
			v2xFile = popen(llc_init, "r");
			if (v2xFile == NULL)
				perror("File error");
			if (fgets(line, sizeof(line), v2xFile) == NULL)
				perror("V2X Ant 1 temp fgets error");
			obox.vtox[i].tempant1 = atof(line);
			pclose(v2xFile);

			//Temperature of Antenna 2
			strcpy(llc_init, "llc -i ");
			sprintf(interface_buffer, "%d", i);
			strcat(llc_init, interface_buffer);
			//Temperature of Antenna 2 is stored in the 2nd column where it is displayed using llc temp, so awk extracts the 2nd column.
			strcpy(command_search, " temp | grep TempPAAnt2 | awk '{print $2}'");
			strcat(llc_init, command_search);
			v2xFile = popen(llc_init, "r");
			if (v2xFile == NULL)
				perror("File error");
			if (fgets(line, sizeof(line), v2xFile) == NULL)
				perror("V2X Ant 2 temp fgets error");
			obox.vtox[i].tempant2 = atof(line);
			pclose(v2xFile);

			//There are two types of channel intervals operating in V2X: Service Channel Interval and Control Channel Interval
			//This sets the cch and sch channels is they're not up already
			v2xFile = popen("ifconfig llc-cch-ipv6 | grep -o UP", "r");
			if (v2xFile == NULL)
				perror("File error");
			if (fgets(line, sizeof(line), v2xFile) == NULL)
				perror("LLC-CCH-IPv6 not UP");
			if (strncmp("UP", line, 2) != 0) {
				if (system("ifconfig llc-cch-ipv6 fc00::1/64 up") == -1)
					perror("System error");
			}
			pclose(v2xFile);

			v2xFile = popen("ifconfig llc-sch-ipv6 | grep -o UP", "r");
			if (v2xFile == NULL)
				perror("File error");
			if (fgets(line, sizeof(line), v2xFile) == NULL)
				perror("LLC-SCH-IPv6 not UP");
			if (strncmp("UP", line, 2) != 0) {
				if (system("ifconfig llc-sch-ipv6 fc00::1/64 up") == -1)
					perror("System error");
			}
			pclose(v2xFile);

			//MAC Address
			//MAC Address for SCH Channel is stored in the 5th column where it is displayed using ifconfig, so awk extracts the 5th column.
			v2xFile = popen("ifconfig llc-sch-ipv6 | grep HWaddr | awk '{print $5}'", "r");
			if (v2xFile == NULL)
				perror("File error");
			if (fgets(line, sizeof(line), v2xFile) == NULL)
				perror("V2X SCH Mac fgets error");
			strcpy(obox.vtox[i].sch.macaddr, line);
			pclose(v2xFile);

			//RX bytes
			//RX bytes for SCH Channel is stored in the 2nd column where it is displayed using ifconfig, so awk extracts the 2nd column.
			v2xFile = popen("ifconfig llc-sch-ipv6 | grep \"RX bytes\" | awk '{print $2}' | cut -d ':' -f 2", "r");
			if (v2xFile == NULL)
				perror("File error");
			if (fgets(line, sizeof(line), v2xFile) == NULL)
				perror("V2X SCH RX fgets error");
			obox.vtox[i].sch.rxbytes = atoi(line);
			pclose(v2xFile);

			//TX bytes
			//TX bytes for SCH Channel is stored in the 6th column where it is displayed using ifconfig, so awk extracts the 6th column.
			v2xFile = popen("ifconfig llc-sch-ipv6 | grep \"TX bytes\" | awk '{print $6}' | cut -d ':' -f 2", "r");
			if (v2xFile == NULL)
				perror("File error");
			if (fgets(line, sizeof(line), v2xFile) == NULL)
				perror("V2X SCH TX fgets error");
			obox.vtox[i].sch.txbytes = atoi(line);
			pclose(v2xFile);

			//Inet6 address
			//Inet6 address for SCH Channel is stored in the 3rd column where it is displayed using ifconfig, so awk extracts the 3rd column.
			v2xFile = popen("ifconfig llc-sch-ipv6 | grep Scope:Link | awk '{print $3}'", "r");
			if (v2xFile == NULL)
				perror("File error");
			if (fgets(line, sizeof(line), v2xFile) == NULL)
				perror("V2X SCH IP6 fgets error");
			if (sizeof(line) > 1) {
				char temp_address_inet6[INET6_ADDRSTRLEN] = {'\0'};
				int l = 0;

				//Address is in the form of X:X:X:X:X/Y. Manipulating strings to extract address before '/'.
				for (l = 0; line[l] != '/'; l++)
					temp_address_inet6[l] = line[l];
				temp_address_inet6[l] = '\0';
				if (inet_pton(AF_INET6, temp_address_inet6, &obox.vtox[i].sch.inet6_addr) != 1)
					perror("Inet6 conversion error");
			}
			pclose(v2xFile);

			//Control Channel Interval values

			//MAC Address
			//MAC Address for CCH Channel is stored in the 5th column where it is displayed using ifconfig, so awk extracts the 5th column.
			v2xFile = popen("ifconfig llc-cch-ipv6 | grep HWaddr | awk '{print $5}'", "r");
			if (v2xFile == NULL)
				perror("File error");
			if (fgets(line, sizeof(line), v2xFile) == NULL)
				perror("V2X CCH Mac fgets error");
			strcpy(obox.vtox[i].cch.macaddr, line);
			pclose(v2xFile);

			//RX bytes
			//RX bytes for CCH Channel is stored in the 2nd column where it is displayed using ifconfig, so awk extracts the 2nd column.
			v2xFile = popen("ifconfig llc-cch-ipv6 | grep \"RX bytes\" | awk '{print $2}' | cut -d ':' -f 2", "r");
			if (v2xFile == NULL)
				perror("File error");
			if (fgets(line, sizeof(line), v2xFile) == NULL)
				perror("V2X CCH RX fgets error");
			obox.vtox[i].cch.rxbytes = atoi(line);
			pclose(v2xFile);

			//TX bytes
			//TX bytes for SCH Channel is stored in the 6th column where it is displayed using ifconfig, so awk extracts the 6th column.
			v2xFile = popen("ifconfig llc-cch-ipv6 | grep \"TX bytes\" | awk '{print $6}' | cut -d ':' -f 2", "r");
			if (v2xFile == NULL)
				perror("File error");
			if (fgets(line, sizeof(line), v2xFile) == NULL)
				perror("V2X CCH TX fgets error");
			obox.vtox[i].cch.txbytes = atoi(line);
			pclose(v2xFile);

			//Inet6 address
			//Inet6 Address for SCH Channel is stored in the 3rd column where it is displayed using ifconfig, so awk extracts the 3rd column.
			v2xFile = popen("ifconfig llc-cch-ipv6 | grep Scope:Link | awk '{print $3}'", "r");
			if (v2xFile == NULL)
				perror("File error");
			if (fgets(line, sizeof(line), v2xFile) == NULL)
				perror("V2X CCH IP6 fgets error");
			if (sizeof(line) > 1) {
				char temp_address_inet6[INET6_ADDRSTRLEN] = {'\0'};
				int l = 0;

				//Address is in the form of X:X:X:X:X/Y. Manipulating strings to extract address before '/'.
				for (l = 0; line[l] != '/'; l++)
					temp_address_inet6[l] = line[l];
				temp_address_inet6[l] = '\0';
				if (inet_pton(AF_INET6, temp_address_inet6, &obox.vtox[i].cch.inet6_addr) != 1)
					perror("Inet6 conversion error");
			}
			pclose(v2xFile);
		}
	}
}

void update_eth_stats(void)
{
	FILE *ethFile;
	char num_of_eth_ifaces[2], line[50], ifconfig_init[100] = "ifconfig ", command_search[100], interface_name[10];
	int x, i = 0;

	//-c option in grep will tell the number of interfaces with "eth" string in them.
	ethFile = popen("ifconfig | grep -c eth", "r");
	if (ethFile == NULL)
		perror("File error");

	if (fgets(num_of_eth_ifaces, sizeof(num_of_eth_ifaces), ethFile) == NULL)
		perror("Ethernet interface number fgets error");
	x = atoi(num_of_eth_ifaces);
	obox.num_eth_ifaces = x;
	pclose(ethFile);

	//Name
	ethFile = popen("ifconfig | grep \"eth\" | awk '{print $1}'", "r");
	if (ethFile == NULL)
		perror("File error");
	while (fgets(line, sizeof(line), ethFile)) {
		//Names are in the format 1. X\n  2. Y\n 3. Z\n
		//Manipulating strings to store only the names.
		char temp_name[50];
		int p = 0;

		for (p = 0; line[p] != '\n'; p++)
			temp_name[p] = line[p];
		temp_name[p] = '\0';
		strcpy(obox.eth_faces[i++].ifname, temp_name);
	}
	pclose(ethFile);

	//Loop runs for number of interfaces listed
	for (i = 0; i < x; i++) {

		//MAC address
		strcpy(interface_name, obox.eth_faces[i].ifname);
		strcpy(ifconfig_init, "ifconfig ");
		strcat(ifconfig_init, interface_name);
		//MAC Address is stored in the 5th column where it is displayed using ifconfig, so awk extracts the 5th column.
		strcpy(command_search, " | grep HWaddr | awk '{print $5}'");
		strcat(ifconfig_init, command_search);
		ethFile = popen(ifconfig_init, "r");
		if (ethFile == NULL)
			perror("File error");
		if (fgets(line, sizeof(line), ethFile) == NULL)
			perror("Ethernet mac fgets error");
		strcpy(obox.eth_faces[i].macaddr, line);
		pclose(ethFile);

		//RX bytes
		strcpy(ifconfig_init, "ifconfig ");
		strcat(ifconfig_init, interface_name);
		//The bytes are stored in the second column where RX bytes:XYZ is displayed.
		strcpy(command_search, " | grep \"RX bytes\" | awk '{print $2}'");
		strcat(ifconfig_init, command_search);
		ethFile = popen(ifconfig_init, "r");
		if (ethFile == NULL)
			perror("File error");
		if (fgets(line, sizeof(line), ethFile) == NULL)
			perror("Ethernet rx fgets error");
		//RX bytes stored in the form of bytes:XYZ; Manipulating string to extract number of bytes only
		int j = 0, prev_rx, prev_tx;
		char rxbytes[10], txbytes[10];

		for (int k = 6; line[k] != '\0'; k++, j++)
			rxbytes[j] = line[k];
		rxbytes[j] = '\0';
		prev_rx = obox.eth_faces[i].rxbytes;
		//For RX rate calculation
		obox.eth_faces[i].rxbytes = atoi(rxbytes);
		pclose(ethFile);

		//TX bytes
		strcpy(ifconfig_init, "ifconfig ");
		strcat(ifconfig_init, interface_name);
		//Bytes stored in 6th column. Using awk to extract 6th column.
		strcpy(command_search, " | grep \"TX bytes\" | awk '{print $6}'");
		strcat(ifconfig_init, command_search);
		ethFile = popen(ifconfig_init, "r");
		if (ethFile == NULL)
			perror("File error");
		if (fgets(line, sizeof(line), ethFile) == NULL)
			perror("Ethernet TX fgets error");
		//TX bytes stored in the form of bytes:XYZ; Manipulating string to extract number of bytes only
		//Reinitialising int j to 0 to read from the start of line.
		j = 0;

		for (int k = 6; line[k] != '\0'; k++, j++)
			txbytes[j] = line[k];
		txbytes[j] = '\0';
		//Storing previous TX bytes for TX rate calculation
		prev_tx = obox.eth_faces[i].txbytes;
		obox.eth_faces[i].txbytes = atoi(txbytes);
		pclose(ethFile);

		//RX Dropped
		strcpy(ifconfig_init, "ifconfig ");
		strcat(ifconfig_init, interface_name);
		//Bytes stored in 4th column. Using awk to extract 4th column.
		strcpy(command_search, " | grep \"RX packets\" | awk '{print $4}' | cut -d ':' -f 2");
		strcat(ifconfig_init, command_search);
		ethFile = popen(ifconfig_init, "r");
		if (ethFile == NULL)
			perror("File error");
		if (fgets(line, sizeof(line), ethFile) == NULL)
			perror("Ethernet RX dropped fgets error");
		obox.eth_faces[i].rxdropped = atoi(line);
		pclose(ethFile);

		//RX Errors
		strcpy(ifconfig_init, "ifconfig ");
		strcat(ifconfig_init, interface_name);
		//Bytes stored in 3rd column. Using awk to extract 3rd column.
		strcpy(command_search, " | grep \"RX packets\" | awk '{print $3}' | cut -d ':' -f 2");
		strcat(ifconfig_init, command_search);
		ethFile = popen(ifconfig_init, "r");
		if (ethFile == NULL)
			perror("File error");
		if (fgets(line, sizeof(line), ethFile) == NULL)
			perror("Ethernet RX errors fgets error");
		obox.eth_faces[i].rxerrors = atoi(line);
		pclose(ethFile);

		//TX Dropped
		strcpy(ifconfig_init, "ifconfig ");
		strcat(ifconfig_init, interface_name);
		//Bytes stored in 4th column. Using awk to extract 4th column.
		strcpy(command_search, " | grep \"TX packets\" | awk '{print $4}' | cut -d ':' -f 2");
		strcat(ifconfig_init, command_search);
		ethFile = popen(ifconfig_init, "r");
		if (ethFile == NULL)
			perror("File error");
		if (fgets(line, sizeof(line), ethFile) == NULL)
			perror("Ethernet TX dropped fgets error");
		obox.eth_faces[i].txdropped = atoi(line);
		pclose(ethFile);

		//TX Errors
		strcpy(ifconfig_init, "ifconfig ");
		strcat(ifconfig_init, interface_name);
		//Bytes stored in 3rd column. Using awk to extract 3rd column.
		strcpy(command_search, " | grep \"TX packets\" | awk '{print $3}' | cut -d ':' -f 2");
		strcat(ifconfig_init, command_search);
		ethFile = popen(ifconfig_init, "r");
		if (ethFile == NULL)
			perror("File error");
		if (fgets(line, sizeof(line), ethFile) == NULL)
			perror("Ethernet TX errors fgets error");
		obox.eth_faces[i].txerrors = atoi(line);
		pclose(ethFile);

		//RX Rate
		//Current time stamp is noted and is compared with the previous updated time stamp to get the time difference.
		clock_t current_time = clock();
		//The difference is divided by predefined CLOCKS_PER_SEC as the difference is in terms of CPU cycles elapsed.
		double time_difference = (((double) (current_time - obox.time_stamp)) / CLOCKS_PER_SEC);
		int rx_bytes_diff = obox.eth_faces[i].rxbytes - prev_rx;
		//The value is divided by 1000 to make the unit kBps instead of Bps.
		obox.eth_faces[i].rxrate = ((rx_bytes_diff / time_difference) / 1000);

		//TX Rate
		int tx_bytes_diff = obox.eth_faces[i].txbytes - prev_tx;

		obox.eth_faces[i].txrate = ((tx_bytes_diff / time_difference) / 1000);

		//UP
		strcpy(ifconfig_init, "ifconfig ");
		strcat(ifconfig_init, interface_name);
		//-o Option searches for the exact match of the given regex. If UP is found, it will be set true.
		strcpy(command_search, " | grep -o UP");
		strcat(ifconfig_init, command_search);
		ethFile = popen(ifconfig_init, "r");
		if (ethFile == NULL)
			perror("File error");
		if (fgets(line, sizeof(line), ethFile) == NULL)
			perror("Ethernet UP fgets error");
		//Compare the first two characters with UP, to avoid comparison of any extra characters observed on display.
		if (strncmp("UP", line, 2) == 0)
			obox.eth_faces[i].up = true;
		else
			obox.eth_faces[i].up = false;
		pclose(ethFile);

		//RUNNING
		strcpy(ifconfig_init, "ifconfig ");
		strcat(ifconfig_init, interface_name);
		//-o Option searches for the exact match of the given regex. If RUNNING is found, it will be set true.
		strcpy(command_search, " | grep -o RUNNING");
		strcat(ifconfig_init, command_search);
		ethFile = popen(ifconfig_init, "r");
		if (ethFile == NULL)
			perror("File error");
		if (fgets(line, sizeof(line), ethFile) == NULL)
			perror("Ethernet running fgets error");
		//Compare the first 7 characters with RUNNING, to avoid comparison of any extra characters observed on display.
		if (strncmp("RUNNING", line, 7) == 0)
			obox.eth_faces[i].running = true;
		else
			obox.eth_faces[i].running = false;
		pclose(ethFile);

		//MULTICAST
		strcpy(ifconfig_init, "ifconfig ");
		strcat(ifconfig_init, interface_name);
		//-o Option searches for the exact match of the given regex. If MULTICAST is found, it will be set true.
		strcpy(command_search, " | grep -o MULTICAST");
		strcat(ifconfig_init, command_search);
		ethFile = popen(ifconfig_init, "r");
		if (ethFile == NULL)
			perror("File error");
		if (fgets(line, sizeof(line), ethFile) == NULL)
			perror("Ethernet multicast fgets error");
		//Compare the first 9 characters with MULTICAST, to avoid comparison of any extra characters observed on display.
		if (strncmp("MULTICAST", line, 9) == 0)
			obox.eth_faces[i].multicast = true;
		else
			obox.eth_faces[i].multicast = false;
		pclose(ethFile);

		//BROADCAST
		strcpy(ifconfig_init, "ifconfig ");
		strcat(ifconfig_init, interface_name);
		//-o Option searches for the exact match of the given regex. If BROADCAST is found, it will be set true.
		strcpy(command_search, " | grep -o BROADCAST");
		strcat(ifconfig_init, command_search);
		ethFile = popen(ifconfig_init, "r");
		if (ethFile == NULL)
			perror("File error");
		if (fgets(line, sizeof(line), ethFile) == NULL)
			perror("Ethernet broadcast fgets error");
		//Compare the first 9 characters with BROADCAST, to avoid comparison of any extra characters observed on display.
		if (strncmp("BROADCAST", line, 9) == 0)
			obox.eth_faces[i].broadcast = true;
		else
			obox.eth_faces[i].broadcast = false;
		pclose(ethFile);

		//Inet 6 Address
		strcpy(ifconfig_init, "ifconfig ");
		strcat(ifconfig_init, interface_name);
		//Address is stored in 3rd column of display. Awk will extract the 3rd column.
		strcpy(command_search, " | grep inet6 | awk '{print $3}'");
		strcat(ifconfig_init, command_search);
		ethFile = popen(ifconfig_init, "r");
		if (ethFile == NULL)
			perror("File error");
		if (fgets(line, sizeof(line), ethFile) == NULL)
			perror("Ethernet inet6 fgets error");
		//If anything is displayed on output, then only it stores anything in the obox structure address.
		if (sizeof(line) > 1) {
			char temp_address_inet6[INET6_ADDRSTRLEN] = {'\0'};
			int l = 0;

			//Address is in the form of X:X:X:X:X/Y. Manipulating strings to extract address before '/'.
			for (l = 0; line[l] != '/'; l++)
				temp_address_inet6[l] = line[l];
			temp_address_inet6[l] = '\0';
			if (inet_pton(AF_INET6, temp_address_inet6, &obox.eth_faces[i].inet6_addr) != 1)
				perror("Inet6 conversion error");
		}
		pclose(ethFile);

		//Inet 4 Address
		strcpy(ifconfig_init, "ifconfig ");
		strcat(ifconfig_init, interface_name);
		//Address is stored in 2nd column of display. Awk will extract the 2nd column.
		strcpy(command_search, " | grep inet | awk '{print $2}'");
		strcat(ifconfig_init, command_search);
		ethFile = popen(ifconfig_init, "r");
		if (ethFile == NULL)
			perror("File error");
		if (fgets(line, sizeof(line), ethFile) == NULL)
			perror("Ethernet inet4 fgets error");
		char temp_address_inet4[INET_ADDRSTRLEN];
		int y = 0;

		//Address stored in form of addr:X:X:X:X; Manipulating strings to extract address from 5th character onwards.
		for (int q = 5; line[q] != '\0'; q++, y++)
			temp_address_inet4[y] = line[q];
		temp_address_inet4[y-1] = '\0';
		//If anything is displayed on output then only it will store in obox structure address.
		if (strlen(temp_address_inet4) > 1) {
			if (inet_pton(AF_INET, temp_address_inet4, &obox.eth_faces[i].inet4_addr) != 1)
				perror("Inet4 conversion error");
		}
		pclose(ethFile);
	}
}

void update_temp_stats(void)
{
	FILE *tempFile;
	char line[10];

	tempFile = popen("./scripts/temp_sensor.sh", "r");
	if (tempFile == NULL)
		perror("File error");
	if (fgets(line, sizeof(line), tempFile) == NULL)
		perror("Temp sensor fgets error");
	obox.temp_sense = atof(line);
	pclose(tempFile);
}

/*      update_gnss_stats_rssi() function is only for querying the rssi parameter of GNSS.
 *      It has been kept separate from the GNSS updation function since RSSI has to be displayed in dynamic mode of CLI hence it required less delay
 *      The GNSS updation function takes 2 seconds to update the entire GNSS structure whereas this function takes 0.7 seconds to update RSSI
 */
void update_gnss_stats_rssi(void)
{
	FILE *gpsFile;
	char line[20];

	//0.7 seconds specifically has been chosen for timeout since with any timeout below 0.7s the input received is incomplete.
	//system() call has been instead of popen() in this case because cat command takes more time to display then popen call takes to read the display hence it returns the call before reading anything.
	if (system("timeout 0.7 cat /dev/ttyLP3 > gnsslogfile") == -1)
		perror("System error");
	gpsFile = popen("./scripts/gnss_rssi.sh", "r");
	if (gpsFile == NULL)
		perror("File error");
	if (fgets(line, sizeof(line), gpsFile) == NULL)
		strcpy(line, "Unavailable");
	if (strlen(line) == 0)
		strcpy(line, "Unavailable");
	strcpy(obox.gps.rssi, line);
	remove("gnsslogfile");
	pclose(gpsFile);
}

//is_empty() function checks if the buffer is empty.
int is_empty(char *buf, int size)
{
	int i;
	//Loop runs till defined size to check if any character is stored. If stored, is_empty returns a false.
	for (i = 0; i < size; i++) {
		if (buf[i] != 0)
			return 0;
	}
	return 1;
}

//receive_cli_tool_cmd function waits for client request in a non blocking fashion for 5000 us.
void receive_cli_tool_cmd(int sockfd)
{
	char cli_buffer[MAX];
	int cli_length;
	struct sockaddr_in client;
	struct timeval read_timeout;

	read_timeout.tv_sec = 0;
	read_timeout.tv_usec = 5000;

	//setsockopt() function enables SO_RCVTIMEO option which makes the server wait for client query for 5000 us else return to updating the obox structure
	if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &read_timeout, sizeof(read_timeout)) < 0)
		perror("\nTimeout failed");

	cli_length = sizeof(client);
	bzero(cli_buffer, MAX);
	recvfrom(sockfd, cli_buffer, sizeof(cli_buffer), 0, (SA *)&client, &cli_length);
	//If no query is received from client side after 5000 us, the function doesn't proceed any further and returns to main()
	if (is_empty(cli_buffer, sizeof(cli_buffer)))
		return;

	/*      The client tool operates in two modes: interactive and non interactive.
	 *       cli_buffer[10] indicates Interactive CLI while cli_buffer[0] indicates non interactive CLI
	 *       -> Interactive mode: Entire query is received (obox_cli -option level) hence cli_buffer[10] extracts the option selected
	 *       -> Non interactive mode: Query received holds only the option and level (-option level) hence cli_buffer[1] extracts the option selected
	 */

	//GNSS function is called only when it is queried for and is not called during routinely updation of obox structure since it has a delay of 2 seconds.
	if (cli_buffer[10] == 'g' || cli_buffer[1] == 'g')
		update_gps_stats();

	//strlen() check is added since any interactive cli command (obox_cli -option -level) will have 'b' in cli_buffer[1] therefore would enter this function.
	//The length of buffer received in case of non interactive will be always less than 5. (-option level -> length = 4 / -option -> length = 2)
	if (cli_buffer[10] == 'b' || (cli_buffer[1] == 'b') && (strlen(cli_buffer) < 5)) {
		if (cli_buffer[1] == 'b') {
			//If GUI is enabled, that is if the request is sent through non interactive mode, is_gui_enabled flag is set.
			is_gui_enabled = true;
			//GUI can be operated in two modes: static and dynamic. Static mode is more comprehensive hence takes more time, dynamic mode needs to be fast hence verbosity level is 1
			if (cli_buffer[3] == '1')
				//If GUI is operated in dynamic mode this option is selected, is_dynamic_mode flag is set which helps in decreasing delay (verbosity set to 1)
				is_dynamic_mode = true;

		} else
			//If request is through interactive mode, is_interactive_cli_cmd flag is set, more params are extracted making the structure more verbose
			is_interactive_cli_cmd = true;

		//Bluetooth updation function is called again to update parameters according to flags set and to send the client tool the updated structure.
		update_ble_stats();
	}

	if (cli_buffer[10] == 'v' || cli_buffer[1] == 'v') {
		if (cli_buffer[1] == 'v') {
			is_gui_enabled = true;
			if (cli_buffer[3] == '1')
				is_dynamic_mode = true;
		} else
			is_interactive_cli_cmd = true;
		update_vtox_stats();
	}

	//The flags are reset after extracting the required params
	is_interactive_cli_cmd = false;
	is_dynamic_mode = false;
	is_gui_enabled = false;

	//Sends the updated obox structure when client sends a request.
	sendto(sockfd, &obox, sizeof(obox), 0, (SA *)&client, cli_length);
}

//init_stat() function initialises the entire obox structure to NULL by inidividually setting all structures to 0 by memset()
void init_stat(void)
{
	//Number of interfaces set to NULL
	obox.num_wifi_ifaces = 0;
	obox.num_ble_ifaces = 0;
	obox.num_eth_ifaces = 0;
	obox.num_vtox_ifaces = 0;

	//All parameters of accelerometer set to NULL
	memset(&obox.acc, 0, sizeof(obox.acc));

	//All parameters of all possible bluetooth interfaces set to NULL
	for (int i = 0; i < MAX_IFACES; i++)
		memset(&obox.bluetooth[i], 0, sizeof(obox.bluetooth[i]));

	//All parameters of gps set to NULL
	memset(&obox.gps, 0, sizeof(obox.gps));

	//All parameters of all possible ethernet interfaces set to NULL
	for (int i = 0; i < MAX_IFACES; i++)
		memset(&obox.eth_faces[i], 0, sizeof(obox.eth_faces[i]));

	//All parameters of all possible Wi-Fi interfaces set to NULL
	for (int i = 0; i < MAX_IFACES; i++)
		memset(&obox.wifi[i], 0, sizeof(obox.wifi[i]));

	//All parameters of all possible V2X interfaces set to NULL
	for (int i = 0; i < MAX_IFACES; i++)
		memset(&obox.vtox[i], 0, sizeof(obox.vtox[i]));

	//All parameters of temperature sensor set to NULL
	memset(&obox.temp_sense, 0, sizeof(obox.temp_sense));

	obox.time_stamp = clock();
}

int main(void)
{
	int sockfd;
	struct sockaddr_in server_address;

	init_stat();
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd == -1) {
		printf("Socket Creation Failed...\n");
		close(sockfd);
		exit(0);
	}
	//setsockopt() function enables SO_REUSEADDR option which makes the socket address reuseable
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0)
		perror("\nsetsockopt(SO_REUSEADDR) failed");

	//Sets server address to zero
	bzero(&server_address, sizeof(server_address));

	server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr = htonl(INADDR_ANY);
	server_address.sin_port = htons(PORT);
	if ((bind(sockfd, (SA *)&server_address, sizeof(server_address))) != 0) {
		printf("\nSocket Bind failed...%d\n", errno);
		close(sockfd);
		exit(0);
	}
	//The obx_system_stats_d is a daemon process hence this looks keeps updating the obox structure infinitely as a background process
	while (1) {
		//receive_cli_tool_cmd checks if any request received by client and sends the updated structure.
		receive_cli_tool_cmd(sockfd);
		update_acc_stats();
		//update_gnss_stats_rssi() function created for grepping only rssi values to reduce timeout.
		update_gnss_stats_rssi();
		update_ble_stats();
		update_eth_stats();
		update_wifi_stats();
		update_vtox_stats();
		update_temp_stats();
		/*      Each of these functions make use of popen() system call to update each params of every structure under obox structure every second.
		 *      The value extracted from popen() is stored in a file (separate for each function) and fgets is used to extract readings line by line.
		 *      Further string manipulation functions have been applied to make the extracted value compatible to datatype of parameter.
		 */
		obox.time_stamp = clock();
	}
	close(sockfd);
	return 0;
}

