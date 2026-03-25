# Internet_Of_Things
This project is an IoT Smart Aviation Hub prototype designed for Changi Airport. It features RFID self-check-in, automated baggage measurement, Telegram luggage tracking, and biometric immigration gates built with Arduino, ESP32-CAM, Python, and Google Sheets.

Key Features
- Self Check-in Kiosk: Utilizes RFID technology to verify passenger identities and retrieve flight details from a cloud database.
- Baggage Measurement System: An automated station that uses an HX711 load cell and HC-SR04 ultrasonic sensors to weigh and measure luggage dimensions in real time.
- RFID Luggage Tracking: Passengers receive real-time location updates via a Telegram Bot as their bags move through the airport toward the aircraft.
- Automated Immigration Control: A multi-layered security gate combining RFID authentication and facial recognition (ESP32-CAM) to automate passenger clearance.
- Real-time Dashboard: A Google Sheets-powered interface that visualizes passenger throughput, airline-specific check-in trends, and baggage statistics.

Technical Architecture
Hardware Stack
The prototype is built on a modular architecture using the following components:
| Component | Function |
| :--- | :--- |
| Arduino Uno WiFi Rev 2 | Primary microcontroller and cloud communication bridge. |
| ESP32-CAM | Handles wireless facial recognition and video streaming. |
| RFID-RC522 | Scans passports and luggage tags for identity verification. |
| HX711 & Load Cell | Precisely weighs passenger baggage. |
| HC-SR04 (x5) | Measures baggage length, width, and height using ultrasonic waves. |
| ULN2003A & Stepper Motor | Acts as the physical mechanism for the automated immigration gate. |
Software Stack
- Python: Used for facial enrollment, training (LBPH recognizer), and the main recognition program.
- Google Apps Script: Orchestrates data flow between the hardware and the database.
- Google Sheets: Serves as the central database and real-time analytical dashboard.
- Telegram Bot API: Provides passengers with instant baggage status notifications.

System Workflow
- Check-in: The user taps an RFID passport; the system retrieves flight data from the AirlineDB and logs the check-in.
- Baggage: Luggage is weighed and measured; data is sent to Google Sheets, and the passenger is notified of the baggage location via Telegram.
- Immigration: The passenger taps their RFID tag at the gate. If authenticated, the ESP32-CAM performs facial recognition. Upon a match, the stepper motor opens the gate.
- Security: The system includes "Blacklisted Face Detection," which automatically terminates the program and keeps gates closed if a restricted individual is identified.
