#include <SPI.h>
#include <MFRC522.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>

// Pin Definitions
#define RST_PIN 9
#define SS_PIN 10
#define BUZZER_PIN 6
#define RELAY_PIN 5

MFRC522 mfrc522(SS_PIN, RST_PIN);
LiquidCrystal_I2C lcd(0x27, 16, 2); // LCD I2C address: 0x27, 16x2 display
SoftwareSerial gsm(7, 8); // TX, RX for GSM module

// Predefined authorized RFID tags (Replace with actual UIDs)
String authorizedUIDs[] = {"3B2A1C9F", "E491B327"}; // Example UIDs (No spaces)

// Setup function
void setup() {
    Serial.begin(9600);
    gsm.begin(9600);
    SPI.begin();
    mfrc522.PCD_Init();
    lcd.init();
    lcd.backlight();

    pinMode(BUZZER_PIN, OUTPUT);
    pinMode(RELAY_PIN, OUTPUT);
    digitalWrite(RELAY_PIN, LOW);

    lcd.setCursor(0, 0);
    lcd.print("RFID Secure Access!");
    delay(2000);
    lcd.clear();
}

void loop() {
    Serial.println("Waiting for RFID Card...");

    // Check if a new card is present
    if (!mfrc522.PICC_IsNewCardPresent()) {
        return;
    }
    if (!mfrc522.PICC_ReadCardSerial()) {
        return;
    }

    // Read RFID UID
    String cardUID = getCardUID();
    Serial.println("Scanned UID: " + cardUID);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Scanning...");

    if (isAuthorized(cardUID)) {
        Serial.println("Access Granted!");
        lcd.setCursor(0, 0);
        lcd.print("Access Granted!");
        digitalWrite(RELAY_PIN, HIGH);
        delay(3000); // Unlock for 3 seconds
        digitalWrite(RELAY_PIN, LOW);
    } else {
        Serial.println("Access Denied!");
        lcd.setCursor(0, 0);
        lcd.print("Access Denied!");
        digitalWrite(BUZZER_PIN, HIGH);
        delay(2000);
        digitalWrite(BUZZER_PIN, LOW);

        // Send alert via GSM
        sendSMS("ALERT! Unauthorized Access Attempt Detected.");
    }

    delay(2000);
    lcd.clear();
    mfrc522.PICC_HaltA();
}

// Function to get RFID UID as a string
String getCardUID() {
    String uid = "";
    for (byte i = 0; i < mfrc522.uid.size; i++) {
        uid += String(mfrc522.uid.uidByte[i], HEX); // Convert byte to HEX
    }
    uid.toUpperCase(); // Convert to uppercase
    return uid;
}

// Function to check if the scanned UID is authorized
bool isAuthorized(String uid) {
    for (int i = 0; i < (sizeof(authorizedUIDs) / sizeof(authorizedUIDs[0])); i++) {
        if (uid.equalsIgnoreCase(authorizedUIDs[i])) {
            return true;
        }
    }
    return false;
}

// Function to send SMS via GSM
void sendSMS(String message) {
    Serial.println("Sending SMS...");
    gsm.println("AT+CMGF=1"); // Set SMS mode to text
    delay(1000);
    gsm.println("AT+CMGS=\"+918866416817\""); // Replace with your phone number
    delay(1000);
    gsm.print(message);
    delay(1000);
    gsm.write(26); // End SMS with Ctrl+Z
    delay(3000);
    Serial.println("SMS Sent!");
}
