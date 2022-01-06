/*
This file contains your prive information: 
- Your Wifi SSID and password
- Your SSL certificate and your private key for your https webserver to work
- Your token, which will be required by all REST requests.

You must rename it as config.h.

To Generate a certificate and a private key, run the following command: 

    openssl req -newkey rsa:1024 -x509 -sha256 -days 3650 -nodes -out cert.txt -keyout key.txt

This will generate two files cert.text and key.txt. 
*/
#ifndef CONFIG__H_
#define CONFIG__H_

#include <Arduino.h>

const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";


const char serverCert[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
content of cert.txt file goes here
-----END CERTIFICATE-----
)EOF";

const char serverKey[] PROGMEM =  R"EOF(
-----BEGIN PRIVATE KEY-----
content of the private key (key.txt file) goes here
-----END PRIVATE KEY-----
)EOF";

const String authenticationToken("INSERT_THE_TOKEN_HERE");

#endif