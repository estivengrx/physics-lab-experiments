#include <Arduino.h>

// Cantidad de operaciones que realizará cada tarea
const uint32_t OPERACIONES = 30000000;

// Aquí la declaración de las tareas
void taskSingleA(void* param);
void taskSingleB(void* param);
void taskDualA(void* param);
void taskDualB(void* param);
void taskControl(void* param);

// Aquí la declaración de semáforos
SemaphoreHandle_t semSingleA;
SemaphoreHandle_t semSingleB;
SemaphoreHandle_t semDualA;
SemaphoreHandle_t semDualB;

// Aquí la declaración del mutex
SemaphoreHandle_t mutexSerial;

// Aquí la declaración de la cola
QueueHandle_t queueResultados;

// Estructura para enviar resultados
struct Resultado {
  char tarea;
  unsigned long tiempo;
};

// Variables para guardar resultados
volatile uint32_t resultadoA = 0;
volatile uint32_t resultadoB = 0;

void setup() {
  Serial.begin(115200);

  // Creación de los semáforos
  semSingleA = xSemaphoreCreateBinary();
  semSingleB = xSemaphoreCreateBinary();
  semDualA = xSemaphoreCreateBinary();
  semDualB = xSemaphoreCreateBinary();

  // Creación del mutex
  mutexSerial = xSemaphoreCreateMutex();

  // Creación de la cola
  queueResultados = xQueueCreate(4, sizeof(Resultado));

  // ==========================================================
  // Creación de las tareas
  // ==========================================================

  // Tareas Single Core: las dos están fijadas al Core 0
  xTaskCreatePinnedToCore(taskSingleA, "SingleA", 4096, NULL, 1, NULL, 0);
  xTaskCreatePinnedToCore(taskSingleB, "SingleB", 4096, NULL, 1, NULL, 0);

  // Tareas Dual Core: una tarea en Core 0 y otra en Core 1
  xTaskCreatePinnedToCore(taskDualA, "DualA", 4096, NULL, 1, NULL, 0);
  xTaskCreatePinnedToCore(taskDualB, "DualB", 4096, NULL, 1, NULL, 1);

  // Tarea de control
  xTaskCreatePinnedToCore(taskControl, "Control", 4096, NULL, 2, NULL, 0);
}

void loop() {}

// ============================================================
// Tarea de control
// ============================================================

void taskControl(void* param) {
  while (1) {
    if (Serial.available()) {
      String comando = Serial.readStringUntil('\n');
      comando.trim();

      if (comando == "empezar") {
        // ====================================================
        // SINGLE CORE
        // ====================================================

        if (xSemaphoreTake(mutexSerial, portMAX_DELAY)) {
          Serial.println();
          Serial.println("======================================");
          Serial.println("BENCHMARK SINGLE CORE");
          Serial.println("======================================");
          Serial.println("Tarea A -> Core 0");
          Serial.println("Tarea B -> Core 0");
          Serial.println("Iniciando...");
          xSemaphoreGive(mutexSerial);
        }

        unsigned long inicioSingle = micros();

        // Iniciar las dos tareas
        xSemaphoreGive(semSingleA);
        xSemaphoreGive(semSingleB);

        Resultado r1, r2;

        // Esperar a que terminen las dos tareas
        xQueueReceive(queueResultados, &r1, portMAX_DELAY);
        xQueueReceive(queueResultados, &r2, portMAX_DELAY);

        unsigned long finSingle = micros();
        unsigned long tiempoSingle = finSingle - inicioSingle;

        // ====================================================
        // DUAL CORE
        // ====================================================

        if (xSemaphoreTake(mutexSerial, portMAX_DELAY)) {
          Serial.println();
          Serial.println("======================================");
          Serial.println("BENCHMARK DUAL CORE");
          Serial.println("======================================");
          Serial.println("Tarea A -> Core 0");
          Serial.println("Tarea B -> Core 1");
          Serial.println("Iniciando...");
          xSemaphoreGive(mutexSerial);
        }

        unsigned long inicioDual = micros();

        // Iniciar las dos tareas
        xSemaphoreGive(semDualA);
        xSemaphoreGive(semDualB);

        Resultado r3, r4;

        // Esperar a que terminen las dos tareas
        xQueueReceive(queueResultados, &r3, portMAX_DELAY);
        xQueueReceive(queueResultados, &r4, portMAX_DELAY);

        unsigned long finDual = micros();
        unsigned long tiempoDual = finDual - inicioDual;

        // ====================================================
        // RESULTADOS
        // ====================================================

        float mejora = ((float)(tiempoSingle - tiempoDual) / tiempoSingle) * 100.0;

        if (xSemaphoreTake(mutexSerial, portMAX_DELAY)) {
          Serial.println();
          Serial.println("======================================");
          Serial.println("RESULTADOS");
          Serial.println("======================================");
          Serial.print("Tiempo Single Core: ");
          Serial.print(tiempoSingle);
          Serial.println(" us");
          Serial.print("Tiempo Dual Core:   ");
          Serial.print(tiempoDual);
          Serial.println(" us");
          Serial.print("Mejora: ");
          Serial.print(mejora, 2);
          Serial.println(" %");
          Serial.println("======================================");
          xSemaphoreGive(mutexSerial);
        }
      }
    }
    vTaskDelay(pdMS_TO_TICKS(20));
  }
}

