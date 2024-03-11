#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <SoftwareSerial.h>
#include <LiquidCrystal_I2C.h>
#include <ESP8266HTTPClient.h>

String URL = "http://api.thingspeak.com/update?api_key=XICPLN4OF8ADTWUV&field1=";

LiquidCrystal_I2C lcd(0x27, 16, 2);

#define SIM800_RX_PIN D3
#define SIM800_TX_PIN D4

SoftwareSerial sim800lSerial(SIM800_TX_PIN, SIM800_RX_PIN);

#define TH_C 38
#define TH_F 99.5
#define TH_BPM 90
#define TH_SpO2 50
#define TH_A 200

void setup() {
  Serial.begin(115200);
  sim800lSerial.begin(9600);

  lcd.init();
  lcd.backlight();
  lcd.print("Connected");
  
  WiFi.begin("AdvancedCollege", "acem@123");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("Connected");
  lcd.clear();
}

void loop() {
  int ecg;
  ecg = analogRead(A0);
  Serial.println(ecg);
  sendDataecg(ecg);
  if (Serial.available() > 0) {
    String data = Serial.readStringUntil('\n');
    Serial.println(data);

    if (data.length() > 0) {
      processAndDisplayData(data);
    }
  }
}

void processAndDisplayData(String data) {
  float b, s, c, f, a;
  

  int comma1 = data.indexOf(',');
  int comma2 = data.indexOf(',', comma1 + 1);
  int comma3 = data.indexOf(',', comma2 + 1);
  int comma4 = data.indexOf(',', comma3 + 1);

  if (comma1 != -1 && comma2 != -1 && comma3 != -1 && comma4 != -1) {
    b = data.substring(0, comma1).toFloat();
    s = data.substring(comma1 + 1, comma2).toFloat();
    c = data.substring(comma2 + 1, comma3).toFloat();
    f = data.substring(comma3 + 1, comma4).toFloat();
    a = data.substring(comma4 + 1).toFloat();
    int ecg ;
    //ecg = analogRead(A0);
    
    displaySensorData(b, s, c, f, a , ecg);

    if (b > TH_BPM) {
      sendSMS("+9779840892632", "High HeartBeat! BPM: " + String(b));
      delay(2000);
    }
    if (s < TH_SpO2) {
      sendSMS("+9779840892632", "Low Oxygen Level! SpO2: " + String(s));
      delay(2000);
    }
    if (c > TH_C) {
      sendSMS("+9779840892632", "High Body Temperature! C: " + String(c));
      delay(2000);
    }
    if (a > TH_A) {
      sendSMS("+9779840892632", "Polluted Environment! PPM: " + String(a));
      delay(2000);
    }
    
    sendData(b, s, c, f, a, ecg );
  }
}

void displaySensorData(float b, float s, float c, float f, float a , float ecg) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("BPM: " + String(b));

  lcd.setCursor(0, 1);
  lcd.print("SpO2: " + String(s) + " %");

  delay(1000);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Body Temp:" + String(f) + " C");

  lcd.setCursor(0, 1);
  lcd.print("Air Qua.: " + String(a) + " PPM");
  
  delay(2000);
  lcd.clear();

}
//ecg
void sendDataecg(int ecg) {
  WiFiClient client;
  HTTPClient http;

  String newUrl = URL +  "&field6=" + String(ecg);

  if (http.begin(client, newUrl)) {
    int responseCode = http.GET();
    String payload = http.getString();
    Serial.println(payload);
    http.end();
  } else {
    Serial.println("Failed to connect to ThingSpeak");
  }
}
//
void sendData(float b, float s, float c, float f, float a, int ecg) {
  WiFiClient client;
  HTTPClient http;

  String newUrl = URL + String(b) + "&field2=" + String(s) + "&field3=" + String(c) + "&field4=" + String(f) + "&field5=" + String(a) + "&field6=" + String(ecg) ;

  if (http.begin(client, newUrl)) {
    int responseCode = http.GET();
    String payload = http.getString();
    Serial.println(payload);
    http.end();
  } else {
    Serial.println("Failed to connect to ThingSpeak");
  }
}

void sendSMS(String number, String message) {
  sim800lSerial.println("AT+CMGF=1");
  delay(1000);

  sim800lSerial.print("AT+CMGS=\"");
  sim800lSerial.print(number);
  sim800lSerial.println("\"");
  delay(1000);

  sim800lSerial.print(message);
  sim800lSerial.write(0x1A); // End of message character
  delay(1000);

  while (sim800lSerial.available()) {
    Serial.write(sim800lSerial.read());
  }
}