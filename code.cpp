/*
MIT License

Copyright (c) [2024] [PAVAN KALSARIYA]

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND.
*/

/*This code monitors temperature and humidity using a DHT sensor. It displays readings on an 
  LCD screen with color-coded alerts for high values. The code also connects to Arduino Cloud 
  to send data and checks Wi-Fi status periodically, updating the LCD and blinking an LED if 
  disconnected.
*/

#include "thingProperties.h"
#include "DHT.h"
#include "rgb_lcd.h"
#include <WiFi.h>

#define DHTPIN 4    
#define DHTTYPE DHT11  

rgb_lcd lcd;
const int colorR = 100;
const int colorG = 100;
const int colorB = 100;

const int tempThreshold = 30;  
const int humThreshold = 70;   
unsigned long previousMillis = 0;
const long interval = 1000;    
unsigned long lastWifiCheck = 0;
const long wifiCheckInterval = 5000; 

DHT dht(DHTPIN, DHTTYPE);

void setup() {
    Serial.begin(9600);
    while (!Serial);  

    dht.begin();
    initProperties();

    ArduinoCloud.begin(ArduinoIoTPreferredConnection);
    setDebugMessageLevel(1);
    ArduinoCloud.printDebugInfo();

    lcd.begin(16, 2);
    lcd.setRGB(colorR, colorG, colorB);
    
    displayWelcomeMessage();  

    delay(2000);  
    lcd.clear();

    pinMode(LED_BUILTIN, OUTPUT); 
}

void loop() {
    unsigned long currentMillis = millis();
    ArduinoCloud.update();

    if (currentMillis - lastWifiCheck >= wifiCheckInterval) {
        lastWifiCheck = currentMillis;
        checkWiFiStatus();
    }

    if (currentMillis - previousMillis >= interval) {
        previousMillis = currentMillis;

        float hum = dht.readHumidity();
        float temp = dht.readTemperature();

        if (isnan(hum) || isnan(temp)) {
            displayError("Sensor Error!");
            blinkRedBacklight();
            return;
        }

        humidity = hum;  
        tEMP = temp;     

        Serial.print("Temp: ");
        Serial.print(temp);
        Serial.println("°C");
        Serial.print("Hum: ");
        Serial.print(hum);
        Serial.println("%");

        updateLCD(temp, hum);
        checkAlerts(temp, hum);
    }
}

void displayWelcomeMessage() {
    lcd.clear();  
    lcd.setCursor(0, 0);
    lcd.print("Welcome to");
    lcd.setCursor(0, 1);
    lcd.print("WeatherStation");
    lcd.setRGB(0, 255, 0);  
}

void updateLCD(float temp, float hum) {
    lcd.clear();  
    lcd.setCursor(0, 0);
    lcd.print("Temp: ");
    lcd.print(temp, 1);  
    lcd.print(" C");

    lcd.setCursor(0, 1);
    lcd.print("Hum:  ");
    lcd.print(hum, 1);   
    lcd.print(" %");
}

void checkAlerts(float temp, float hum) {
    if (temp > tempThreshold) {
        lcd.setRGB(255, 0, 0);  
        Serial.println("Temperature Alert!");
    } else if (hum > humThreshold) {
        lcd.setRGB(0, 0, 255);  
        Serial.println("Humidity Alert!");
    } else {
        lcd.setRGB(colorR, colorG, colorB);  
    }
}

void displayError(const char* message) {
    lcd.clear();  
    lcd.setCursor(0, 0);
    lcd.print("Error:");
    lcd.setCursor(0, 1);
    lcd.print(message);
    Serial.println(message);
}

void blinkRedBacklight() {
    for (int i = 0; i < 3; i++) {  
        lcd.setRGB(255, 0, 0);
        delay(500);
        lcd.setRGB(0, 0, 0);
        delay(500);
    }
    lcd.setRGB(colorR, colorG, colorB);  
}

void checkWiFiStatus() {
    lcd.clear();  
    if (WiFi.status() == WL_CONNECTED) {
        lcd.setCursor(0, 0);
        lcd.print("WiFi: Connected");
        lcd.setCursor(0, 1);
        lcd.print("Signal: ");
        lcd.print(WiFi.RSSI());
        lcd.print(" dBm");
        digitalWrite(LED_BUILTIN, HIGH); 
    } else {
        lcd.setCursor(0, 0);
        lcd.print("WiFi: Disconnected");
        lcd.setCursor(0, 1);
        lcd.print("Signal: N/A");
        blinkBuiltInLED(); 
    }
}

void blinkBuiltInLED() {
    for (int i = 0; i < 10; i++) {  
        digitalWrite(LED_BUILTIN, HIGH);
        delay(200);
        digitalWrite(LED_BUILTIN, LOW);
        delay(200);
    }
}

void onTEMPChange() {
    Serial.println("Temperature changed on IoT Cloud");
}

void onHumidityChange() {
    Serial.println("Humidity changed on IoT Cloud");
}
