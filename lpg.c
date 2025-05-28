#include <HX711 ADC.h>
#include <Wire.h>
#include <LiquidCrystal I2C.h>
#include <SoftwareSerial.h>
#if defined(ESP8266)||defined(ESP32) ||defined(AVR)
#include <EEPROM.h>
#endif
// HX711 pins:
const int HX711 dout = 4; // MCU > HX711 DOUT pin
const int HX711 sck = 5; // MCU > HX711 SCK pin
// GSM module pins (SIM800C):
const int GSM TX = 11; // Arduino TX − > GSM RX
const int GSM RX = 10; // Arduino RX − > GSM TX
// LCD setup:
LiquidCrystal I2C lcd(0x27, 16, 2); // I2C address 0x27, 16 columns, 2 rows
// HX711 constructor:
HX711 ADC LoadCell(HX711 dout, HX711 sck);
// GSM module setup:
SoftwareSerial gsm(GSM RX, GSM TX); // GSM communication on pins 10 and 11
const int calVal eepromAdress = 0; // EEPROM address for calibration value
const float maxWeight = 5000.0; // Maximum weight (5kg)
const float targetWeight = 1000.0; // Weight threshold to send SMS (1kg)
unsigned long t = 0;
bool messageSent = false; // Flag to avoid multiple SMS messages
Dept.of E&IE, S.I.T.,Tumakuru-03 25
Smart LPG Gas Level Detection and Community Sharing System with Real-Time
Alerts and Web Access 2024-25
String dummyBookingNumber = “ABC123456”; // Dummy booking number with alphabet
String dummyCallNumber = “+916361510297”; // Dummy number for call
String callno = “+917483571709”; // User’s phone number to call
void setup() {
Serial.begin(57600);
gsm.begin(9600); // Initialize SIM800C at 9600 baud rate
delay(10);
Serial.println();
Serial.println(“Starting...”);
// Initialize LCD:
lcd.init();
lcd.backlight();
lcd.setCursor(0, 0);
lcd.print(“Initializing...”);
// Initialize HX711:
LoadCell.begin();
float calibrationValue = 696.0; // Calibration value
EEPROM.get(calVal eepromAdress, calibrationValue); // Fetch calibration value from
EEPROM
unsigned long stabilizingtime = 2000; // Stabilizing time after power-up
boolean tare = true; // Perform tare during initialization
LoadCell.start(stabilizingtime, tare);
if (LoadCell.getTareTimeoutFlag()) {
Serial.println(“Timeout, check MCU>HX711 wiring and pin designations”);
lcd.setCursor(0, 1);
lcd.print(“Error: Timeout”);
while (1);
} else {
LoadCell.setCalFactor(calibrationValue); // Set calibration value
Dept.of E&IE, S.I.T.,Tumakuru-03 26
Smart LPG Gas Level Detection and Community Sharing System with Real-Time
Alerts and Web Access 2024-25
Serial.println(“Startup is complete”);
lcd.setCursor(0, 1);
lcd.print(“Ready”);
}
delay(2000);
lcd.clear();
// Test GSM module:
testGSM();
}
void loop() {
static boolean newDataReady = 0;
const int serialPrintInterval = 500; // Interval for serial printing
// Check for new data and start the next conversion:
if (LoadCell.update()) newDataReady = true;
if (newDataReady) {
if (millis() > t + serialPrintInterval) {
float weight = LoadCell.getData(); // Get the weight in grams
float percentage = (weight / maxWeight) * 100.0; // Calculate percentage
// Display weight and percentage on LCD:
lcd.setCursor(0, 0);
lcd.print(“Weight: ”);
lcd.print(weight / 1000.0, 2); // Convert to kg and display
lcd.print(“ kg”);
lcd.setCursor(0, 1);
lcd.print(“Per: ”);
lcd.print(percentage, 1); // Display percentage
lcd.print(“%”);
// Print to Serial Monitor:
Serial.print(“Load cell output val: ”);
Serial.print(weight);
Serial.print(“ ( ”);
Dept.of E&IE, S.I.T.,Tumakuru-03 27
Smart LPG Gas Level Detection and Community Sharing System with Real-Time
Alerts and Web Access 2024-25
Serial.print(percentage);
Serial.println(“ )”);
// Send SMS if weight equals 1kg:
if (fabs(weight - targetWeight) ¡= 10 && !messageSent) { // Allow for slight measurement
variation
sendSMS(weight);
makeCall();
messageSent = true; // Prevent multiple messages
}
newDataReady = 0;
t = millis(); }
}
// Receive command from serial terminal to initiate tare operation:
if (Serial.available() ¿ 0) {
char inByte = Serial.read();
if (inByte == ’t’) {
lcd.setCursor(0, 1);
lcd.print(“Taring...”);
LoadCell.tareNoDelay();
messageSent = false; // Reset SMS flag after tare
}
}
// Check if the tare operation is complete:
if (LoadCell.getTareStatus() == true) {
Serial.println(“Tare complete”);
lcd.setCursor(0, 1);
lcd.print(“Tare complete”);
delay(2000);
lcd.clear();
}
}
Dept.of E&IE, S.I.T.,Tumakuru-03 28
Smart LPG Gas Level Detection and Community Sharing System with Real-Time
Alerts and Web Access 2024-25
void sendSMS(float weight) {
String message = “Alert! LPG Gas Cylinder has reached−− >” + String(weight / 1000.0,
2) + “ kg. ” \n“; message += ”Book LPG Gas Cylinder Immediately.\n“; message +=
”Booking Number: “ + dummyBookingNumber + ”\n“; message += ”Call for booking:
tel:“ + dummyCallNumber + ”\n“; // Add the dummy call number as a link
Serial.println(”Sending SMS...“);
gsm.println(”AT“); // Test AT command
delay(100);
if (gsm.find(”OK“)) {
Serial.println(”GSM Module is ready“);
} else {
Serial.println(”GSM Module not responding“);
lcd.setCursor(0, 1);
lcd.print(”GSM Error“);
return;
}
gsm.println(”AT+CMGF=1“); // Set SMS text mode
delay(100);
gsm.println(”AT+CMGS=“+917483571709”“); // Replace with recipient’s phone number
delay(100);
gsm.print(message); // SMS content
delay(100);
gsm.write(26); // CTRL+Z to send SMS
delay(5000); // Wait for the SMS to send
if (gsm.find(”OK“)) {
Serial.println(”Message sent successfully!“);
lcd.setCursor(0, 1);
lcd.print(”SMS Sent!“);
} else {
Serial.println(”Failed to send message.“);
Dept.of E&IE, S.I.T.,Tumakuru-03 29
Smart LPG Gas Level Detection and Community Sharing System with Real-Time
Alerts and Web Access 2024-25
lcd.setCursor(0, 1);
lcd.print(”SMS Fail“);
}
delay(2000);
lcd.clear();
}
void makeCall() {
Serial.println(” Making a call”);
lcd.setCursor(0, 0);
lcd.print(”Calling user“); // Display calling message on LCD
gsm.println(”ATD“ + callno + ”;“); // Make a call to the user number
delay(10000); // Wait for the call duration (10 seconds)
gsm.println(”ATH“); // Hang up the call
Serial.println(”Call ended.“);
// Display call ended on LCD
lcd.setCursor(0, 0);
lcd.print(”Call ended.“);
delay(2000); lcd.clear();
}
void testGSM() {
Serial.println(”Testing GSM Module...“);
gsm.println(”AT“);
delay(1000);
if (gsm.find(”OK“)) { Serial.println(”GSM Module is working“);
} else { Serial.println(” GSM Module not responding.“);
lcd.setCursor(0, 1);
lcd.print(”GSM Error“);
while (1);
}