// Tarea Single Core A

void taskSingleA(void* param) {
  while (1) {
    // Esperar la señal de inicio
    xSemaphoreTake(semSingleA, portMAX_DELAY);

    unsigned long inicio = micros();
    uint32_t resultado = 0;

    // Trabajo computacional
    for (uint32_t i = 0; i < OPERACIONES; i++) {
      resultado += (i * 1664525UL) + 1013904223UL;
      resultado ^= (i >> 3);
    }

    resultadoA = resultado;
    unsigned long fin = micros();

    Resultado datos;
    datos.tarea = 'A';
    datos.tiempo = fin - inicio;

    // Enviar resultado mediante la cola
    xQueueSend(queueResultados, &datos, portMAX_DELAY);
  }
}

// Tarea Single Core B

void taskSingleB(void* param) {
  while (1) {
    // Esperar la señal de inicio
    xSemaphoreTake(semSingleB, portMAX_DELAY);

    unsigned long inicio = micros();
    uint32_t resultado = 0;

    // Trabajo computacional
    for (uint32_t i = 0; i < OPERACIONES; i++) {
      resultado += (i * 1664525UL) + 1013904223UL;
      resultado ^= (i >> 3);
    }

    resultadoB = resultado;
    unsigned long fin = micros();

    Resultado datos;
    datos.tarea = 'B';
    datos.tiempo = fin - inicio;

    // Enviar resultado mediante la cola
    xQueueSend(queueResultados, &datos, portMAX_DELAY);
  }
}

// Tarea Dual Core A

void taskDualA(void* param) {
  while (1) {
    // Esperar la señal de inicio
    xSemaphoreTake(semDualA, portMAX_DELAY);

    unsigned long inicio = micros();
    uint32_t resultado = 0;

    // Trabajo computacional
    for (uint32_t i = 0; i < OPERACIONES; i++) {
      resultado += (i * 1664525UL) + 1013904223UL;
      resultado ^= (i >> 3);
    }

    resultadoA = resultado;
    unsigned long fin = micros();

    Resultado datos;
    datos.tarea = 'A';
    datos.tiempo = fin - inicio;

    // Enviar resultado mediante la cola
    xQueueSend(queueResultados, &datos, portMAX_DELAY);
  }
}

// Tarea Dual Core B

void taskDualB(void* param) {
  while (1) {
    // Esperar la señal de inicio
    xSemaphoreTake(semDualB, portMAX_DELAY);

    unsigned long inicio = micros();
    uint32_t resultado = 0;

    // Trabajo computacional
    for (uint32_t i = 0; i < OPERACIONES; i++) {
      resultado += (i * 1664525UL) + 1013904223UL;
      resultado ^= (i >> 3);
    }

    resultadoB = resultado;
    unsigned long fin = micros();

    Resultado datos;
    datos.tarea = 'B';
    datos.tiempo = fin - inicio;

    // Enviar resultado mediante la cola
    xQueueSend(queueResultados, &datos, portMAX_DELAY);
  }
}