//Settings server, to easily setup wifi and host files on the ESP8266 SPIFFS. 
//Borrowed heavily from FSBrowser example.


#include "SettingsServer.h"
#include <ESP8266httpUpdate.h>
#include <ESP8266HTTPUpdateServer.h>
#include <FS.h>
#include <ESP8266SSDP.h>

#define DBG_OUTPUT_PORT Serial

const char* pvhostname = "woodblock";

ESP8266WebServer server(80);
//MDNSResponder mdns;
ESP8266HTTPUpdateServer httpUpdater;

void showWifiConfigAPMessage(void);

void handle_root(){
  server.send(200, "text/plain", "Woodblock lighting");
  delay(100);
}


//format bytes
String formatBytes(size_t bytes){
  if (bytes < 1024){
    return String(bytes)+"B";
  } else if(bytes < (1024 * 1024)){
    return String(bytes/1024.0)+"KB";
  } else if(bytes < (1024 * 1024 * 1024)){
    return String(bytes/1024.0/1024.0)+"MB";
  } else {
    return String(bytes/1024.0/1024.0/1024.0)+"GB";
  }
}

String getContentType(String filename){
  if(server.hasArg("download")) return "application/octet-stream";
  else if(filename.endsWith(".htm")) return "text/html";
  else if(filename.endsWith(".html")) return "text/html";
  else if(filename.endsWith(".css")) return "text/css";
  else if(filename.endsWith(".js")) return "application/javascript";
  else if(filename.endsWith(".png")) return "image/png";
  else if(filename.endsWith(".gif")) return "image/gif";
  else if(filename.endsWith(".jpg")) return "image/jpeg";
  else if(filename.endsWith(".ico")) return "image/x-icon";
  else if(filename.endsWith(".xml")) return "text/xml";
  else if(filename.endsWith(".pdf")) return "application/x-pdf";
  else if(filename.endsWith(".zip")) return "application/x-zip";
  else if(filename.endsWith(".gz")) return "application/x-gzip";
  return "text/plain";
}




bool handleFileRead(String path){
  DBG_OUTPUT_PORT.println("handleFileRead: " + path);
  if(path.endsWith("/")) path += "index.htm";
  String contentType = getContentType(path);
  String pathWithGz = path + ".gz";
  if(SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)){
    if(SPIFFS.exists(pathWithGz))
      path += ".gz";
    File file = SPIFFS.open(path, "r");
    size_t sent = server.streamFile(file, contentType);
    file.close();
    return true;
  }
  return false;
}



void handleFileList() {
  if(!server.hasArg("dir")) {server.send(500, "text/plain", "BAD ARGS"); return;}
  
  String path = server.arg("dir");
  DBG_OUTPUT_PORT.println("handleFileList: " + path);
  Dir dir = SPIFFS.openDir(path);
  path = String();

  String output = "[";
  while(dir.next()){
    File entry = dir.openFile("r");
    if (output != "[") output += ',';
    bool isDir = false;
    output += "{\"type\":\"";
    output += (isDir)?"dir":"file";
    output += "\",\"name\":\"";
    output += String(entry.name()).substring(1);
    output += "\"}";
    entry.close();
  }
  
  output += "]";
  server.send(200, "text/json", output);
}

void handle_refresh(void){
  server.send(200, "text/plain", "Refreshing now");
}

void handle_wifisetup(void){
  server.send(200, "text/plain", "Resetting WiFi settings and rebooting, connect to Woodblock_AP to setup new wifi connection.");
  WiFiManager wifiManager;
  wifiManager.resetSettings();

  delay(1000);
    //reset and try again, or maybe put it to deep sleep
    ESP.reset();
}

void handle_brightness(void)
{
    String aStr = server.arg("brightness");
    int brightness = aStr.toInt();
    if ((brightness >= 0) && (brightness <= 100))
    {
      analogWrite(0,brightness);
    }
    server.send(200, "text/plain", "brightness set");
}


void handle_reboot(void)
{
  server.send(200, "text/plain", "restarting.......");
  delay(600);
  ESP.reset();
}
   

void startSettingsServer(void){
    SPIFFS.begin();
  {
    Dir dir = SPIFFS.openDir("/");
    while (dir.next()) {    
      String fileName = dir.fileName();
      size_t fileSize = dir.fileSize();
      DBG_OUTPUT_PORT.printf("FS File: %s, size: %s\n", fileName.c_str(), formatBytes(fileSize).c_str());
    }
    DBG_OUTPUT_PORT.printf("\n");
  }

  ArduinoOTA.setHostname(pvhostname);
  ArduinoOTA.onError([](ota_error_t error) { ESP.restart(); });
  ArduinoOTA.begin();
  
  server.on("/refresh", handle_refresh);
  server.on("/wifisetup",handle_wifisetup);
  server.on("/reboot", handle_reboot);
  server.on("/brightness",handle_brightness);
   //list directory
  server.on("/list", HTTP_GET, handleFileList);
  server.on("/description.xml", HTTP_GET, [](){ //SSDP server added
      SSDP.schema(server.client());
  });

 //called when the url is not defined here
  //use it to load content from SPIFFS
  server.onNotFound([](){
    if(!handleFileRead(server.uri()))
      server.send(404, "text/plain", "FileNotFound");
  });
    httpUpdater.setup(&server); //setup update via webpage

  server.begin();
  Serial.println("HTTP server started");
  //ArduinoOTA handles mdns service
 // if(!mdns.begin(pvhostname, WiFi.localIP())) {
 //   Serial.println("Error setting up MDNS responder!");
 // }
 // Serial.println("mDNS responder started");

 //   mdns.addService("http", "tcp", 80);

} 

void settingsServerTask(void){
  ArduinoOTA.handle();
  server.handleClient();
}

void showWifiConfigAPMessage(WiFiManager *myWiFiManager)
{
  
}

void setupWiFi(void){
    // The extra parameters to be configured (can be either global or just in the setup)
  // After connecting, parameter.getValue() will get you the configured value
  // id/name placeholder/prompt default length
  WiFi.hostname(pvhostname);

  WiFiManager wifiManager;
  wifiManager.setMinimumSignalQuality(10);
  //ifiManager.setAPCallback(showWifiConfigAPMessage);
  wifiManager.setConfigPortalTimeout(180);
  wifiManager.setConnectTimeout(60);
  if (!wifiManager.autoConnect("woodblock_AP")) { 
    Serial.println("failed to connect and hit timeout");
    delay(3000);
    //reset and try again, or maybe put it to deep sleep
    ESP.reset();
    delay(5000);
  }
  
  Serial.println("local ip");
  Serial.println(WiFi.localIP());
  Serial.println(WiFi.gatewayIP());
  Serial.println(WiFi.subnetMask());

}



