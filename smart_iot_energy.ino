#define BLYNK_TEMPLATE_ID "TMPL6h-2cmwMN"
#define BLYNK_TEMPLATE_NAME "Quickstart Template"
#define BLYNK_PRINT Serial
 
#include "EmonLib.h"
#include <EEPROM.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 20, 4);
 
const float vCalibration = 41.5;
const float currCalibration = 0.15;

const char auth[] = "6SyNhr96X9PNXQbYjeuuW1HfPLPWXNWp";
const char ssid[] = "010302";
const char pass[] = "1234567890";
 
EnergyMonitor emon;
 
BlynkTimer timer;
 
float kWh = 0.0;
unsigned long lastMillis = millis();
 
const int addrVrms = 0;
const int addrIrms = 4;
const int addrPower = 8;
const int addrKWh = 12;
 
void sendEnergyDataToBlynk();
void readEnergyDataFromEEPROM();
void saveEnergyDataToEEPROM();
 
 
void setup()
{
  Serial.begin(115200);
  Blynk.begin(auth, ssid, pass);
 
  lcd.init();
  lcd.backlight();
 
  EEPROM.begin(32); 
 
  readEnergyDataFromEEPROM();
 
  emon.voltage(35, vCalibration, 1.7); 
  emon.current(34, currCalibration);    
 
  timer.setInterval(5000L, sendEnergyDataToBlynk);

  delay(1000);
}
 
void loop()
{
  Blynk.run();
  timer.run();
}
  
void sendEnergyDataToBlynk()
{
  emon.calcVI(20, 2000);
 
  unsigned long currentMillis = millis();
  kWh += emon.apparentPower * (currentMillis - lastMillis) / 3600000000.0;
  lastMillis = currentMillis;
 
  Serial.printf("Vrms: %.2fV\tIrms: %.4fA\tPower: %.4fW\tkWh: %.5fkWh\n",emon.Vrms, emon.Irms, emon.apparentPower, kWh);
 
  saveEnergyDataToEEPROM();
 
  Blynk.virtualWrite(V0, emon.Vrms);
  Blynk.virtualWrite(V1, emon.Irms);
  Blynk.virtualWrite(V2, emon.apparentPower);
  Blynk.virtualWrite(V3, kWh);
 
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Vrms: ");
  lcd.print(emon.Vrms, 2);
  lcd.print(" V");
 
  lcd.setCursor(0, 1);
  lcd.print("Irms: ");
  lcd.print(emon.Irms, 4);
  lcd.print(" A");
 
  lcd.setCursor(0, 2);
  lcd.print("Power: ");
  lcd.print(emon.apparentPower, 4);
  lcd.print(" W");
 
  lcd.setCursor(0, 3);
  lcd.print("kWh: ");
  lcd.print(kWh, 5);
  lcd.print(" kWh");
}
 
 
void readEnergyDataFromEEPROM()
{
  EEPROM.get(addrKWh, kWh);
 
  if (isnan(kWh))
  {
    kWh = 0.0;
    saveEnergyDataToEEPROM(); 
  }
}
 
 
void saveEnergyDataToEEPROM()
{
  EEPROM.put(addrKWh, kWh);
  EEPROM.commit();
}