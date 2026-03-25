#include "DFRobot_RGBLCD1602.h"
#include <SPI.h>
#include <MFRC522.h>
#include <WiFiNINA.h>
#include "HX711.h"
#include <ArduinoJson.h>

// WiFi credentials 
char ssid[] = "Hello"; 
char pass[] = "Joeltan3"; 
int keyIndex = 0; 
int status = WL_IDLE_STATUS; 
 
unsigned long connectionTimeout = 20000;  
 
WiFiSSLClient client; // Creates a client that can connect to a specified internet IP address. 
 
int HTTP_PORT = 443;                      // HTTP port that web server uses. HTTPS is secure and is on port 443, while HTTP is unsecured and available on port 80. 
String HTTP_METHOD = "GET";               // Request method (POST or GET) 
char HOST_NAME[] = "script.google.com";   // Host name of web server 
const char redirectHost[] = "script.googleusercontent.com";
String PATH_NAME = "/macros/s/"; 
// Google Script URL 

#define RST_PIN 0 //RFID
#define SS_PIN 1 //RFID
#define DOUT_PIN  2 // Connect to DT (Data) pin of HX711 loadcell
#define SCK_PIN   3 // Connect to SCK (Clock) pin of HX711 loadcell
#define trigPin_w1 4 
#define echoPin_w1 5 
#define trigPin_w2 6
#define echoPin_w2 7 
#define trigPin_l1 8
#define echoPin_l1 9 
#define trigPin_l2 10
#define echoPin_l2 11
#define trigPin_h 12
#define echoPin_h 13
#define BUTTON_PIN A0 //button
MFRC522 rfid(SS_PIN, RST_PIN);

float duration_left; // variable for the duration of sound wave travel
float distance_left; // variable for the distance measurement
float duration_right;
float distance_right;
float duration_front;
float distance_front;
float duration_back;
float distance_back;
float duration_top;
float distance_top;

float box_width = 18.1; //box dimension
float box_length = 21.5;
float box_height = 8.7;

float object_width; //object dimension
float object_length;
float object_height;

// LCD
DFRobot_RGBLCD1602 lcd(/*RGBAddr*/0x60, /*lcdCols*/16, /*lcdRows*/2);  // 16 characters and 2 lines of show
// Initialize HX711 object
HX711 scale;
float calibration_factor = -103150; //calibration for load cell

void setup() {
  Serial.begin(9600);
  pinMode(BUTTON_PIN, INPUT);
  pinMode(trigPin_w1, OUTPUT); // Sets the trigPin as an OUTPUT
  pinMode(trigPin_w2, OUTPUT);
  pinMode(trigPin_l1, OUTPUT);
  pinMode(trigPin_l2, OUTPUT);
  pinMode(trigPin_h, OUTPUT);
  
  pinMode(echoPin_w1, INPUT); // Sets the echoPin as an INPUT
  pinMode(echoPin_w2, INPUT); // Sets the echoPin as an INPUT
  pinMode(echoPin_l1, INPUT); // Sets the echoPin as an INPUT
  pinMode(echoPin_l2, INPUT); // Sets the echoPin as an INPUT
  pinMode(echoPin_h, INPUT); // Sets the echoPin as an INPUT
  lcd.init();
  SPI.begin();  // Start SPI communication here to be used later in RFID()
  rfid.PCD_Init();  // Initialize the RFID reader
  scale.begin(DOUT_PIN, SCK_PIN); //initialize loadcell
  scale.tare();
  while (!Serial);  
  // Connect to WiFi 
  enable_WiFi(); 
  connect_WiFi(); 
  Serial.println("Connected to WiFi"); 
  printWifiStatus(); 
}



void loop() {
  byte buttonState = digitalRead(BUTTON_PIN);
  String PID = "";
  String BID = "";
  float weight = 0;
  PID = RFID();
  if (PID != "") {  // Check if a UID was returned
    display_passport(PID); // display passport id
    String response = check_url(PID);
    if (response == "checked in"){
      String flight = getflight(PID);
      display_flight(flight);
      delay(3000);
      while (BID == "") {
        display("Scan Bag");
        BID = RFID();  // Attempt to read the BID
        delay(500);    // Add a short delay to avoid rapid re-scanning
      }
      display_pass_bag(PID, BID); //display passport and bag id
      String name = getname(PID);
      display_button();
      while(digitalRead(BUTTON_PIN) == LOW){
      }
      weight = weighing();
      object_length = measure_length();
      object_width = measure_width();
      object_height = measure_height();
      Serial.print("\n");
      Serial.print(distance_left);
      Serial.print("\n");
      Serial.print(distance_right);
      Serial.print("\n");
      Serial.print(distance_front);
      Serial.print("\n");
      Serial.print(distance_back);
      Serial.print("\n");
      Serial.print(distance_top);

      display_weight(weight);
      delay(3000);
      display_dimension(object_length, object_width, object_height);
      delay(3000);
      construct_url(PID, name, flight, BID, weight, object_length, object_width, object_height);
    }
    else if (response == "not checked in"){
      display(response);
      delay(5000);
    }
  }
  

  else
  display("Scan Passport");
  delay(500);
}












