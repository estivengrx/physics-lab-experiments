// Code to characterize a nitrogen diode using an Arduino. 
// The code generates a PWM signal to vary the voltage across the diode and measures the 
// voltage and current to plot the I-V curve.
// Output is printed in CSV format for easy import into Python.

// Circuit parameters
const int pwmPin = 11;     // PWM pin to generate voltage
const int pinA0 = A0;      // Measurement after the resistor (voltage across the diode)
const int pinA1 = A1;      // Measurement before the resistor
const float R = 20000.0;    // Resistor value in ohms

// Arduino parameters
const float Vcc = 5.0;          // Reference voltage
const int adcResolution = 1023; // 10 bits

void setup() {
  Serial.begin(9600);
  pinMode(pwmPin, OUTPUT);
  Serial.println("Vdiode(V), Idiode(A)");
}

void loop() {
  for (int duty = 0; duty <= 255; duty += 1) {
    analogWrite(pwmPin, duty);
    delay(50); 

    // Readings
    int rawA0 = analogRead(pinA0);
    int rawA1 = analogRead(pinA1);

    // Conversion to voltages
    float VA0 = (rawA0 * Vcc) / adcResolution;
    float VA1 = (rawA1 * Vcc) / adcResolution;

    // Voltage across the diode
    float Vd = VA0;

    // Current (I = (VA1 - VA0)/R)
    float I = (abs(VA1 - VA0)) / R;

    Serial.print(Vd, 4);
    Serial.print(",");
    Serial.println(I, 6);
  }

  delay(10000000); // Wait before repeating sweep
}