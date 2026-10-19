// ---------- Pines ----------
const int PIN_BOTON   = 0;
const int PIN_LED_EXT = 26;
const int PIN_AO      = 34;

// ---------- Tipos ----------
typedef struct {
  int      valor;
  uint32_t tiempoMs;
} Lectura_t;

// ---------- Handles globales ----------
QueueHandle_t     xQueueParpadeos = NULL;
QueueHandle_t     xQueueLecturas  = NULL;
SemaphoreHandle_t xSemBoton       = NULL;

// Lecturas descartadas por cola llena (la escribe vTaskSensor, la lee vTaskLogger)
volatile uint32_t ulLecturasPerdidas = 0;

// ---------- ISR ----------
void IRAM_ATTR isrBoton() {
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;

  xSemaphoreGiveFromISR(xSemBoton, &xHigherPriorityTaskWoken);

  if (xHigherPriorityTaskWoken == pdTRUE) {
    portYIELD_FROM_ISR();
  }
}

// ---------- Tareas ----------
void vTaskBoton(void *pvParameters) {
  while (true) {
    // Duerme hasta que la ISR avise de una pulsación
    if (xSemaphoreTake(xSemBoton, portMAX_DELAY) == pdTRUE) {

      vTaskDelay(pdMS_TO_TICKS(50));               // antirrebote

      if (digitalRead(PIN_BOTON) == LOW) {         // pulsación real
        int numParpadeos = random(1, 21);          // 1 a 20

        if (xQueueSend(xQueueParpadeos, &numParpadeos, pdMS_TO_TICKS(100)) != pdTRUE) {
          // Cola llena: se ignora esta pulsación
        }

        // Espera a que suelte el botón
        while (digitalRead(PIN_BOTON) == LOW) {
          vTaskDelay(pdMS_TO_TICKS(20));
        }
        vTaskDelay(pdMS_TO_TICKS(50));             // rebote al soltar
      }

      // Limpia avisos generados por rebotes durante el antirrebote
      xSemaphoreTake(xSemBoton, 0);
    }
  }
}

void vTaskLed(void *pvParameters) {
  int numParpadeos;

  while (true) {
    if (xQueueReceive(xQueueParpadeos, &numParpadeos, portMAX_DELAY) == pdTRUE) {
      for (int i = 0; i < numParpadeos; i++) {
        digitalWrite(PIN_LED_EXT, HIGH);
        vTaskDelay(pdMS_TO_TICKS(500));
        digitalWrite(PIN_LED_EXT, LOW);
        vTaskDelay(pdMS_TO_TICKS(500));
      }
    }
  }
}

void vTaskSensor(void *pvParameters) {
  Lectura_t lectura;

  while (true) {
    lectura.valor    = analogRead(PIN_AO);
    lectura.tiempoMs = millis();

    // Timeout 0: si la cola está llena, se descarta y se cuenta
    if (xQueueSend(xQueueLecturas, &lectura, 0) != pdTRUE) {
      ulLecturasPerdidas++;
    }

    vTaskDelay(pdMS_TO_TICKS(100));
  }
}

void vTaskLogger(void *pvParameters) {
  Lectura_t lectura;

  while (true) {
    if (xQueueReceive(xQueueLecturas, &lectura, portMAX_DELAY) == pdTRUE) {
      Serial.print("t=");
      Serial.print(lectura.tiempoMs);
      Serial.print(" ms  valor=");
      Serial.print(lectura.valor);
      Serial.print("  perdidas=");
      Serial.println(ulLecturasPerdidas);
    }
  }
}

// ---------- Utilidad para errores de arranque ----------
void vErrorFatal(const char *mensaje) {
  Serial.println(mensaje);
  while (true) {
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

// ---------- setup / loop ----------
void setup() {
  Serial.begin(115200);

  pinMode(PIN_BOTON, INPUT_PULLUP);
  pinMode(PIN_LED_EXT, OUTPUT);
  digitalWrite(PIN_LED_EXT, LOW);

  analogReadResolution(12);
  analogSetPinAttenuation(PIN_AO, ADC_11db);

  randomSeed(esp_random());

  // 1) Colas y semáforo ANTES que las tareas
  xQueueParpadeos = xQueueCreate(5, sizeof(int));
  xQueueLecturas  = xQueueCreate(10, sizeof(Lectura_t));
  xSemBoton       = xSemaphoreCreateBinary();

  if (xQueueParpadeos == NULL || xQueueLecturas == NULL || xSemBoton == NULL) {
    vErrorFatal("Error: no se pudo crear una cola o el semaforo");
  }

  // 2) Tareas
  if (xTaskCreate(vTaskBoton,   "Boton",   4096, NULL, 1, NULL) != pdPASS ||
      xTaskCreate(vTaskLed,     "Led",     4096, NULL, 1, NULL) != pdPASS ||
      xTaskCreate(vTaskSensor,  "Sensor",  4096, NULL, 1, NULL) != pdPASS ||
      xTaskCreate(vTaskLogger,  "Logger",  4096, NULL, 1, NULL) != pdPASS) {
    vErrorFatal("Error: no se pudo crear una tarea");
  }

  // 3) La interrupción al final, cuando todo lo que usa ya existe
  attachInterrupt(digitalPinToInterrupt(PIN_BOTON), isrBoton, FALLING);
}

void loop() {
  vTaskDelete(NULL);   // loop() no hace nada: se elimina su tarea
}