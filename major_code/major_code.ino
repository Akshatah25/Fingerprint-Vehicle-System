#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Adafruit_Fingerprint.h>
#include <SoftwareSerial.h>

// Initialize the LCD (I2C address 0x27 and size 16x2)
LiquidCrystal_I2C lcd(0x27, 16, 2);

// For UNO and others without hardware serial, use software serial
SoftwareSerial mySerial(2, 3); // RX, TX
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

// Define relay pin
#define relayPin 8
bool relayState = false; // To track relay ON/OFF state

void setup() {
  Serial.begin(9600);
  while (!Serial);  // For Yun/Leo/Micro/Zero/...
  delay(100);
  Serial.println("\n\nFingerprint Relay Toggle");

  // Initialize LCD
  lcd.init();                 // Initialize the LCD
  lcd.backlight();            // Turn on the backlight
  lcd.setCursor(0, 0);
  lcd.print("Initializing...");
  
  // Initialize the fingerprint sensor
  finger.begin(57600);
  delay(5);
  if (finger.verifyPassword()) {
    Serial.println("Found fingerprint sensor!");
    lcd.setCursor(0, 1);
    lcd.print("Sensor Ready");
  } else {
    Serial.println("Did not find fingerprint sensor :(");
    lcd.setCursor(0, 1);
    lcd.print("Sensor Error");
    while (1) { delay(1); }
  }

  // Initialize relay
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, LOW); // Ensure relay is off initially

  // Display sensor parameters
  finger.getParameters();
  Serial.print(F("Status: 0x")); Serial.println(finger.status_reg, HEX);
  Serial.print(F("Capacity: ")); Serial.println(finger.capacity);
  Serial.print(F("Template count: ")); Serial.println(finger.templateCount);

  delay(2000);
  lcd.clear();
  lcd.print("Waiting for FP");
}

void loop() {
  // Continuously check for a valid fingerprint
  int id = getFingerprintID();
  if (id != -1) {
    // Toggle the relay state if fingerprint matches
    relayState = !relayState;
    digitalWrite(relayPin, relayState ? HIGH : LOW);

    // Provide feedback via Serial and LCD
    Serial.print("Relay state: ");
    Serial.println(relayState ? "ON" : "OFF");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Engine: ");
    lcd.print(relayState ? "ON" : "OFF");

    delay(2000); // Avoid multiple triggers
    lcd.clear();
    lcd.print("Waiting for FP");
  }
  delay(50); // Reduce processing speed
}

int getFingerprintID() {
  uint8_t p = finger.getImage();
  if (p != FINGERPRINT_OK) return -1;

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK) return -1;

  p = finger.fingerFastSearch();
  if (p != FINGERPRINT_OK) return -1;

  // Match found
  Serial.print("Found ID #"); Serial.print(finger.fingerID);
  Serial.print(" with confidence "); Serial.println(finger.confidence);

  // Display ID on LCD
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("ID Found: ");
  lcd.print(finger.fingerID);
  lcd.setCursor(0, 1);
  lcd.print("Confidence: ");
  lcd.print(finger.confidence);

  delay(2000); // Display for 2 seconds
  return finger.fingerID;
}
