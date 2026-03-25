
#include <SPI.h>
#include <WiFiNINA.h>
#include <Stepper.h>

const int stepsPerRevolution = 2038;
const int stepsPerQuarterTurn = stepsPerRevolution / 4; // Approximately 510 steps for a 90-degree turn

Stepper myStepper(stepsPerRevolution, 8, 10, 9, 11);
char ssid[] = "wifi_name";           // your network SSID (name)
char pass[] = "wifi_password";       // your network password
int port = 80;

WiFiServer server(port);

void setup() {
  Serial.begin(9600);
  //stepper.setSpeed(30);

  // Attempt to connect to WiFi network
  if (WiFi.begin(ssid, pass) != WL_CONNECTED) {
    Serial.println("Connecting to WiFi...");
    while (WiFi.status() != WL_CONNECTED) {
      delay(1000);
      Serial.print(".");
    }
  }

  // Start the server
  server.begin();
  Serial.print("Connected to WiFi. IP Address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  WiFiClient client = server.available();   // Check for incoming clients

  if (client) {
    Serial.println("New client connected");

    // Read the client request
    String request = client.readStringUntil('\n');
    Serial.println("Request: " + request);

    if (request.indexOf("PASS") != -1) {
      // Rotate the stepper motor 90 degrees clockwise
      myStepper.setSpeed(10);
      myStepper.step(stepsPerQuarterTurn);
      delay(3000);  // Wait for 3 seconds
      myStepper.step(-stepsPerQuarterTurn);
    }

    client.stop();  // Disconnect from the client
    Serial.println("Client disconnected");
  }
}



