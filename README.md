# ESP8266-LED-Websockets
My code for creating a wooden block LED light with RGB + white WS2812 led strips.
This code makes it controllable from any webbrowser, with the files hosted on the ESP8266.
So no external server needed.

#Versions
There are two versions at the moment, the rgbwstrip version is my way of hacking support into the firmware for the SK6812 led, which is a real nice RGBWW strip with the white leds also individually adressable. Fastled has no support for this yet. I now use the fastled library for all animations and calculations etcetera and use the Adafruit Neopixel library to actually drive the leds. Which is a good enough compromise for now. The white leds of the strip are for now only driven as a group. But this is only a matter of changing a small part of the code.


This is still a WIP. The upload via Arduino IDE is broken... As is the mdns server....




This work was forked from https://github.com/TheAustrian/ESP8266-LED-Websockets/
