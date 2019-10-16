/* Based on the websocket example and 
the code here http://www.whatimade.today/esp8266-on-websockets-mdns-ota-and-leds/ */
#include <EEPROM.h>

WebSocketsServer webSocket = WebSocketsServer(81);
extern void startSleepTimer(int);
extern int getSleepTimerRemainingTime(void);
extern void disableSleepTimer(void);
extern int getAnimationSpeed(void);


String getStatusString(void){
  String message;
  message.reserve(100);
  message = "H" + String(myHue);
  message = message + ",S" + String(mySaturation);
  message = message + ",V" + String(myValue);
  message = message + ",W" + String(myWhiteLedValue);
  message = message + ",T" + String(getAnimationSpeed());
  message = message + ",F" + String(getSleepTimerRemainingTime()); //Sends a string with the HSV and white led  values to the client website when the conection gets established
  return message;
}



void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t lenght) {

    switch(type) {
        case WStype_DISCONNECTED:
  //          USE_SERIAL.printf("[%u] Disconnected!\n", num);
            break;
        case WStype_CONNECTED:
            {
                IPAddress ip = webSocket.remoteIP(num);
 //               USE_SERIAL.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
        
        // send message to client
        String info = getStatusString();
       
        webSocket.sendTXT(num, info);
        
        info = ESP.getResetInfo();
        webSocket.sendTXT(num, info); //Handy for debugging
        
        info=String("PROGS")+ledPatternNamesList;
        webSocket.sendTXT(num,info);
			}
        break;
        case WStype_TEXT:
            {
  //            USE_SERIAL.printf("[%u] get Text: %s\n", num, payload);
            

            // send message to client
            // webSocket.sendTXT(num, "message here");

            // send data to all connected clients
            // webSocket.broadcastTXT("message here");

            String text = String((char *) &payload[0]);
            Serial.print("\n" + text);
             if (text.startsWith("f")) {
               String fStrVal = (text.substring(text.indexOf("f") + 1, text.length()));
               int fVal = fStrVal.toInt();
               if (fVal != 0){
                 startSleepTimer(fVal);
               }
              }
             if (text.startsWith("a")) {
                String xVal = (text.substring(text.indexOf("a") + 1, text.length()));
                changeLedAnimation(xVal.toInt());
             }
             if (text.startsWith("b")) {
               String xVal = (text.substring(text.indexOf("b") + 1, text.length()));
                changeHue(xVal.toInt());
             }
             if (text.startsWith("c")) {
               String xVal = (text.substring(text.indexOf("c") + 1, text.length()));
               changeSaturation(xVal.toInt());
               }
             if (text.startsWith("d")) {
               String xVal = (text.substring(text.indexOf("d") + 1, text.length()));
               changeRGBIntensity(xVal.toInt());
             }
             if (text.startsWith("e")) {
               String xVal = (text.substring(text.indexOf("e") + 1, text.length()));
               changeWhiteIntensity(xVal.toInt());
             }
             if (text.startsWith("s")) {
               String xVal = (text.substring(text.indexOf("s") + 1, text.length()));
               changeAnimationSpeed(xVal.toInt());  
             }
             if (text.startsWith("g")) {
               String xVal = (text.substring(text.indexOf("g") + 1, text.length()));
               Serial.print("\nGCool " + xVal);
               changeCooling(xVal.toInt());  
               EEPROM.write(27, xVal.toInt());
               eepromCommitted = false; 
             }
             if (text.startsWith("h")) {
               String xVal = (text.substring(text.indexOf("h") + 1, text.length()));
               Serial.print("\nHSpark " + xVal);
               changeSparking(xVal.toInt()); 
               EEPROM.write(28, xVal.toInt());
               eepromCommitted = false;   
             }
             //restart
             if (text.startsWith("restart")) {
               delay(600);
               ESP.reset();
             }
             //save mqtt ip to eeprom
             if (text.startsWith("m")) {
               String strIP = (text.substring(text.indexOf("m") + 1, text.length()));
               int Parts[4] = {0,0,0,0};
                int Part = 0;
                for ( int i=0; i<strIP.length(); i++ )
                {
                 char c = strIP[i];
                  if ( c == '.' )
                  {
                    Part++;
                    continue;
                  }
                  Parts[Part] *= 10;
                  Parts[Part] += c - '0';
                }
                IPAddress ip( Parts[0], Parts[1], Parts[2], Parts[3] );
               EEPROM.write(7, Parts[0]);
               EEPROM.write(8, Parts[1]);
               EEPROM.write(9, Parts[2]);
               EEPROM.write(10, Parts[3]);
               eepromCommitted = false;  
               Serial.print("\nSetting new mqtt ip");
             }
             //save mqtt port to EEPROM
             if (text.startsWith("p")) {
               String strPORT = (text.substring(text.indexOf("p") + 1, text.length()));
               int port = strPORT.toInt();
               uint8_t xlow = port & 0xff;
               uint8_t xhigh = (port >> 8);
               EEPROM.write(11, xhigh);
               EEPROM.write(12, xlow);
               eepromCommitted = false;  
               Serial.print("\nSetting new mqtt port");
             }
             //save mqtt topic to EEPROM (max 15 chars)
             if (text.startsWith("t")) {
              //write to eeprom
              String Topic = (text.substring(text.indexOf("t") + 1, text.length()));
              int charLength=Topic.length();
              int l = Topic.length();
              if (l > 15) { l = 15; }
 
              Serial.println("\nwriting eeprom mqtt topic:");
              for (int i = 0; i < l; ++i)
              {
                EEPROM.write(13+i, Topic[i]);
                Serial.print("Wrote: ");
                Serial.println(Topic[i]);
              }
              String a = "+";
              for (int i = l; i<15; ++i) {
                EEPROM.write(13+i, a[0]);
                Serial.println("Wrote: +");
              }
              eepromCommitted = false; 

            }
            break;
            }
        case WStype_BIN:
            //USE_SERIAL.printf("[%u] get binary lenght: %u\n", num, lenght);
            hexdump(payload, lenght);

            // send message to client
            // webSocket.sendBIN(num, payload, lenght);
            break;
    }

}
