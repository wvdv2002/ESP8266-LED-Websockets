#ifndef SETTINGSSERVER_H
#define SETTINGSSERVER_H

#include "Arduino.h"
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266WebServer.h>
//#include <ESP8266mDNS.h>
#include <WiFiManager.h>
//#include <WiFiUdp.h>
#include <ArduinoOTA.h>


extern void startSettingsServer(void);
extern int setupWiFi(int timeout);
extern void settingsServerTask(void);
extern const char* pvhostname;
extern bool updateStats;
extern void CheckOTAServer(void);
#endif //SETTINGSSERVER_H
