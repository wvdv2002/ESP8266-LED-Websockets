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
              if (text.startsWith("t")) {
               String xVal = (text.substring(text.indexOf("t") + 1, text.length()));
              changeAnimationSpeed(xVal.toInt());  
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
