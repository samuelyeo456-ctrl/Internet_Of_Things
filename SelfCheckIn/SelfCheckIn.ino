#include <SPI.h> 
#include <MFRC522.h> 
#include <WiFiNINA.h> 
#include "DFRobot_RGBLCD1602.h"

 
// RFID pins 
#define SS_PIN 10 
#define RST_PIN 9 
MFRC522 rfid(SS_PIN, RST_PIN); 
 
// WiFi credentials 
char ssid[] = "YC"; 
char pass[] = "yc123456"; 
int keyIndex = 0; 
int status = WL_IDLE_STATUS; 
const char* redirectHost = "script.googleusercontent.com";


const int colorR = 255;
const int colorG = 0;
const int colorB = 0;
 
unsigned long connectionTimeout = 20000;  
 
WiFiSSLClient client; // Creates a client that can connect to a specified internet IP address. 
DFRobot_RGBLCD1602 lcd(/*RGBAddr*/0x60 ,/*lcdCols*/16,/*lcdRows*/2);  //16 characters and 2 lines of show

int HTTP_PORT = 443;                      // HTTP port that web server uses. HTTPS is secure and is on port 443, while HTTP is unsecured and available on port 80. 
String HTTP_METHOD = "GET";               // Request method (POST or GET) 
char HOST_NAME[] = "script.google.com";   // Host name of web server 
String PATH_NAME = "/macros/s/"; 
// Google Script URL 
 
void setup() { 
  Serial.begin(9600); 
  while (!Serial);  
  // Connect to WiFi 
  enable_WiFi(); 
  connect_WiFi(); 
  Serial.println("Connected to WiFi"); 
  printWifiStatus(); 
  lcd.init();
} 
 
void loop() { 
  // Look for new RFID cards 
  String PID = "";
  SPI.begin(); 
  rfid.PCD_Init(); 
  Serial.println("Scan your passport to CHECK IN..."); 
  display("Scan Passport");
  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) { 
    delay(1000); // No card detected; wait a bit 
    return; 
  } 
 
  // Read the UID 
  for (byte i = 0; i < rfid.uid.size; i++) { 
    PID += String(rfid.uid.uidByte[i] < 0x10 ? "0" : ""); // Add leading 0 for single hex digits 
    PID += String(rfid.uid.uidByte[i], HEX); 
  } 
  PID.toUpperCase(); 
 
  // Display the UID in Serial Monitor 
  Serial.print("Card UID: "); 
  Serial.println(PID); 
  display_passport(PID);
  
  String Name= getname (PID);
  display_name(PID, Name);
  
  String Flight= getflight(PID);
  display_flightnumber(PID, Flight);

  // Construct the URL 
  String url = PATH_NAME; 
  url += "AKfycbyOVdDMWMUqByh_YGCmLszhEqq68JiCxNTL-H_vJcw3JvOohYVsaCdtZtuPx2_W7gZ-SA"; //Your Deployment ID 
  url += "/exec?";   
  url += "PID="; 
  url += PID;
  url += "&Name=";
  url += Name;
  url += "&Flight=";
  url += Flight; 

 
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
  display("Printing...");
  delay(5000);
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

void display(String message) {
  lcd.clear();
  lcd.setCursor(0, 0);  // Set position on LCD
  lcd.print(message);       // Print UID on LCD
}

void display_passport(String PID) {
  lcd.clear();
  lcd.setCursor(0, 0);  // Set position on LCD
  lcd.print("PID:");       
  lcd.setCursor(4, 0);  // Set position on LCD
  lcd.print(PID);
  delay(3000);
}

void display_name(String PID, String Name) {
  lcd.clear();
  lcd.setCursor(0, 0);  // Set position on LCD
  lcd.print("Name:");     
  lcd.setCursor(5, 0);  // Set position on LCD
  lcd.print(Name);
  delay(3000);
}

void display_flightnumber(String PID, String Flight) {
  lcd.clear();
  lcd.setCursor(0, 0);  // Set position on LCD
  lcd.print("FlightNB:");       
  lcd.setCursor(9, 0);  // Set position on LCD
  lcd.print(Flight);
  delay(3000);
}

String getflight (String PID){
  String url = PATH_NAME; 
  url += "AKfycbz2BL4XHYEio-ErssSh5oAS1lI1_Dj6chFcJqpbXgalQK4QYncRPuTZJadRTau3iCgnUg"; //Your Deployment ID 
  url += "/exec?";   
  url += "action=Flight";
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

String getname (String PID){
  String url = PATH_NAME; 
  url += "AKfycbz2BL4XHYEio-ErssSh5oAS1lI1_Dj6chFcJqpbXgalQK4QYncRPuTZJadRTau3iCgnUg"; //Your Deployment ID 
  url += "/exec?";   
  url += "action=Name";
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
