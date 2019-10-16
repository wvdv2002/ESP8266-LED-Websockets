#ifdef USE_MQTT
#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>


//Set these definitions.
//IPAddress mqttServerIp(192, 168, 1, 4);
IPAddress mqttServerIp;

//const char* mqttCmdTopic = "woodblock/cmd/#";
//const char* mqttStatTopic = "woodblock/state";
//const char* mqttAnimationNamesTopic = "woodblock/animationNames";
const char* a = "+";
const char* b = "/#";

String mqttCmdTopic = "woodblock/cmd/#";
String mqttStatTopic = "woodblock/state";
String mqttAnimationNamesTopic = "woodblock/animationNames";

WiFiClient EspClient;                    
PubSubClient mqttClient(EspClient);
uint32_t reconnectTimeoutTimer;
bool disableMQTT = 0;
bool mqttConnectedAtLeastOnce = 0;
#define MQTT_MAX_PACKET_SIZE 512
#define MQTT_IGNOREFIRSTCOMMANDS

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  char dataBuf[length+1];
  memcpy(dataBuf, payload, length);
  dataBuf[sizeof(dataBuf)-1] = 0;
  String aTopic = topic;
  String aPayload = dataBuf;
  String aCmd = aTopic.substring(aTopic.lastIndexOf('/')+1);
  int fVal;
  aCmd.toLowerCase();
  aCmd.trim();
  
  Serial.print("\nMessage arrived [");
  Serial.print(aTopic);
  Serial.print("] ");
  Serial.println(aPayload);
  if(length>0){
    if(aCmd == "hue"){
     changeHue(aPayload.toInt());    
    }else if(aCmd=="saturation"){
     changeSaturation(aPayload.toInt());      
    }else if(aCmd=="value"){
     changeRGBIntensity(aPayload.toInt());        
    }else if(aCmd=="white"){
     changeWhiteIntensity(aPayload.toInt());     
    }else if(aCmd=="speed"){
     changeAnimationSpeed(aPayload.toInt());
    }else if(aCmd=="hsv"){
      changeHSV(aPayload.substring(0,aPayload.indexOf(',')).toInt(),aPayload.substring(aPayload.indexOf(',')+1,aPayload.lastIndexOf(',')).toInt(),aPayload.substring(aPayload.lastIndexOf(',')+1).toInt());
    }else if(aCmd=="rgb"){
      changeRGB(aPayload.substring(0,aPayload.indexOf(',')).toInt(),aPayload.substring(aPayload.indexOf(',')+1,aPayload.lastIndexOf(',')).toInt(),aPayload.substring(aPayload.lastIndexOf(',')+1).toInt());
    }else if(aCmd=="demo"){
      demorun = aPayload.toInt();
    }else if(aCmd=="demotime"){
      demotime = aPayload.toInt();
    }else if(aCmd=="firecooling"){
      changeCooling(aPayload.toInt());
    }else if(aCmd=="firesparking"){
      changeSparking(aPayload.toInt());
    }else if(aCmd=="sleep"){
     fVal = aPayload.toInt();
     if (fVal != 0){
      startSleepTimer(fVal);
     }else{
      disableSleepTimer();
     }
    }else if(aCmd=="animation"){
      changeLedAnimation(aPayload.toInt());
    }else{
        Serial.println("Did not understand command");
    }
  }else{
    Serial.println("Did not understand payload");
  }

   
   
}


void mqttPostStatus(void){
  CRGB aColor;
  aColor = CHSV(myHue,mySaturation,myValue);
  String aStringStatus;
  aStringStatus.reserve(129);
  StaticJsonBuffer<256> aStatusBuffer;
  JsonObject& aStatus = aStatusBuffer.createObject();
  aStatus["animation"] = myEffect;
  aStatus["speed"] = myAnimationSpeed;
  aStatus["sleep"] = getSleepTimerRemainingTime();
  aStatus["white"] = myWhiteLedValue;
  aStatus["h"] = myHue;
  aStatus["s"] = mySaturation;
  aStatus["v"] = myValue;
  aStatus["r"] = aColor.red;
  aStatus["g"] = aColor.green;
  aStatus["b"] = aColor.blue;
  aStatus.printTo(aStringStatus);
  char c[mqttStatTopic.length()];
  mqttStatTopic.toCharArray(c, mqttStatTopic.length());
  mqttClient.publish(c,aStringStatus.c_str());
}

void mqttPostAnimationString(void){
  char c[mqttAnimationNamesTopic.length()];
  mqttAnimationNamesTopic.toCharArray(c, mqttAnimationNamesTopic.length());
  mqttClient.publish(c,ledPatternNamesList);
}

void reconnect() {
  // Loop until we're reconnected
  if(!mqttClient.connected()) {
    Serial.print("\nAttempting MQTT connection...");
    // Attempt to connect
    if (mqttClient.connect(pvhostname)) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      mqttPostStatus();
      mqttPostAnimationString();
      // ... and resubscribe
      char c[mqttCmdTopic.length()];
      mqttCmdTopic.toCharArray(c, mqttCmdTopic.length());
      //c[mqttCmdTopic.length()+1] = b[0];
      //c[mqttCmdTopic.length()+2] = b[1];
      Serial.println(c );
      Serial.println(b );
      mqttClient.subscribe(c);
      mqttConnectedAtLeastOnce = 1;
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      if(!mqttConnectedAtLeastOnce){
        disableMQTT = 1;
        Serial.print("Disabled MQTT");
      }
    }
  }
}

String GetMqttTopic() {

  //read to eeprom

  Serial.println("\n\nReading EEPROM ssid");
  String esid;
  for (int i = 13; i < 27; ++i)
  {
   char c = EEPROM.read(i);
   if (c != a[0]) { esid += c; }
  }
  //esid.trim();
  Serial.println(esid.length());
  Serial.print("Mqtt Topic: ");
  Serial.println(esid);
  return esid;
}

void mqttBegin(){
   IPAddress mqttServerIp(EEPROM.read(7), EEPROM.read(8), EEPROM.read(9), EEPROM.read(10));
   Serial.print("Using ip: ");
   Serial.print(EEPROM.read(7) );
   Serial.print("." );
   Serial.print(EEPROM.read(8) );
   Serial.print("." );
   Serial.print(EEPROM.read(9) );
   Serial.print("." );
   Serial.print(EEPROM.read(10));
   String topic = GetMqttTopic();
   Serial.print(topic);
   //char char_array[topic.length()];
   //topic.toCharArray(char_array, topic.length());
   mqttCmdTopic = topic+"/cmd/#/";
   mqttStatTopic = topic+"/state";
   mqttAnimationNamesTopic = topic+"/animationNames";
   Serial.print("\nTopic: "+mqttCmdTopic);
   int port1 = EEPROM.read(11);
   int port2 = EEPROM.read(12);
   int port = port1*256 + port2;
   Serial.print("\nPort: ");
   Serial.print(port);
   mqttClient.setServer(mqttServerIp, port);
   mqttClient.setCallback(mqttCallback);
}

void mqttTask(){
  if(!disableMQTT){
  if (!mqttClient.connected()){

      if((millis()-reconnectTimeoutTimer)>10000) {
        reconnect();
        reconnectTimeoutTimer = millis();
      }
    }else{
      mqttClient.loop();
    }
  }
}
#else
void mqttTask(void){};
void mqttBegin(void){};
void mqttPostStatus(void){};
#endif
