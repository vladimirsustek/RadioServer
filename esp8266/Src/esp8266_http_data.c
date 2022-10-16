/*
 * esp8266_http_data.c
 *
 *  Created on: Aug 16, 2022
 *      Author: 42077
 */

#include "esp8266_http_data.h"

char *pageIndex ="<!DOCTYPE html>\n<html>\n\
				<body>\n\
				<h1>RDA5807 control page</h1>\n\
				<form action=\"/pageIndex DO_INIT\r\n\">\n\
				<input type=\"submit\" value=\"Radio on\">\n\
				</form>\n\
				<br>\n\
				<form action=\"/pageIndex DO_RSET\r\n\">\n\
				<input type=\"submit\" value=\"Radio off\">\n\
				</form>\n\
		        <br>\n\
				<form action=\"/pageIndex ST_VOLM\">\n\
				<input type=\"text\" id=\"volm\" name=\"volm\" value=\"\"><br><br>\n\
				<input type=\"submit\" value=\"SendVolume\">\n\
				</form>\n\
				<br>\n\
				<form action=\"/pageIndex\ ST_FREQ\">\n\
				<input type=\"text\" id=\"freq\" name=\"freq\" value=\"08920\"><br><br>\n\
				<input type=\"submit\" value=\"SendFrequency\">\n\
				</form>\n\
				<br>\n\
				<form action=\"/pageIndex ST_FREQfreq=08920\r\n\">\n\
				<input type=\"submit\" value=\"(AT) RADIO O3\">\n\
				</form>\n\
		        <br>\n\
				<form action=\"/pageIndex ST_FREQfreq=10170\r\n\">\n\
				<input type=\"submit\" value=\"(AT) RADIO FM4\">\n\
				</form>\n\
		        <br>\n\
				<form action=\"/pageIndex ST_FREQfreq=09910\r\n\">\n\
				<input type=\"submit\" value=\"(AT) ANTENNE\">\n\
				</form>\n\
		        <br>\n\
				<form action=\"/pageIndex ST_FREQfreq=09520\r\n\">\n\
				<input type=\"submit\" value=\"(SK) EXPRESS\">\n\
				</form>\n\
		        <br>\n\
				<form action=\"/pageIndex ST_FREQfreq=10780\r\n\">\n\
				<input type=\"submit\" value=\"(CZ) DVOJKA\">\n\
				</form>\n\
		        <br>\n\
				<form action=\"/pageIndex ST_FREQfreq=09770\r\n\">\n\
				<input type=\"submit\" value=\"(CZ) IMPULZ\">\n\
				</form>\n\
		        <br>\n\
				<br>\n\
				<br>\n\
				<br>\n\
				<br>\n\
				<br>\n\
				<br>\n\
				<form action=\"/pageIndex ST_TIME\">\n\
				<input type=\"text\" id=\"time\" name=\"time\" value=\"\"><br><br>\n\
				<input type=\"submit\" value=\"SetTime\">\n\
				</form>\n\
				</body></html>";

const char * atCmd = "AT\r\n";
const char * atCmd_RST = "AT+RST\r\n";
const char * atCmd_CWQAP = "AT+CWQAP\r\n";
const char * atCmd_CWMODE = "AT+CWMODE=1\r\n";
const char * atCmd_CIPMUX = "AT+CIPMUX=1\r\n";
#if STATIC_IP_AND_NEW_WIFI
const char * atCmd_CWSTAIP = "AT+CWSTAIP=0.0.0.0";
#endif
const char * atCmd_CWJAP = "AT+CWJAP=";
const char * atCmd_CIPSERVER = "AT+CIPSERVER=1,80\r\n";
const char * atRsp_OK = "OK";
const char * atRsp_ready = "ready";
const char * atCmd_CIPSEND = "AT+CIPSEND=";
const char * atCmd_CIPCLOSE = "AT+CIPCLOSE=";
const char * atRsp_WifiGotIp = "WIFI GOT IP";