void display(String message) {
  lcd.clear();
  lcd.setCursor(0, 0);  // Set position on LCD
  lcd.print(message);       // Print UID on LCD
}
void display_button(){
  lcd.clear();
  lcd.setCursor(0, 0);  // Set position on LCD
  lcd.print("Push button when bag is placed");
  lcd.setCursor(0, 1);  // Set position on LCD
  lcd.print("bag is placed");

}
void display_passport(String PID) {
  lcd.clear();
  lcd.setCursor(0, 0);  // Set position on LCD
  lcd.print("PID:");       
  lcd.setCursor(4, 0);  // Set position on LCD
  lcd.print(PID);
}
void display_flight(String flight){
  lcd.clear();
  lcd.setCursor(0, 0);  // Set position on LCD
  lcd.print("Flight:");    
  lcd.setCursor(0,1);
  lcd.print(flight); 
}
void display_pass_bag(String PID, String BID) {
  lcd.clear();
  lcd.setCursor(0, 0);  // Set position on LCD
  lcd.print("PID:");       
  lcd.setCursor(4, 0);  // Set position on LCD
  lcd.print(PID);

  lcd.setCursor(0,1);
  lcd.print("BID:");
  lcd.print(BID);
}
void display_weight(float weight){
  lcd.clear();
  lcd.setCursor(0, 0);  // Set position on LCD
  lcd.print("weight:");
  lcd.setCursor(7, 0);
  lcd.print(weight);
  lcd.setCursor(11,0);
  lcd.print("kg");
}
void display_dimension(float length, float width , float height){
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("L:");
  lcd.setCursor(2,0);
  lcd.print(length); 
  lcd.setCursor(6,0);
  lcd.print("cm");      

  lcd.setCursor(0,1);
  lcd.print("W:");
  lcd.setCursor(2,1);
  lcd.print(width);
  lcd.setCursor(6,1);
  lcd.print("cm");

  lcd.setCursor(8,1);
  lcd.print("H:");
  lcd.setCursor(10,1);
  lcd.print(height);
  lcd.setCursor(14,1);
  lcd.print("cm");
}

String RFID() {
  // Check if a new card is present and read it
  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) {
    return "";  // No card detected; exit function
  }

  // Read the UID
  String cardUID = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
    cardUID += String(rfid.uid.uidByte[i] < 0x10 ? "0" : ""); // Add leading 0 for single hex digits
    cardUID += String(rfid.uid.uidByte[i], HEX);
  }
  cardUID.toUpperCase();

  // Display the UID in Serial Monitor
  Serial.print("Card UID: ");
  Serial.println(cardUID);

  rfid.PICC_HaltA();       // Stop reading the current card
  rfid.PCD_StopCrypto1();  // Stop encryption on the RFID reader

  return cardUID;  // Return the UID as a string
}

float weighing(){
  float weight = scale.get_units(10) / calibration_factor; 
  return weight;
}

void construct_url(String PID, String name, String flight, String BID, float weight, float length, float width, float height){
   // Construct the URL 
  String url = PATH_NAME; 
  url += "AKfycbzHJsyp5FtGsWyWGdDf-_AgkrSz3xMIW6o0qLabpTMrJXmvaW_Q65bhavQxj4j986O0DQ"; //Your Deployment ID 
  url += "/exec?";   
  url += "action=addBaggage";
  url += "&PID="; 
  url += PID;
  url += "&name="; 
  url += name;
  url += "&flight="; 
  url += flight;
  url += "&BID="; 
  url += BID;
  url += "&weight="; 
  url += String(weight);
  url += "&length=";
  url += String(length);
  url += "&width=";
  url += String(width);
  url += "&height=";
  url += String(height);

 
  Serial.print("Request URL: "); 
  Serial.println(url); 
 
  // Attempt to connect 
  unsigned long startConnection = millis(); 
  while (!client.connect(HOST_NAME, HTTP_PORT)) { 
    if (millis() - startConnection > connectionTimeout) { 
      Serial.println("Connection timed out"); 
      return;  // Exit if connection fails 
    } 
    delay(100);  // Small delay between retries 
  } 
 
  // Send HTTPS request 
  Serial.println("Connected to server"); 
  client.println(HTTP_METHOD + " " + String (url) + String(" HTTP/1.1")); 
  Serial.println(HTTP_METHOD + " " + String (url) + String(" HTTP/1.1")); 
  client.println("Host: script.google.com"); 
  client.println("Connection: close"); 
  client.println();  // End of headers 
  client.stop();     // Close connection 
  Serial.println("Connection closed"); 
 
  delay(2000); 
}

