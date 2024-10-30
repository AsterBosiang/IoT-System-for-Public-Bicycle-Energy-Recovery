#include <LiquidCrystal_PCF8574.h>
#define RED_sensor 2      // Define infrared sensor pin
#define MAGNET_PIN 3      // Define magnetic solenoid pin
#define BUTTON_PIN 4      // Define button pin
#define buzzerPin 5       // Define buzzer pin

int beepDuration = 100;   // Duration of buzzer sound in milliseconds
int pauseDuration = 50;   // Pause duration between beeps in milliseconds
bool bikeTaken;           // Status of bike being taken
unsigned long startTime;  // Time when bike rental started
LiquidCrystal_PCF8574 lcd(0x27);  // Set I2C address for LCD
bool biketa;              // Flag to indicate bike rental activity
bool returnbike = true;   // Flag for returning bike
volatile bool buttonPressed = false;  // Tracks button press for interrupt

void setup() {
  pinMode(RED_sensor, INPUT);         // Set infrared sensor to input mode
  pinMode(MAGNET_PIN, OUTPUT);        // Set solenoid to output mode
  pinMode(BUTTON_PIN, INPUT_PULLUP);  // Set button to input mode with pull-up resistor
  pinMode(buzzerPin, OUTPUT);         // Set buzzer to output mode
  lcd.begin(16, 2);                   // Initialize LCD display
  lcd.setBacklight(255);              // Set LCD backlight
  lcd.clear();
  delay(500);

  // Set up button interrupt for quick response to button press
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), buttonISR, FALLING);
}

void loop() {
  returnbike = true;  // Enable bike return feature
  borrow_bike_all();  // Start bike borrowing process
}

// Display bike rental message on LCD
void rent_lcd() {
  lcd.clear();
  delay(1000);
  lcd.setCursor(5, 0);  // Set cursor position (column, row)
  lcd.print("Enjoy");
  lcd.setCursor(3, 1);
  lcd.print("Your  Bike");
  delay(1000);
}

// Display "take out" prompt on LCD
void take_out(){
  lcd.clear();
  lcd.setCursor(5, 0);
  lcd.print("Please");
  lcd.setCursor(3, 1);
  lcd.print("take  out");
  delay(1000);
  lcd.clear();
}

// Main bike borrowing function
void borrow_bike_all() {
  if (digitalRead(BUTTON_PIN) == HIGH) { // Check if button is pressed
    digitalWrite(MAGNET_PIN, HIGH);      // Activate solenoid to release bike
    for (int i = 0; i < 4; i++) {
      rent_lcd();                        // Display rental message 4 times
    }
    lcd.clear();                         // Clear LCD display
    
    startTime = millis();  // Record start time
    biketa = true;
    bikeTaken = false;
  }
  
  while (biketa) {  // Continuously monitor bike status
    if (buttonPressed) {  // Exit if button is pressed
      buttonPressed = false;
      return;
    }

    if (digitalRead(RED_sensor) == HIGH) { // If bike is taken out
      digitalWrite(MAGNET_PIN, LOW);      // Deactivate solenoid
      bikeTaken = true;                   // Mark bike as taken
      biketa = false;                     // Exit loop
      return_bike_all();                  // Proceed to bike return process
    }
    
    if (millis() - startTime >= 4000) { // Check if 4 seconds have passed
      if (!bikeTaken) {                 // If bike is not taken
        take_out();                     // Prompt user to take bike
        // Activate buzzer for alert signal
        digitalWrite(buzzerPin, HIGH); 
        delay(beepDuration);
        digitalWrite(buzzerPin, LOW); 
        delay(pauseDuration);
        digitalWrite(buzzerPin, HIGH);
        delay(beepDuration / 2);
        digitalWrite(buzzerPin, LOW);
        delay(beepDuration / 2);

        digitalWrite(buzzerPin, HIGH);
        delay(beepDuration / 2);
        digitalWrite(buzzerPin, LOW);
        delay(pauseDuration);

        digitalWrite(buzzerPin, HIGH);
        delay(beepDuration * 2); // Long beep
        digitalWrite(buzzerPin, LOW);
        delay(pauseDuration);
      }
      if (bikeTaken) {                 
        biketa = false;
      }
    }
    delay(500);  // Add delay to reduce processing load
  }
}

// Bike return process with LCD feedback
void return_bike_all(){
  lcd.clear();
  delay(1000);
  lcd.setCursor(3, 0);
  lcd.print("Please");
  lcd.setCursor(1, 1);
  lcd.print("Return Bike");
  delay(2000);  // Delay to ensure message visibility

  while (returnbike) {
    if (buttonPressed) {  // Exit if button is pressed
      buttonPressed = false;
      return;
    }

    if (digitalRead(RED_sensor) == LOW) {  // Detect bike return
        lcd.clear();
        lcd.setCursor(3, 0);
        lcd.print("Detecting");
        lcd.setCursor(1, 1);
        lcd.print("State Of Charge");
        delay(4000);  // Display for 4 seconds

        if (buttonPressed) {
            buttonPressed = false;
            return;
        }

        // Wait for button press to display "Discharge"
        while(digitalRead(BUTTON_PIN) == LOW){
            lcd.clear();
            delay(1000);
            lcd.setCursor(3, 0);
            lcd.print("Discharge...");
            delay(1000);  // Display "Discharge..." until button is pressed
        }
        
        borrow_bike_all();  // Re-enter bike borrowing process for a continuous loop
    }
  }
}

// Interrupt Service Routine for button press
void buttonISR() {
  buttonPressed = true;  // Set flag to indicate button press
}
