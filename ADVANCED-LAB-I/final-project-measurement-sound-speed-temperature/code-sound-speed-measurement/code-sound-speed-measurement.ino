/*
  Velocidad del sonido en funcion de la temperatura
  Sensores: HC-SR04 + DHT11
  Placa:    ACEBOTT ESP32-Max-V1.0

  CONEXIONES:
    DHT11:
      DATA -> GPIO5
      VCC  -> 3V3
      GND  -> GND

    HC-SR04:
      TRIG -> GPIO23
      ECHO -> GPIO19
      VCC  -> 5V
      GND  -> GND

  PROTOCOLO:
    Python envia 'a' por serial -> ESP32 empieza a medir y enviar datos
    Python envia 's' por serial -> ESP32 se detiene
    Formato de salida: duracion_us,temperatura_C,humedad_%
*/

#include <DHT.h>

#define DHTPIN    5
#define DHTTYPE   DHT11
#define TRIG_PIN  23
#define ECHO_PIN  19

DHT dht(DHTPIN, DHTTYPE);

char    comando    = ' ';
bool    midiendo   = false;
const unsigned long INTERVALO_MS = 500;  // cada cuanto medir (ms)
unsigned long ultimaMedicion     = 0;

void setup() {
  Serial.begin(115200);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  digitalWrite(TRIG_PIN, LOW);
  dht.begin();
  delay(1000);  // estabilizacion DHT11
}

void loop() {

  // --- Leer comando de Python ---
  if (Serial.available() > 0) {
    comando = Serial.read();
    if (comando == 'a') {
      midiendo = true;
    } else if (comando == 's') {
      midiendo = false;
    }
  }

  // --- Tomar medicion si corresponde ---
  if (midiendo && (millis() - ultimaMedicion >= INTERVALO_MS)) {
    ultimaMedicion = millis();

    // 1. Temperatura y humedad
    float temperatura = dht.readTemperature();
    float humedad     = dht.readHumidity();

    // 2. Pulso ultrasonico
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);
    long duracion = pulseIn(ECHO_PIN, HIGH, 30000UL);  // timeout 30 ms

    // 3. Enviar solo si todo es valido
    if (!isnan(temperatura) && !isnan(humedad) && duracion > 0) {
      Serial.print(duracion);
      Serial.print(",");
      Serial.print(temperatura, 1);
      Serial.print(",");
      Serial.println(humedad, 1);
    }
  }
}