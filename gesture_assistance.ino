/*
 * PROJECT: Gesture Assistant (Final Fixed Version)
 * SENSOR:  SparkFun APDS-9960
 * MODE:    Polling (Continuous Check) - No Interrupt Pin needed
 */

#include <Wire.h>
#include <SparkFun_APDS9960.h>
#include <LiquidCrystal_I2C.h>

// --- HARDWARE CONFIGURATION ---
LiquidCrystal_I2C lcd(0x27, 16, 2); 
SparkFun_APDS9960 apds = SparkFun_APDS9960();

// Variables
int currentMode = 1;
const unsigned long COMBO_TIMEOUT = 1000; 

// ============================================================

// Forward declaration
void showModeStatus(); 

void displayMessage(String line1, String line2 = "") {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(line1);
  lcd.setCursor(0, 1);
  lcd.print(line2);
  // Keep the text on screen for 3 seconds
  delay(3000); 
  showModeStatus(); // Go back to "Ready" screen
}

void showModeStatus() {
  lcd.clear();
  lcd.setCursor(0, 0);
  if (currentMode == 1) {
    lcd.print("M1: Greetings");
  } else {
    lcd.print("M2: Needs");
  }
  lcd.setCursor(0, 1);
  lcd.print("Ready...");
}

// Function to check for mode switching combos
void checkForModeSwitch(int targetGesture, int newMode, String line1, String line2) {
  
  lcd.clear();
  lcd.print("..."); // Visual cue that we are waiting
  
  unsigned long startTime = millis();
  bool switched = false;
  
  // Wait loop
  while ((millis() - startTime) < COMBO_TIMEOUT) {
    if (apds.isGestureAvailable()) {
      int nextGesture = apds.readGesture();
      
      // If the NEXT gesture matches our target, we switch!
      if (nextGesture == targetGesture) {
        currentMode = newMode;
        switched = true;
        lcd.clear();
        lcd.print("Switched to");
        lcd.setCursor(0, 1);
        lcd.print("Mode " + String(currentMode));
        delay(1500);
        showModeStatus(); // Show the menu for the new mode
        break; // Exit loop
      }
    }
  }
  
  // If we didn't switch, it means it was just a normal single swipe
  if (!switched) {
    displayMessage(line1, line2);
  }
}

// ============================================================
// 2. LOGIC ENGINE
// ============================================================
void handleGesture() {
  // Check if gesture is present
  if (apds.isGestureAvailable()) {
    
    int gesture = apds.readGesture();
    
    // --- LOGIC FOR MODE 1 ---
    if (currentMode == 1) {
      switch (gesture) {
        case DIR_UP:
          displayMessage("Hello, myself", "Abhijeeth");
          break;
        case DIR_DOWN:
          displayMessage("Thank you");
          break;
        case DIR_LEFT:
          displayMessage("Good bye");
          break;
        case DIR_RIGHT:
          // *** COMBO CHECK (Right -> Left) ***
          checkForModeSwitch(DIR_LEFT, 2, "I didn't", "understand");
          break;
      }
    }
    
    // --- LOGIC FOR MODE 2 ---
    else if (currentMode == 2) {
      switch (gesture) {
        case DIR_UP:
          displayMessage("I am not", "feeling well");
          break;
        case DIR_DOWN:
          displayMessage("I need medicine");
          break;
        case DIR_RIGHT:
          displayMessage("I am hungry");
          break;
        case DIR_LEFT:
          // *** COMBO CHECK (Left -> Right) ***
          checkForModeSwitch(DIR_RIGHT, 1, "Where is the", "restroom");
          break;
      }
    }
  }
}

// ============================================================
// 3. SETUP & MAIN LOOP
// ============================================================
void setup() {
  Serial.begin(9600);
  
  lcd.init();
  lcd.backlight();
  
  lcd.print("Init Sensor...");
  delay(500);

  // Initialize Sensor
  if (apds.init()) { 
    Serial.println("Sensor Init: SUCCESS"); 
  } else { 
    // Force continue even if ID fails
    Serial.println("Sensor Init: FAILED (Forced)"); 
  }
  
  // Increase Gain (Sensitivity) - IMPORTANT FOR CLONES
  apds.setGestureGain(GGAIN_4X);
  apds.setGestureLEDDrive(LED_DRIVE_100MA); // Max Power

  // Start Gesture Engine
  if (apds.enableGestureSensor(true)) { 
    Serial.println("Gesture Engine: RUNNING"); 
    lcd.clear();
    lcd.print("Engine ON");
    delay(1000);
  } else {
    Serial.println("Gesture Engine: FAILED");
    lcd.clear();
    lcd.print("Engine Fail");
    delay(1000);
  }
  
  showModeStatus();
}

void loop() {
  // CONSTANTLY CHECK (Polling)
  handleGesture();
  
  // Small delay to prevent crashing
  delay(100); 
}
