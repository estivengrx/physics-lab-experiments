// Code to count pulses from a sensor connected to pin 7 and measure the time taken for every 5 pulses,
// with visual feedback using an LED on pin 13.

// Counter for detecting pulses on the input pin
int counter = 0;
// Pin connected to the input signal (sensor or signal source)
int pin = 7;

// Variable to store elapsed time in milliseconds
long elapsed_time = 0;

// Variable to store the initial time for measuring intervals
long initial_time = 0;

void setup() {
  // Initialize serial communication at 9600 baud for data transmission
  Serial.begin(9600);
  // Configure pin 7 as input (receives signal from external device)
  pinMode(pin, INPUT);
  // Configure pin 13 as output (controls LED feedback)
  pinMode(13, OUTPUT);

  // Initialize the starting time to zero
  initial_time = 0;
}

void loop() {
  // Check if the input pin detects a high signal (rising edge or continuous signal)
  if (digitalRead(pin) == 1) {
    // Increment the pulse counter
    counter++;
    // Debounce delay to avoid false triggering from electrical noise
    delay(10);
    // Turn on LED on pin 13 as visual feedback for detected pulse
    digitalWrite(13, 1);
    // Keep LED on for 100 milliseconds
    delay(100);
    // Turn off LED after feedback period
    digitalWrite(13, 0);
  }

  // Check if we have counted 5 pulses
  if (counter == 5) {
    // Calculate the elapsed time since the last measurement in milliseconds
    elapsed_time = millis() - initial_time;
    // Send the elapsed time through serial port for monitoring/logging
    Serial.println(elapsed_time);
    // Reset the counter to zero for the next measurement cycle
    counter = 0;
    // Record the current time as the new starting point for the next interval
    initial_time = millis();
  }
}