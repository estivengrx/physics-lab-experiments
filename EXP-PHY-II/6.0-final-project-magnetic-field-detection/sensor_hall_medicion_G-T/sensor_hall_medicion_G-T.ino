// Definición de pines
const int pinSensor = A0;      // Conecta la salida del sensor al pin analógico A0
const int led = 13;             // Define el pin del LED
const int digitalPin = 3;      // Define el pin digital para la lectura del sensor

// Factores de conversión y sensibilidad del sensor
const float GaussToTesla = 0.1; // Factor de conversión de Gauss a Tesla
const float sensitivity = 1.8;  // Sensibilidad del sensor para el cálculo del campo magnético
const float valorConstante = 1.43912; // Valor constante para calibrar el sensor

void setup() {
  pinMode(led, OUTPUT);        // Configura el pin del LED como salida
  pinMode(digitalPin, INPUT);  // Configura el pin digital para la lectura del sensor como entrada
  Serial.begin(9600);          // Inicializa la comunicación serial a 9600 baudios
}

void loop() {
  // Lee el estado digital del sensor
  int digitalVal = digitalRead(digitalPin);

  // Controla el estado del LED en función del estado digital del sensor
  if (digitalVal == HIGH) {
    digitalWrite(led, HIGH);  // Enciende el LED si el sensor detecta un campo magnético
  } else {
    digitalWrite(led, LOW);   // Apaga el LED si el sensor no detecta un campo magnético
  }

  // Lee el valor analógico del sensor
  int valorSensor = analogRead(pinSensor);

  // Convierte el valor a voltaje (considerando un Arduino con referencia de 5V)
  float voltaje = valorSensor * (5.0 / 1023.0);

  // Calcula la intensidad del campo magnético en unidades adecuadas
  float intensidadCampoGauss = (voltaje / sensitivity) - valorConstante;

  // Convierte la intensidad del campo a Tesla
  float intensidadCampoTesla = intensidadCampoGauss * GaussToTesla;

  // Muestra los datos en el Monitor Serie
  Serial.print("Estado Digital: ");
  Serial.print(digitalVal);
  Serial.print(", Valor del Sensor: ");
  Serial.print(valorSensor);
  Serial.print(",\t Voltaje (V): ");
  Serial.print(voltaje, 5); // Muestra el voltaje con 5 decimales
  Serial.print(",\t Intensidad del Campo Magnético (G): ");
  Serial.print(intensidadCampoGauss, 5); // Muestra la intensidad del campo con 5 decimales en mT
  Serial.print(",\t Densidad de Campo en Tesla (mT): ");
  Serial.println(intensidadCampoTesla, 5); // Muestra la densidad de campo con 5 decimales en Tesla

  delay(1000); // Espera 1 segundo antes de realizar la siguiente lectura
}