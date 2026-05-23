#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "../../models/model.h"
#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

Adafruit_MPU6050 mpu;

// Define input and output dimensions
const int kInputSize = 3;   // x, y, z from accelerometer
const int kOutputSize = 3;  // 3 gesture classes

// TensorFlow Lite model setup
tflite::MicroInterpreter* interpreter = nullptr;

constexpr int kTensorArenaSize = 80 * 1024;
alignas(16) uint8_t tensor_arena[kTensorArenaSize];

// ✅ Tamaño ajustado al número de ops registradas
static tflite::MicroMutableOpResolver<3> resolver;

// Helper function to get the index of the highest output
int max_index(float* arr, int size) {
  int max_i = 0;
  for (int i = 1; i < size; i++) {
    if (arr[i] > arr[max_i]) {
      max_i = i;
    }
  }
  return max_i;
}

void setup() {
  Serial.begin(115200);

  // ✅ Inicializar MPU6050
  if (!mpu.begin()) {
    Serial.println("MPU6050 no encontrado. Verifica la conexión.");
    while (1) delay(10);
  }
  Serial.println("MPU6050 inicializado correctamente.");

  // ✅ Registrar ops ANTES de crear el intérprete
  resolver.AddFullyConnected();
  resolver.AddRelu();
  resolver.AddSoftmax();

  // Cargar el modelo
  const tflite::Model* model = tflite::GetModel(gesture_model_tflite);
  if (model->version() != TFLITE_SCHEMA_VERSION) {
    Serial.println("Versión del modelo incompatible.");
    while (1) delay(10);
  }

  // Crear el intérprete
  static tflite::MicroInterpreter static_interpreter(
    model, resolver, tensor_arena, kTensorArenaSize);
  interpreter = &static_interpreter;

  // ✅ Verificar AllocateTensors
  TfLiteStatus alloc_status = interpreter->AllocateTensors();
  if (alloc_status != kTfLiteOk) {
    Serial.println("AllocateTensors() falló. Aumenta kTensorArenaSize.");
    while (1) delay(1000);
  }

  Serial.println("Modelo cargado y tensores asignados correctamente.");
  Serial.print("Arena usada (bytes): ");
  Serial.println(interpreter->arena_used_bytes());
}

void loop() {
  // Obtener datos del sensor
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  float sensor_data[kInputSize] = {
    a.acceleration.x,
    a.acceleration.y,
    a.acceleration.z
  };

  // Cargar datos en el tensor de entrada
  float* input = interpreter->input(0)->data.f;
  for (int i = 0; i < kInputSize; i++) {
    input[i] = sensor_data[i];
  }

  // Correr inferencia
  TfLiteStatus invoke_status = interpreter->Invoke();
  if (invoke_status != kTfLiteOk) {
    Serial.println("Invoke() falló.");
    return;
  }

  // Obtener resultado
  float* output = interpreter->output(0)->data.f;
  int predicted_gesture = max_index(output, kOutputSize);

  Serial.print("Gesto detectado: ");
  Serial.println(predicted_gesture);

  delay(1000); // Pequeña pausa entre inferencias
}