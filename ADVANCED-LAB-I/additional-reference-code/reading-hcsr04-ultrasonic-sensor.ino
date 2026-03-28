// Pin configuration for the ultrasonic sensor HC-SR04
const int trigPin = 16;   // Trigger pin (sends the ultrasonic pulse)
const int echoPin = 17;   // Echo pin (receives the reflected signal)

// Variables to store measurement data
long duration;            // Time taken for the echo to return (in microseconds)
float distance;           // Calculated distance (in centimeters)
char datoLeido;           // Character received from serial communication

void setup() {
  // Configure pin modes
  pinMode(trigPin, OUTPUT);   // Trigger pin as output
  pinMode(echoPin, INPUT);    // Echo pin as input

  // Initialize serial communication at 115200 baud rate
  Serial.begin(115200);
}

void loop() {
  // Check if data is available in the serial buffer
  if (Serial.available() > 0) {
    
    // Read one character from the serial input
    datoLeido = Serial.read();

    // If the received character is 'a', start measurements, this 
    // allows us to control when to take measurements from the Python script.
    if (datoLeido == 'a') {

      // Take 10 consecutive distance measurements
      for (int i = 0; i < 10; i++) {

        // rigger the ultrasonic pulse
        digitalWrite(trigPin, LOW);        // Ensure trigger is LOW
        delayMicroseconds(5);
        digitalWrite(trigPin, HIGH);       // Send HIGH pulse for 10 microseconds
        delayMicroseconds(10);
        digitalWrite(trigPin, LOW);        // End pulse

        // Measure the echo response
        duration = pulseIn(echoPin, HIGH); // Measure how long echo pin stays HIGH

        // Convert time to distance
        // Speed of sound = 0.0343 cm/us
        // Divide by 2 because the signal travels to the object and back
        distance = (duration * 0.0343) / 2;

        // Send the measured distance via serial (one value per line)
        Serial.println(distance);

        // Small delay between measurements to stabilize readings
        delay(100);
      }
    }
  }
}