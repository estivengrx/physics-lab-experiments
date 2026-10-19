#include <math.h>  // para operaciones matemáticas

const int comandoEmpezar = 1;

const int PIN_AO = 34;
const int PIN_DO = 27;
const int PIN_LED = 2;
const int umbral = 35;
const int PIN_BOTON = 0;
const int PIN_LED_EXT = 26;

// Aquí la declaración de colas
QueueHandle_t queueMensajes;
QueueHandle_t queueSensor;
QueueHandle_t queueTitilar;

// Aquí la declaración de las tareas
void taskLeerSerial(void* param);  // Tarea #1: recibir un comando por serial, ese comando activará las siguientes tareas
void taskMsg200(void* param);      // Tarea #2: enviar mensaje serial cada 200ms en 5s, incluyendo contador y palabra clave del comando
void taskSensor100(void* param);   // Tarea #3: leer sensor cada 100ms, escribir valor por 10s, formato csv, al final promedio y desv. est. variable sensor
void taskBoton(void* param);       // Tarea #4 y 5: si presiona botón, numero random, led titilar con cuenta regresiva
void taskControlLED(void* param);

// Aquí la declaración de semáforos
SemaphoreHandle_t mutexSerial;

void setup() {
  Serial.begin(115200);                   // Iniciación serial
  mutexSerial = xSemaphoreCreateMutex();  // Iniciación semaphore para el serial

  // Creación de las colas
  queueMensajes = xQueueCreate(10, sizeof(int));
  queueSensor = xQueueCreate(10, sizeof(int));

  // Creación de las tareas
  xTaskCreate(taskLeerSerial, "LeerSerial", 2048, NULL, 1, NULL);  // Tarea #1
  xTaskCreate(taskMsg200, "Msg200", 2048, NULL, 1, NULL);          // Tarea #2

  pinMode(PIN_DO, INPUT);
  pinMode(PIN_LED, OUTPUT);
  analogReadResolution(12);
  analogSetPinAttenuation(PIN_AO, ADC_11db);
  xTaskCreate(taskSensor100, "Sensor100", 2048, NULL, 1, NULL);  // Tarea #3

  pinMode(PIN_BOTON, INPUT_PULLUP);
  pinMode(PIN_LED_EXT, OUTPUT);
  queueTitilar = xQueueCreate(5, sizeof(int));  // Tarea #4 y 5
  randomSeed(analogRead(PIN_AO));
  xTaskCreate(taskBoton, "Boton", 4096, NULL, 1, NULL);
  xTaskCreate(taskControlLED, "ControlLED", 4096, NULL, 1, NULL);
}

void loop() {}

