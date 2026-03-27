// Code to count pulses from a sensor connected to pin 7 and measure the time taken for every 22.74 seconds,
// with visual feedback using an LED on pin 13. This version focuses on counting 
// the number of pulses in fixed time intervals rather than measuring the time for a fixed number of pulses.
// Note: 22.74s is the mean value of the time intervals measured in the previous version (sketch_may7a.ino) for every 5 pulses.



// Counter for tracking the number of detected pulses
int counter = 0;
// Pin connected to the input signal (sensor or signal source)
int pin = 7;

// Variable to store elapsed time in milliseconds (currently unused)
long elapsed_time = 0;

// Variable to store the initial time for measuring fixed time intervals
long initial_time = 0;

void setup() {
  // Initialize serial communication at 9600 baud for data transmission
  Serial.begin(9600);
  // Configure pin 7 as input to receive signals from external device
  pinMode(pin, INPUT);

  // Initialize the starting time to zero
  initial_time = 0;
}

void loop() {
  // Check if the input pin detects a high signal (pulse detected)
  if (digitalRead(pin) == 1) {
    // Increment the pulse counter when signal is detected
    counter++;
    // Small debounce delay (5 ms) to eliminate electrical noise and false triggers
    delay(5);
  }

  // Check if the fixed time interval (22740 milliseconds ≈ 22.74 seconds) has elapsed
  if (millis() - initial_time >= 22740) {
    // Send the total number of pulses counted during this time interval
    Serial.println(counter);
    // Reset the counter to zero for the next measurement period
    counter = 0;
    // Record the current time as the starting point for the next interval
    initial_time = millis();
  }
}