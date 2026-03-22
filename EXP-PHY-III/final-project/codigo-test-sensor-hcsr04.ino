#define PIN_TRIG 2
#define PIN_ECHO 3
 
long duration, cm;
 
void setup() {
 
  Serial.begin (9600);
  // Inicializar la comunicación del puerto serie a 9600
  pinMode(PIN_TRIG, OUTPUT);
  // pinMode(PIN_ECHO, INPUT);
}
 
void loop() {
 
  // Primero, generar un pulso corto de 2-5 microsegundos.
 
  // digitalWrite(PIN_TRIG, LOW);
  // delayMicroseconds(5);
  digitalWrite(PIN_TRIG, HIGH);
  delayMicroseconds(100);
  digitalWrite(PIN_TRIG, LOW);
 
  // Después de ajustar un nivel de señal alto, esperamos unos 10 microsegundos. En este punto el sensor enviará señales con una frecuencia de 40 kHz.
  // delayMicroseconds(10);
 
  // Tiempo de retardo de la señal acústica en el sonar.
  // duration = pulseIn(PIN_ECHO, HIGH);
 
  // // Ahora es el momento de convertir el tiempo a distancia
  // cm = (duration / 2) / 29.1;
 
  // Serial.print("Distancia al objeto: ");
  // Serial.print(cm);
  // Serial.println(" см.");
 
  // Retraso entre mediciones para el correcto funcionamiento del ejemplo
  delayMicroseconds(100);
}