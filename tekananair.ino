#include <SoftwareSerial.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <TinyGPS++.h>

// Pin definitions
const int flowPin = 2;

// Variables for flowmeter
volatile int flowPulseCount = 0;
unsigned long oldTime = 0;
float flowRate = 0.0;
float totalLiters = 0.0;

// Initialize GPS
TinyGPSPlus gps;
SoftwareSerial ss(10, 11); // RX, TX

// Initialize OLED display
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Interrupt service routine for flowmeter
void flowISR() {
  flowPulseCount++;
}

void setup() {
  // Initialize serial communication
  Serial.begin(9600);
  ss.begin(9600);
  
  // Initialize flowmeter pin
  pinMode(flowPin, INPUT);
  attachInterrupt(digitalPinToInterrupt(flowPin), flowISR, RISING);

  // Initialize display
  if (!display.begin(SSD1306_I2C_ADDRESS, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }
  display.display();
  delay(2000);
  display.clearDisplay();
}

void loop() {
  // Calculate flow rate every second
  if ((millis() - oldTime) > 1000) {
    // Disable interrupt during calculation
    detachInterrupt(digitalPinToInterrupt(flowPin));
    
    // Calculate flow rate in liters/minute
    flowRate = (flowPulseCount / 7.5); 
    totalLiters += (flowRate / 60);
    
    // Reset pulse count and timer
    flowPulseCount = 0;
    oldTime = millis();
    
    // Re-enable interrupt
    attachInterrupt(digitalPinToInterrupt(flowPin), flowISR, RISING);

    // Display flow rate and total liters
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("Flow Rate (L/min):");
    display.println(flowRate);
    display.println("Total Liters:");
    display.println(totalLiters);
    
    // Display GPS data
    display.println("GPS Data:");
    if (gps.location.isValid()) {
      display.print("Lat: "); display.println(gps.location.lat(), 6);
      display.print("Lon: "); display.println(gps.location.lng(), 6);
    } else {
      display.println("Searching...");
    }
    display.display();
  }
  
  // Read GPS data
  while (ss.available() > 0) {
    gps.encode(ss.read());
  }
}