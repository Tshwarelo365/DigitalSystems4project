#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>

#define PIR_PIN 4
#define ULTRASONIC_TRIGGER_PIN 19
#define ULTRASONIC_ECHO_PIN 23
#define GREEN_LED 18
#define RED_LED 5


// Set the LCD address to 0x27 for a 16 chars and 2 line display
LiquidCrystal_I2C lcd = LiquidCrystal_I2C(0x27, 16, 2);

float distance_cm, distance_m, length;
unsigned long duration_us; // Declare duration_us here
bool sensorsBlocked = false;

// Define keypad matrix
const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte rowPins[ROWS] = {26, 25, 17, 16}; // Connect to the row pinouts of the keypad
byte colPins[COLS] = {27, 14, 12, 13}; // Connect to the column pinouts of the keypad

// Create the Keypad object (outside of setup)
Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );
String secretPIN = "1234";
String enteredPIN = "";
int wrongAttempts = 0;

void setup() {
  Serial.begin(9600); // Initialize serial communication
  pinMode(PIR_PIN, INPUT);
  pinMode(ULTRASONIC_TRIGGER_PIN, OUTPUT);
  pinMode(ULTRASONIC_ECHO_PIN, INPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  
  
  
  lcd.begin(16,2);
  lcd.init();                      // Initialize the LCD
  lcd.backlight();                 // Turn on backlight
  String longText = "Welcome Smart Security System";          
  for (int i = 0; i < longText.length(); i++) {
    lcd.clear();                    // Clear the display
    lcd.setCursor(0, 0);            // Set cursor to the beginning of the first line
    lcd.print(longText.substring(i, (size_t)min((int)(i + 16), (int)longText.length()))); // Print a substring of longText
    lcd.setCursor(0, 1); 
    lcd.print("Loading...");
    delay(500);                     // Adjust delay for desired scrolling speed
  }
  delay(500);
  lcd.clear();
}

void loop() {
  // Check if sensors are blocked
  if (!sensorsBlocked) {
    // Motion detection with PIR sensor
    if ( isMotionDetected()) {
      blinkLed(RED_LED,2);
      
      lcd.setCursor(0, 0);
      lcd.print("Motion Detected!!!");
      
      
    } else {
      digitalWrite(RED_LED, LOW);
      
      lcd.setCursor(0, 0);
      lcd.print("No motion detected");
      delay(1000);
    }

    // Distance measurement with ultrasonic sensor
    ;

    if (distanceMeasurement() < 100) {
      digitalWrite(GREEN_LED, HIGH);
      delay(500);
      lcd.setCursor(0, 1);
      lcd.print("Object close!!!"); // Print label

    } else {
      
      digitalWrite(GREEN_LED, LOW);
      
      lcd.setCursor(0, 1);
      lcd.print("No object close");
      
      delay(500);
      
    }
  } else {
    // Sensors are blocked, clear the screen
    lcd.clear();
    delay(300); // Wait for 30 seconds
    sensorsBlocked = false; // Unblock the sensors
  }

  // Check for keypad input
  char key = keypad.getKey();
  if (key == 'A') {
    sensorsBlocked = true; // Block the sensors
    passwordEntry(); // Call password entry function
  } 

  delay(100); // Adjust delay for keypad responsiveness
}

void passwordEntry() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Enter PIN (4): ");
  enteredPIN = "";
  wrongAttempts = 0;

  while (true) {
    char key = keypad.getKey();
    if (key!= NO_KEY) {
      if (key == '#') {
        if (enteredPIN == secretPIN) {
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("PIN correct");
          lcd.setCursor(0, 1);
          lcd.print("User in the system");
          delay(2000);
          digitalWrite(GREEN_LED, HIGH);
          
          showMainMenu();
          mainMenuSelection();
          delay(2000);
          // Reset PIN entry
          enteredPIN = "";
          wrongAttempts = 0;
          // Display initial message
          lcd.clear();
          lcd.setCursor(0, 0);
          digitalWrite(GREEN_LED, LOW);
          
          break;
        } else {
          wrongAttempts++;
          if (wrongAttempts >= 3) {
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Attempts Exceeded");
            delay(1000);
            lcd.print("SYSTEM BREACHED GOING");
            lcd.setCursor(0, 1);
            lcd.print("BACK TO DEFAULT!!!");
            lcd.clear();
            delay(1000);
            digitalWrite(GREEN_LED, LOW);
            blinkLed(RED_LED, 7);
            
            delay(1000);
            sensorsBlocked = false; // Unblock the sensors
            break;
          } else {
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("PIN incorrect");
            blinkLed(RED_LED,3);
            // Reset PIN entry
            enteredPIN = "";
            // Display initial message
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Enter PIN (4): ");
          }
        }
      } else if( key == 'C') {

        sensorsBlocked = false; // Unblock the sensors
        break;

      } else {
        if (enteredPIN.length() < 4) {
          enteredPIN += key;
          lcd.setCursor(enteredPIN.length(), 1);
          lcd.print("*");
        }
      }
    }
  }
}


