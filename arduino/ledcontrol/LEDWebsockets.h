/* Based on the websocket example and 
the code here http://www.whatimade.today/esp8266-on-websockets-mdns-ota-and-leds/ */

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
             if (text.startsWith("f")) {
               String fStrVal = (text.substring(text.indexOf("f") + 1, text.length()));
               int fVal = fStrVal.toInt();
               if (fVal != 0){
                 startSleepTimer(fVal);
               }
              }
             if (text.startsWith("a")) {
               disableSleepTimer();
               String xVal = (text.substring(text.indexOf("a") + 1, text.length()));
                flickerLed = random(0,NUM_LEDS-1);
                if (myEffect != xVal.toInt()) {  // only do stuff when there was a change
                myEffect = xVal.toInt();
                rainbowHue = myHue;
                EEPROM.write(0, myEffect);       //stores the variable but needs to be committed to EEPROM before being saved - this happens in the loop
                lastChangeTime = millis();
                eepromCommitted = false;
                }
             }
             if (text.startsWith("b")) {
               String xVal = (text.substring(text.indexOf("b") + 1, text.length()));
                if (myHue != xVal.toInt()) {
                myHue = xVal.toInt();
                rainbowHue = myHue;
                EEPROM.write(1, myHue);
                lastChangeTime = millis();
                eepromCommitted = false;
                }
             }
              if (text.startsWith("c")) {
               String xVal = (text.substring(text.indexOf("c") + 1, text.length()));
                if (mySaturation != xVal.toInt()) {
                mySaturation = xVal.toInt();
                EEPROM.write(2, mySaturation);
                lastChangeTime = millis();
                eepromCommitted = false;
                }
             }
              if (text.startsWith("d")) {
               String xVal = (text.substring(text.indexOf("d") + 1, text.length()));
                if (myValue != xVal.toInt()) {
                myValue = xVal.toInt();
                EEPROM.write(3, myValue);
                lastChangeTime = millis();
                eepromCommitted = false;
                }
             }
              if (text.startsWith("e")) {
               String xVal = (text.substring(text.indexOf("e") + 1, text.length()));
                if (myWhiteLedValue != xVal.toInt()) {
                myWhiteLedValue = xVal.toInt();
                EEPROM.write(4, myWhiteLedValue&&255);
                EEPROM.write(5, myWhiteLedValue/256);
                lastChangeTime = millis();
                eepromCommitted = false;
                }
             }
              if (text.startsWith("t")) {
               String xVal = (text.substring(text.indexOf("t") + 1, text.length()));
                if (myAnimationSpeed != xVal.toInt()) {
                myAnimationSpeed = xVal.toInt();
                if(myAnimationSpeed < 10){
                  myAnimationSpeed = 10;
                }
                EEPROM.write(6, myAnimationSpeed&255);
                lastChangeTime = millis();
                eepromCommitted = false;
                }                
              }
             /*currentTime = millis();
              if (currentTime - previousTime > 1000) {
                String websocketStatusMessage = "H" + String(myHue) + ",S" + String(mySaturation) + ",V" + String(myValue);
                webSocket.broadcastTXT(websocketStatusMessage);
                previousTime = currentTime;
               }*/
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
