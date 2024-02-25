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

// Constants for calibration
const float vCalibration = 41.5;
const float currCalibration = 0.15;

// Blynk and WiFi credentials
const char auth[] = "6SyNhr96X9PNXQbYjeuuW1HfPLPWXNWp";
const char ssid[] = "010302";
const char pass[] = "1234567890";

// EnergyMonitor instance
EnergyMonitor emon;

// Timer for regular updates
BlynkTimer timer;

// Variables for energy calculation
float kWh = 0.0;
unsigned long lastMillis = millis();

// EEPROM addresses for each variable
const int addrKWh = 0;

void setup() {
  Serial.begin(115200);
  Blynk.begin(auth, ssid, pass);

  // Initialize the LCD
  lcd.init();
  lcd.backlight();

  // Read the stored energy data from EEPROM
  EEPROM.begin(sizeof(kWh));
  EEPROM.get(addrKWh, kWh);

  // Setup voltage and current inputs
  emon.voltage(35, vCalibration, 1.7); // Voltage: input pin, calibration, phase_shift
  emon.current(34, currCalibration);    // Current: input pin, calibration

  // Setup a timer for sending data every 5 seconds
  timer.setInterval(5000L, sendEnergyDataToBlynk);

  // A small delay for system to stabilize
  delay(1000);
}

void loop() {
  Blynk.run();
  timer.run();
}

void sendEnergyDataToBlynk() {
  emon.calcVI(20, 2000); // Calculate all. No.of half wavelengths (crossings), time-out

  // Calculate energy consumed in kWh
  unsigned long currentMillis = millis();
  float elapsedHours = (currentMillis - lastMillis) / 3600000.0; // Convert milliseconds to hours
  kWh += emon.apparentPower * elapsedHours;
  lastMillis = currentMillis;

  // Print data to Serial for debugging
  Serial.printf("Vrms: %.2fV\tIrms: %.4fA\tPower: %.4fW\tkWh: %.5fkWh\n",
                emon.Vrms, emon.Irms, emon.apparentPower, kWh);

  // Save the latest kWh value to EEPROM
  EEPROM.put(addrKWh, kWh);
  EEPROM.commit();

  // Send data to Blynk
  Blynk.virtualWrite(V0, emon.Vrms);
  Blynk.virtualWrite(V1, emon.Irms);
  Blynk.virtualWrite(V2, emon.apparentPower);
  Blynk.virtualWrite(V3, kWh);

  // Update the LCD with the new values
  updateLCD();
}

void updateLCD() {
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
