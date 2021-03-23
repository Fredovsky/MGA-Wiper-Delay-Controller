#include <Arduino.h>

/* I/O PARAMETERS */
#define PIN_WIPER_OUTPUT 3 // PIN to wiper relay
#define PIN_INPUT_SWITCH 4 // Pin reading wiper dashboard switch position
#define SWITCH_DEBOUNCE_DELAY_MS 50 // Switch debounce delay in ms
#define TOGGLE_LIMIT_DELAY 2000 // Time to consider an ON-OFF-ON as a toggle

/* INTERMITTENT CONTROL PARAMETERS */
#define INTERMITTENT_1_MS 10000 // Intermittent delay 1 in ms
#define INTERMITTENT_2_MS 7500 // Intermittent delay 2 in ms
#define INTERMITTENT_3_MS 5000 // Intermittent delay 3 in ms
#define INTERMITTENT_4_MS 2500 // Intermittent delay 4 in ms

/* WIPER MOTOR RELATED PARAMETERS */
#define RELAY_TIME_MS 500 // Duration (ms) the relay stays closed to initiate the wiper swipe

uint8_t toggles = 0; // Track the ON->OFF->ON toggles
uint8_t switchState = LOW; // Keep track of previous switch status
uint8_t lastSwitchState = LOW;   // the previous reading from the switch
uint8_t forceWipe = false;  // Force a wipe now
unsigned long lastWipeTime = 0;
unsigned long lastOffTime = 0;  // Last time the switch was in the OFF position
unsigned long lastDebounceTime = 0;  // the last time the switch was toggled
unsigned long debounceDelay = 50;    // the debounce time in ms

void setup() {
  // Initialize PIN modes
  pinMode(PIN_WIPER_OUTPUT, OUTPUT);
  pinMode(PIN_INPUT_SWITCH, INPUT);

  // If wiper switch ON at power up, activates wipers
  if(digitalRead(PIN_INPUT_SWITCH)){
    switchState = HIGH;
    lastSwitchState = HIGH;
  }
}

void wipe() {
  // Wipe outputs based on switch position and toggle numbers
  if(switchState == HIGH) {
    int intermittentDelay = 0;
    // Wipers are on
    switch (toggles) {
      case 0:
        intermittentDelay = INTERMITTENT_1_MS;
        break;
      case 1:
        intermittentDelay = INTERMITTENT_2_MS;
        break;
      case 2:
        intermittentDelay = INTERMITTENT_3_MS;
        break;
      case 3:
        intermittentDelay = INTERMITTENT_4_MS;
        break;
      default:
        // Continuous wipers
        intermittentDelay = 0;
        break;
    }

    if(millis()-lastWipeTime > intermittentDelay || forceWipe) {
      //Send a wipe pulse
      digitalWrite(PIN_WIPER_OUTPUT, HIGH);
      lastWipeTime = millis();
      forceWipe = false;
    }

    if(millis()-lastWipeTime > RELAY_TIME_MS) {
      // End the pulse after the required delay
      digitalWrite(PIN_WIPER_OUTPUT, LOW);
    }

  } else {
    // digitalWrite(PIN_WIPER_OUTPUT, LOW);
  }
}

void loop() {

  // read the state of the switch into a local variable:
  int reading = digitalRead(PIN_INPUT_SWITCH);

  // check to see if you just switched the button
  // and you've waited long enough
  // since the last change to ignore any noise:

  // If the switch changed, due to noise or pressing:
  if (reading != lastSwitchState) {
    // reset the debouncing timer
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    // whatever the reading is at, it's been there for longer than the debounce
    // delay, so take it as the actual current state:

    // if the button state has changed:
    if (reading != switchState) {
      switchState = reading;

      // If ON to OFF detected
      if(switchState == LOW){
        lastOffTime = millis();
        digitalWrite(PIN_WIPER_OUTPUT, LOW); // Switch OFF the wipers (in case it's not a toggle, just an OFF action)
      }

      // If OFF to ON detected
      if(switchState == HIGH) {
        // Force a wipe as we either just switched on the wipers or are increasing the frequency
        forceWipe = true;
        if(millis()-lastOffTime < TOGGLE_LIMIT_DELAY) {
          // Toggle detected
          toggles++;
        } else {
          // Switching to ON only (not a toggle)
          toggles = 0;
        }
      }
    }
  }
lastSwitchState = reading;

// Finally...
wipe();
delay(10);
}