String check_url(String PID){
  String url = PATH_NAME; 
  url += "AKfycbzHJsyp5FtGsWyWGdDf-_AgkrSz3xMIW6o0qLabpTMrJXmvaW_Q65bhavQxj4j986O0DQ"; //Your Deployment ID 
  url += "/exec?";   
  url += "action=checkInStatus";
  url += "&PID="; 
  url += PID;
  Serial.println(url);
  // Fetch the response using fetchUrl
  String response = fetchUrl(HOST_NAME, url);
  
  if (response.indexOf("Moved Temporarily") > -1) {
    String redirectUrl = getRedirectUrl(response);
    int pathStart = redirectUrl.indexOf("/macros");
    String redirectPath = redirectUrl.substring(pathStart);
    response = fetchUrl(redirectHost, redirectPath);    
  }

  Serial.println("\nRaw Response:");
  Serial.println(response);  // Print the raw response
  return response;
}
String getname (String PID){
  String url = PATH_NAME; 
  url += "AKfycbzHJsyp5FtGsWyWGdDf-_AgkrSz3xMIW6o0qLabpTMrJXmvaW_Q65bhavQxj4j986O0DQ"; //Your Deployment ID 
  url += "/exec?";   
  url += "action=name";
  url += "&PID="; 
  url += PID;

  // Fetch the response using fetchUrl
  Serial.println(url);
  String response = fetchUrl(HOST_NAME, url);
  
  if (response.indexOf("Moved Temporarily") > -1) {
    String redirectUrl = getRedirectUrl(response);
    int pathStart = redirectUrl.indexOf("/macros");
    String redirectPath = redirectUrl.substring(pathStart);
    response = fetchUrl(redirectHost, redirectPath);    
  }

  Serial.println("\nRaw Response:");
  Serial.println(response);  // Print the raw response
  return response;
}

String getflight (String PID){
  String url = PATH_NAME; 
  url += "AKfycbzHJsyp5FtGsWyWGdDf-_AgkrSz3xMIW6o0qLabpTMrJXmvaW_Q65bhavQxj4j986O0DQ"; //Your Deployment ID 
  url += "/exec?";   
  url += "action=flight";
  url += "&PID="; 
  url += PID;

  // Fetch the response using fetchUrl
  Serial.println(url);
  String response = fetchUrl(HOST_NAME, url);
  
  if (response.indexOf("Moved Temporarily") > -1) {
    String redirectUrl = getRedirectUrl(response);
    int pathStart = redirectUrl.indexOf("/macros");
    String redirectPath = redirectUrl.substring(pathStart);
    response = fetchUrl(redirectHost, redirectPath);    
  }

  Serial.println("\nRaw Response:");
  Serial.println(response);  // Print the raw response
  return response;
}

void printWifiStatus() { 
  // Print the SSID of the network you're attached to: 
  Serial.print("SSID: "); 
  Serial.println(WiFi.SSID()); 
 
  // Print your board's IP address: 
  IPAddress ip = WiFi.localIP(); 
  Serial.print("IP Address: "); 
  Serial.println(ip); 
 
  // Print the received signal strength: 
  long rssi = WiFi.RSSI(); 
  Serial.print("Signal strength (RSSI):"); 
  Serial.print(rssi); 
  Serial.println(" dBm"); 
} 

void enable_WiFi() { 
  // Check for the WiFi module: 
  if (WiFi.status() == WL_NO_MODULE) { 
    Serial.println("Communication with WiFi module failed!"); 
    // Don't continue 
    while (true); 
  } 
 
  String fv = WiFi.firmwareVersion(); 
  if (fv < "1.0.0") { 
    Serial.println("Please upgrade the firmware"); 
  } 
} 

void connect_WiFi() { 
  // Attempt to connect to WiFi network 
  while (status != WL_CONNECTED) { 
    Serial.print("Attempting to connect to SSID: "); 
    Serial.println(ssid); 
    status = WiFi.begin(ssid, pass); 
    delay(10000); // Retry every 10 seconds 
  } 
}

