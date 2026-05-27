#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

Adafruit_MPU6050 mpu;
char datoLeido;

int total_muestras = 300;
int contador = 0;

void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22);

  if (!mpu.begin()) {
    Serial.println("No se encontró el MPU6050");
    while (1);
  }

}

void loop() {

  // Leer dato del puerto serial si hay algo disponible
  if (Serial.available() > 0) {
    datoLeido = Serial.read();
    contador = 0;  // reiniciar conteo cada vez que llega 'a'
  }

  if (datoLeido == 'a') {
    if (contador < total_muestras) {

      sensors_event_t a, g, temp;
      mpu.getEvent(&a, &g, &temp);

      // Enviar datos en formato CSV: x,y,z
      Serial.print(a.acceleration.x);
      Serial.print(",");
      Serial.print(a.acceleration.y);
      Serial.print(",");
      Serial.println(a.acceleration.z);

      contador++;
      delay(50); // ~20 Hz

    } else if (datoLeido != 0){
      Serial.println("FIN");
      datoLeido = 0;  // detener adquisición hasta que envíes 'a' otra vez
    }
  }
}
