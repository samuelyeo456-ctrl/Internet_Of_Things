#include <SPI.h>
#include <MFRC522.h>
#include <WiFiNINA.h>

// RFID pins
#define SS_PIN 10
#define RST_PIN 9
MFRC522 rfid(SS_PIN, RST_PIN);

// WiFi credentials
char ssid[] = "YC";
char pass[] = "yc123456";

// Telegram Bot credentials
char* botToken = "7673727460:AAEERweobawyLBTf8btWIDytJu3fv9VtoYg";
char* chatID = "43498732";
int HTTP_PORT = 443;
char HOST_NAME[] = "api.telegram.org";

// WiFi client for HTTPS
WiFiSSLClient client;

void setup() {
    Serial.begin(9600);
    while (!Serial);

    // Initialize RFID reader
    SPI.begin();
    rfid.PCD_Init();

    // Connect to WiFi
    Serial.println("Connecting to WiFi...");
    while (WiFi.begin(ssid, pass) != WL_CONNECTED) {
        delay(1000);
        Serial.print(".");
    }
    Serial.println("\nWiFi connected!");
    printWifiStatus();
}

void loop() {
    // Look for new RFID cards
    Serial.println("Scan an RFID card...");
    if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) {
        delay(1000); // No card detected; wait a bit
        return;
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

    // Send UID to Telegram
    sendTelegramMessage("BID: " + cardUID +" arrived at SQ349");

    delay(2000); // Wait before scanning again
}

void sendTelegramMessage(String message) {
    if (client.connect(HOST_NAME, HTTP_PORT)) {
        Serial.println("Connected to Telegram server!");

        // Send the message
        String url = "GET /bot" + String(botToken) + "/sendMessage?chat_id=" + chatID + "&text=" + message + " HTTP/1.1";
        client.println(url);
        client.println("Host: api.telegram.org");
        client.println("Connection: close");
        client.println();

        // Wait for the response
        while (client.connected()) {
            if (client.available()) {
                String line = client.readStringUntil('\n');
                Serial.println(line);
            }
        }
        client.stop();
        Serial.println("Message sent!");
    } else {
        Serial.println("Failed to connect to Telegram.");
    }
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
