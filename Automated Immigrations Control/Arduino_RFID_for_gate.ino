// ARDUINO CODE FOR RFID
// RFID code with wifi

// YC CODE TESTED

#include <SPI.h>
#include <MFRC522.h>
#include <WiFiNINA.h>
#include "DFRobot_RGBLCD1602.h"

// RFID pins
#define SS_PIN 10
#define RST_PIN 9
MFRC522 rfid(SS_PIN, RST_PIN);

// WiFi credentials
char ssid[] = "wifi_name";
char pass[] = "wifi_password";
int status = WL_IDLE_STATUS;

unsigned long connectionTimeout = 20000;
WiFiSSLClient client;

int HTTP_PORT = 443;
String HTTP_METHOD = "GET";
const char HOST_NAME[] = "script.google.com";
String PATH_NAME = "/macros/s/";
String DEPLOYMENT_ID = "Add_deployment_ID_here";

void setup() {
  Serial.begin(9600);
  while (!Serial);
  
  SPI.begin();
  rfid.PCD_Init();
  
  enable_WiFi();
  connect_WiFi();
  printWifiStatus();
}

void loop() {
  if (readCard()) {
    String cardUID = getCardUID();
    Serial.print("Card UID: ");
    Serial.println(cardUID);

    String url = constructURL(cardUID);
    Serial.print("Request URL: ");
    Serial.println(url);

    if (sendHTTPRequest(url)) {
      Serial.println("Request sent successfully");
    } else {
      Serial.println("Failed to connect to server");
    }
  }
  delay(2000);
}

bool readCard() {
  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) {
    delay(1000);
    return false;
  }
  return true;
}

String getCardUID() {
  String cardUID = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
    cardUID += String(rfid.uid.uidByte[i] < 0x10 ? "0" : "");
    cardUID += String(rfid.uid.uidByte[i], HEX);
  }
  cardUID.toUpperCase();
  return cardUID;
}

// Your Arduino code
String constructURL(String cardUID) {
  String url = PATH_NAME + DEPLOYMENT_ID + "/exec?";
  url += "cardUID=" + cardUID;
  return url;
}


bool sendHTTPRequest(String url) {
  unsigned long startConnection = millis();
  while (!client.connect(HOST_NAME, HTTP_PORT)) {
    if (millis() - startConnection > connectionTimeout) {
      return false;
    }
    delay(100);
  }

  client.println(HTTP_METHOD + " " + url + " HTTP/1.1");
  client.println("Host: " + String(HOST_NAME));
  client.println("Connection: close");
  client.println();
  client.stop();
  
  Serial.println("Connection closed");
  return true;
}

void printWifiStatus() {
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  long rssi = WiFi.RSSI();
  Serial.print("Signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}

void enable_WiFi() {
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    while (true);
  }

  String fv = WiFi.firmwareVersion();
  if (fv < "1.0.0") {
    Serial.println("Please upgrade the firmware");
  }
}

void connect_WiFi() {
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    status = WiFi.begin(ssid, pass);
    delay(10000);
  }
}