float measure_width(){

  digitalWrite(trigPin_w1, LOW);        // Clears the trigPin condition
  delayMicroseconds(2);
  digitalWrite(trigPin_w1, HIGH);       // Sets the trigPin HIGH (ACTIVE)
  delayMicroseconds(10);
  digitalWrite(trigPin_w1, LOW);

  duration_left = pulseIn(echoPin_w1, HIGH);
  distance_left = duration_left * 0.034 / 2;   // Speed of sound wave divided by 2 (go and back)
  
  digitalWrite(trigPin_w2, LOW);        // Clears the trigPin condition
  delayMicroseconds(2);
  digitalWrite(trigPin_w2, HIGH);       // Sets the trigPin HIGH (ACTIVE)
  delayMicroseconds(10);
  digitalWrite(trigPin_w2, LOW);
  duration_right = pulseIn(echoPin_w2, HIGH);
  distance_right = duration_right * 0.034 / 2;   // Speed of sound wave divided by 2 (go and back)

  object_width = box_width - distance_right - distance_left;

  return object_width;
}

float measure_length(){
  digitalWrite(trigPin_l1, LOW);        // Clears the trigPin condition
  delayMicroseconds(2);
  digitalWrite(trigPin_l1, HIGH);       // Sets the trigPin HIGH (ACTIVE)
  delayMicroseconds(10);
  digitalWrite(trigPin_l1, LOW);
  duration_front = pulseIn(echoPin_l1, HIGH);
  distance_front = duration_front * 0.034 / 2;   // Speed of sound wave divided by 2 (go and back)

  digitalWrite(trigPin_l2, LOW);        // Clears the trigPin condition
  delayMicroseconds(2);
  digitalWrite(trigPin_l2, HIGH);       // Sets the trigPin HIGH (ACTIVE)
  delayMicroseconds(10);
  digitalWrite(trigPin_l2, LOW);
  duration_back = pulseIn(echoPin_l2, HIGH);
  distance_back = duration_back * 0.034 / 2;   // Speed of sound wave divided by 2 (go and back)

  object_length = box_length - distance_front - distance_back;
  return object_length;
}

float measure_height(){
  digitalWrite(trigPin_h, LOW);        // Clears the trigPin condition
  delayMicroseconds(2);
  digitalWrite(trigPin_h, HIGH);       // Sets the trigPin HIGH (ACTIVE)
  delayMicroseconds(10);
  digitalWrite(trigPin_h, LOW);
  duration_top = pulseIn(echoPin_h, HIGH);
  distance_top = duration_top * 0.034 / 2;   // Speed of sound wave divided by 2 (go and back)
    
  object_height = box_height - distance_top;
  return object_height;
}

String readChunkedResponse() {
  String response = "";
  while (true) {
    String chunkSizeStr = "";
    while (client.available()) {
      char c = client.read();
      if (c == '\r') {
        client.read();
        break;
      }
      chunkSizeStr += c;
    }
    
    int chunkSize = hexToInt(chunkSizeStr);
    if (chunkSize == 0) break;
    
    for (int i = 0; i < chunkSize && client.available(); i++) {
      response += (char)client.read();
    }
    
    if (client.available()) client.read();
    if (client.available()) client.read();
  }
  return response;
}

String fetchUrl(const char* serverHost, String path) {
  if (!client.connect(serverHost, HTTP_PORT)) {
    Serial.println("Connection failed");
    return "";
  }
  
  Serial.println("Connected to server");
  
  client.print("GET ");
  client.print(path);
  client.println(" HTTP/1.1");
  client.print("Host: ");
  client.println(serverHost);
  client.println("User-Agent: ArduinoWiFi/1.1");
  client.println("Connection: close");
  client.println();

  while (!client.available()) {
    delay(100);
  }

  bool isChunked = false;
  while (client.available()) {
    String line = client.readStringUntil('\n');
    if (line.indexOf("Transfer-Encoding: chunked") >= 0) {
      isChunked = true;
    }
    if (line == "\r") break;
  }

  String response = isChunked ? readChunkedResponse() : "";
  client.stop();
  return response;
}

int hexToInt(String hex) {
  return strtol(hex.c_str(), NULL, 16);
}

String getRedirectUrl(String response) {
  int startIndex = response.indexOf("HREF=\"") + 6;
  int endIndex = response.indexOf("\"", startIndex);
  String url = response.substring(startIndex, endIndex);
  url.replace("&amp;", "&");
  return url;
}