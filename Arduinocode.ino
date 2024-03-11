#define USE_ARDUINO_INTERRUPTS true
#define MQ2pin (1)
//#include <PulseSensorPlayground.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include "MAX30100_PulseOximeter.h"

#define NUM_INPUTS 4  // Number of inputs from the user
#define REPORTING_PERIOD_MS 1000

LiquidCrystal_I2C lcd(0x27, 16, 2);

PulseOximeter pox;
uint32_t tsLastReport = 0;
const int LED_PIN = 13;  // On-board LED PIN

float sensorValue, tempC, tempF, BPM, SpO2;
int readingsCount = 0;
float sumBPM = 0;
float sumSpO2 = 0;
float sumTempC = 0;
float sumTempF = 0;
float sumSensorValue = 0;

#define ONE_WIRE_BUS 2
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

void onBeatDetected() {
    // Serial.println("Beat!");
}

void setup(void) {
    Serial.begin(115200);
    sensors.begin();  // Start up the library
    lcd.init();
    lcd.backlight();
    lcd.setCursor(0, 0);
    lcd.print("Health Monitor");
    lcd.setCursor(0, 1);
    lcd.print("System");

    Serial.print("Initializing pulse oximeter....");
    if (!pox.begin()) {
        Serial.println("FAILED");
        for (;;);
    } else {
        Serial.println("SUCCESS");
        pox.setOnBeatDetectedCallback(onBeatDetected);
    }

    pox.setIRLedCurrent(MAX30100_LED_CURR_7_6MA);
    pox.setOnBeatDetectedCallback(onBeatDetected);
}

void loop(void) {
    pox.update();
    BPM = pox.getHeartRate();
    SpO2 = pox.getSpO2();
    sensorValue = analogRead(A0);
    tempC = sensors.getTempCByIndex(0);
    tempF = (sensors.getTempCByIndex(0) * 9.0) / 5.0 + 32.0;

    sensors.setWaitForConversion(false);
    sensors.requestTemperatures();
    sensors.setWaitForConversion(true);

    if (millis() - tsLastReport > REPORTING_PERIOD_MS) {
        if (BPM > 0 && SpO2 > 0) {
            readingsCount++;
            sumBPM += BPM;
            sumSpO2 += SpO2;
            sumTempC += tempC;
            sumTempF += tempF;
            sumSensorValue += sensorValue;
            //Calibration by averaging 

            if (readingsCount == NUM_INPUTS) {
                float avgBPM = sumBPM / (NUM_INPUTS );
                float avgSpO2 = sumSpO2 / NUM_INPUTS;
                float avgTempC = sumTempC / NUM_INPUTS;
                float avgTempF = sumTempF / NUM_INPUTS;
                float avgSensorValue = sumSensorValue / NUM_INPUTS;
                
               Serial.print(avgBPM);
                Serial.print(",");
                Serial.print(avgSpO2);
                Serial.print(",");
                Serial.print(avgTempC);
                Serial.print(",");
                Serial.print(avgTempF);
                Serial.print(",");
                Serial.print(avgSensorValue);
                Serial.print("\n");

                readingsCount = 0;
                sumBPM = 0;
                sumSpO2 = 0;
                sumTempC = 0;
                sumTempF = 0;
                sumSensorValue = 0;
            }
            tsLastReport = millis();
        }
    }
}
