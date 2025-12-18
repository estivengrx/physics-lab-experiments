// int led = 13;
// int digitalPin = 3;
// int analogPin = A0;
// int digitalVal;
// int analogVal;

// void setup() {
//   // put your setup code here, to run once:
//   pinMode(led, OUTPUT);
//   pinMode(digitalPin, INPUT);
//   Serial.begin(9600);
// }

// void loop() {
//   // put your main code here, to run repeatedly:
//   digitalVal = digitalRead(digitalPin);
//   if(digitalVal == HIGH)
//   {
//     digitalWrite(led, HIGH);
//   }
//   else
//   {
//     digitalWrite(led, LOW);
//   }

//   analogVal = analogRead(analogPin);
//   Serial.println(analogVal);
//   delay(200);
// }
// int led = 13;
// int digitalPin = 3;
// int analogPin = A0;
// int digitalVal;
// int analogVal;
// int sensitivity = 1.8;

// void setup() {
//   pinMode(led, OUTPUT);
//   pinMode(digitalPin, INPUT);
//   Serial.begin(9600);
// }

// void loop() {
//   digitalVal = digitalRead(digitalPin);
//   if (digitalVal == HIGH) {
//     digitalWrite(led, HIGH);
//   } else {
//     digitalWrite(led, LOW);
//   }

//   analogVal = analogRead(analogPin);

//   // Transforma la lectura analógica de 0 a 1023 a un voltaje de 0 a 5V
//   float volt = analogVal * (5.0 / 1023.0);

//   // Transforma el voltaje a Gauss (ajusta estos valores según las especificaciones del sensor)
//   float magneticField = (volt) * (1.0 / sensitivity) - 2.59; // Convierte a Gauss

//   // Envía los datos al Monitor Serie
//   Serial.print("Digital: ");
//   Serial.print(digitalVal);
//   Serial.print(", Analog (volts): ");
//   Serial.print(volt, 2);  // Muestra el voltaje con dos decimales
//   Serial.print(", Analog (gauss): ");
//   Serial.println(magneticField);

//   delay(200);
// }
// const int pinSensor = A0; // Conecta la salida del sensor al pin analógico A0
// const float sensibilidad = 1.8; // Ajusta según la sensibilidad del sensor en mV/G (o mV/mT) según la hoja de datos

// void setup() {
//   Serial.begin(9600);
// }

// void loop() {
//   int valorSensor = analogRead(pinSensor); // Lee el valor analógico del sensor
//   float voltaje = valorSensor * (5.0 / 1023.0); // Convierte el valor a voltaje (si estás usando un Arduino con referencia de 5V)
  
//   // Realiza el cálculo para obtener la intensidad del campo magnético en unidades adecuadas
//   // (deberías ajustar esto según la sensibilidad del sensor indicada en la hoja de datos)
//   float intensidadCampo = voltaje / sensibilidad; 

//   // Imprime el valor en el puerto serie con unidades
//   Serial.print("Valor del sensor: ");
//   Serial.print(valorSensor);
//   Serial.print("\t Voltaje: ");
//   Serial.print(voltaje, 3); // Muestra el voltaje con 3 decimales
//   Serial.print(" V\t Intensidad del campo magnético: ");
//   Serial.print(intensidadCampo - 1.439, 3); // Muestra la intensidad del campo con 3 decimales
//   Serial.println(" mT");

//   delay(1000); // Espera 1 segundo antes de realizar la siguiente lectura
// }
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