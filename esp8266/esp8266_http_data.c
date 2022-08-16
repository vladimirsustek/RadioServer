/*
 * esp8266_http_data.c
 *
 *  Created on: Aug 16, 2022
 *      Author: 42077
 */


char *pageIndex ="<!DOCTYPE html>\n<html>\n\
				<body>\n\
				<h1>RDA5807 control page</h1>\n\
				<form action=\"/pageIndex LEDON\">\n\
				<input type=\"submit\" value=\"BluePill LED ON  \">\n\
				</form>\n\
				<br>\n\
				<form action=\"/pageIndex LEDOFF\">\n\
				<input type=\"submit\" value=\"BluePill LED OFF\">\n\
				</form>\n\
				<br>\n\
				<form action=\"/pageIndex DO_INITrn\">\n\
				<input type=\"submit\" value=\"Initialize\">\n\
				</form>\n\
				<br>\n\
				<form action=\"/pageIndex DO_RSETrn\">\n\
				<input type=\"submit\" value=\"Reset\">\n\
				</form>\n\
				<br>\n\
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
				</body></html>";

const char * atCmd = "AT\r\n";
const char * atCmd_RST = "AT+RST\r\n";
const char * atCmd_CWMODE = "AT+CWMODE=1\r\n";
const char * atCmd_CIPMUX = "AT+CIPMUX=1\r\n";
#if STATIC_IP_AND_NEW_WIFI
const char * atCmd_CWSTAIP = "AT+CWSTAIP=0.0.0.0";
const char * atCmd_CWJAP = "AT+CWJAP=\"WIFI?\",\"Password\"";
#endif
const char * atCmd_CIPSERVER = "AT+CIPSERVER=1,80\r\n";
const char * atRsp_OK = "AT+OK";
const char * atRsp_ready = "ready";
const char * atCmd_CIPSEND = "AT+CIPSEND=";
const char * atCmd_CIPCLOSE = "AT+CIPCLOSE=";