int distanceMeasurement() {
  // Read the distance value from the ultrasonic sensor
  digitalWrite(ULTRASONIC_TRIGGER_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(ULTRASONIC_TRIGGER_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(ULTRASONIC_TRIGGER_PIN, LOW);
  duration_us = pulseIn(ULTRASONIC_ECHO_PIN, HIGH);
  distance_cm = 0.017 * duration_us;
  
  
  return distance_cm;
}

bool isMotionDetected() {
  // Read the motion value from the PIR sensor
  if (digitalRead(PIR_PIN)== HIGH ){
    return true;
  } else {
    return false;
  }
}

void blinkLed(int ledPin, int blinkTimes) {
  for (int i = 0; i < blinkTimes; i++) {
    digitalWrite(ledPin, HIGH);
    delay(500);
    digitalWrite(ledPin, LOW);
    delay(500);
  }
}

void enterSecretPIN() {
  // Prompt the user to enter the secret PIN
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Enter secret PIN:");
  enteredPIN = "";
  // Wait for user input
  while (true) {
    char key = keypad.getKey();
    if (key!= NO_KEY) {
      if (key == '#') {
        // User has finished entering the PIN
        break;
      } else {
        // Add the entered digit to the PIN
        if (enteredPIN.length() < 4) {
          enteredPIN += key;
          lcd.setCursor(enteredPIN.length(), 1);
          lcd.print("*");
        }
      }
    }
  }
  // Save the entered PIN as the secret PIN
  secretPIN = enteredPIN;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("PIN saved!");
  delay(2000);
  // Display initial message
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Logging out. Log");
  lcd.setCursor(0, 1);
  lcd.print("in with new PIN");
}

void showMainMenu() {
  // Display the main menu options
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("1. Check motion");
  lcd.setCursor(0, 1);
  lcd.print("2. Check distance");
  delay(1000);
}

void mainMenuSelection() {
  char key;
  while ((key = keypad.getKey()) == NO_KEY); // Wait for a valid key press

  int distance; // Declare distance variable here

  switch (key) {
    case '1':
      // Check motion
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Motion detected:");
      if (isMotionDetected()) {
        lcd.setCursor(0, 1);
        lcd.print("Yes");
        blinkLed(GREEN_LED, HIGH);
      } else {
        lcd.setCursor(0, 1);
        lcd.print("No");
        digitalWrite(GREEN_LED, LOW);
      }
      delay(3000);
      break;
    case '2':
      // Check distance
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Distance in (cm):");
      distance = distanceMeasurement(); // Assign value here
      
      lcd.setCursor(0, 1);
      lcd.print(distance);
      delay(3000);
      break;
    case 'C':
      sensorsBlocked = false; // Unblock the sensors
      break;

    case 'A':
      enterSecretPIN();
      break;

    default:
      // Invalid selection
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Invalid selection");
      delay(2000);
      break;
  }
}