// Tarea #1: recibir un comando por serial, ese comando activará las siguientes tareas
void taskLeerSerial(void* param) {
  while (1) {
    if (Serial.available()) {
      String linea = Serial.readStringUntil('\n');
      linea.trim();

      if (linea == "empezar") {
        int comando = comandoEmpezar;
        xQueueSend(queueMensajes, &comando, portMAX_DELAY);
        xQueueSend(queueSensor, &comando, portMAX_DELAY);
      }
    }
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}

// Tarea #2: enviar mensaje serial cada 200ms en 5s, incluyendo contador y palabra clave del comando

void taskMsg200(void* param) {
  int comandoRecibido;

  while (1) {
    if (xQueueReceive(queueMensajes, &comandoRecibido, portMAX_DELAY)) {
      if (comandoRecibido == comandoEmpezar) {
        for (int i = 1; i <= 25; i++) {

          if (xSemaphoreTake(mutexSerial, portMAX_DELAY)) {
            Serial.print("Mensaje ");
            Serial.print(i);
            Serial.print(" | Codigo del comando: ");
            Serial.println(comandoRecibido);
            xSemaphoreGive(mutexSerial);
          }

          vTaskDelay(pdMS_TO_TICKS(200));
        }
      }
    }
  }
}

// Tarea #3: leer sensor cada 100ms, escribir valor por 10s, formato csv, al final promedio y desv. est. variable sensor


void taskSensor100(void* param) {
  int comandoRecibido;

  while (1) {
    // Esperar el comando de inicio
    if (xQueueReceive(queueSensor, &comandoRecibido, portMAX_DELAY) == pdPASS) {

      if (comandoRecibido == comandoEmpezar) {

        const int N = 100;
        const int intervalo_ms = 100;

        int valores[N];
        int tiempo_ms[N];

        TickType_t instanteAnterior = xTaskGetTickCount();

        // 1. Medir el sensor durante 10 segundos
        for (int i = 0; i < N; i++) {

          tiempo_ms[i] = i * intervalo_ms;
          valores[i] = analogRead(PIN_AO);
          Serial.print("Valor sensor: ");
          Serial.println(analogRead(PIN_AO));

          // Controlar el LED según el umbral
          if (valores[i] >= umbral) {
            digitalWrite(PIN_LED, HIGH);
          } else {
            digitalWrite(PIN_LED, LOW);
          }

          // Mantener el intervalo de muestreo de 100 ms
          vTaskDelayUntil(
            &instanteAnterior,
            pdMS_TO_TICKS(intervalo_ms));
        }

        // 2. Calcular el promedio
        float suma = 0.0;

        for (int i = 0; i < N; i++) {
          suma += valores[i];
        }

        float promedio = suma / N;

        // 3. Calcular la desviación estándar poblacional
        float sumaCuadrados = 0.0;

        for (int i = 0; i < N; i++) {
          float diferencia = valores[i] - promedio;
          sumaCuadrados += diferencia * diferencia;
        }

        float desviacionEstandar = sqrtf(sumaCuadrados / N);

        // 4. Imprimir todo el bloque CSV sin interrupciones
        if (xSemaphoreTake(mutexSerial, portMAX_DELAY) == pdTRUE) {

          Serial.println("tipo,tiempo_ms,valor_sensor");

          for (int i = 0; i < N; i++) {
            Serial.print("dato,");
            Serial.print(tiempo_ms[i]);
            Serial.print(",");
            Serial.println(valores[i]);
          }

          Serial.print("promedio,,");
          Serial.println(promedio, 3);

          Serial.print("desviacion_estandar,,");
          Serial.println(desviacionEstandar, 3);

          xSemaphoreGive(mutexSerial);
        }
      }
    }
  }
}

// Tarea #4 y #5:
void taskBoton(void* param) {
  while (true) {

    if (digitalRead(PIN_BOTON) == LOW) {

      int numeroRandom = random(1, 21);

      // Enviar el número a la tarea que controla el LED
      xQueueSend(queueTitilar, &numeroRandom, portMAX_DELAY);

      // Esperar a que se suelte el botón
      while (digitalRead(PIN_BOTON) == LOW) {
        vTaskDelay(pdMS_TO_TICKS(20));
      }
    }

    vTaskDelay(pdMS_TO_TICKS(20));
  }
}

void taskControlLED(void* param) {
  int numeroTitilaciones;

  while (true) {

    // Esperar el número enviado por la tarea del botón
    if (xQueueReceive(queueTitilar,
                      &numeroTitilaciones,
                      portMAX_DELAY)
        == pdPASS) {

      // Mensaje personalizado
      if (xSemaphoreTake(mutexSerial, portMAX_DELAY) == pdTRUE) {
        Serial.println();
        Serial.print("El LED parpadeará ");
        Serial.print(numeroTitilaciones);
        Serial.println(" veces.");
        Serial.println("Iniciando cuenta regresiva:");

        xSemaphoreGive(mutexSerial);
      }

      // Una titilación completa dura 1 segundo
      for (int i = numeroTitilaciones; i >= 1; i--) {

        if (xSemaphoreTake(mutexSerial, portMAX_DELAY) == pdTRUE) {
          Serial.print("Faltan ");
          Serial.print(i);
          Serial.println(" parpadeos.");

          xSemaphoreGive(mutexSerial);
        }

        digitalWrite(PIN_LED_EXT, HIGH);
        vTaskDelay(pdMS_TO_TICKS(500));

        digitalWrite(PIN_LED_EXT, LOW);
        vTaskDelay(pdMS_TO_TICKS(500));
      }

      if (xSemaphoreTake(mutexSerial, portMAX_DELAY) == pdTRUE) {
        Serial.println("Secuencia LED terminada");
        xSemaphoreGive(mutexSerial);
      }
    }
  }
}