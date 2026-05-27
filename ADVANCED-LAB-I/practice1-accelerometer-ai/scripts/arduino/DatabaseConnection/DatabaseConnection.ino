#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "model.h"
#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <WiFi.h>
#include <FirebaseESP32.h>
#include "config.h"

Adafruit_MPU6050 mpu;
FirebaseData fbdo;
FirebaseConfig config;
FirebaseAuth auth;

const int kInputSize = 3;
const int kOutputSize = 3;

tflite::MicroInterpreter* interpreter = nullptr;
constexpr int kTensorArenaSize = 8 * 1024;
alignas(16) uint8_t tensor_arena[kTensorArenaSize];
static tflite::MicroMutableOpResolver<3> resolver;

int max_index(float* arr, int size) {
  int max_i = 0;
  for (int i = 1; i < size; i++) {
    if (arr[i] > arr[max_i]) max_i = i;
  }
  return max_i;
}

void setup() {
  Serial.begin(115200);

  // WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Conectando WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("\nWiFi OK");

  // Firebase con legacy token (lo más simple)
  config.host = DB_HOST;
  config.signer.tokens.legacy_token = DB_SECRET;
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
  Serial.println("Firebase OK");

  // MPU6050
  if (!mpu.begin()) {
    Serial.println("MPU6050 no encontrado");
    while (1) delay(10);
  }
  Serial.println("MPU6050 OK");

  // TFLite
  resolver.AddFullyConnected();
  resolver.AddRelu();
  resolver.AddSoftmax();

  const tflite::Model* model = tflite::GetModel(gesture_model_tflite);
  if (model->version() != TFLITE_SCHEMA_VERSION) {
    Serial.println("Modelo incompatible");
    while (1) delay(10);
  }

  static tflite::MicroInterpreter static_interpreter(
    model, resolver, tensor_arena, kTensorArenaSize);
  interpreter = &static_interpreter;

  if (interpreter->AllocateTensors() != kTfLiteOk) {
    Serial.println("Error AllocateTensors");
    while (1) delay(1000);
  }
  Serial.println("Modelo OK");
}

void loop() {
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  float* input = interpreter->input(0)->data.f;
  input[0] = a.acceleration.x / 10.0;
  input[1] = a.acceleration.y / 10.0;
  input[2] = a.acceleration.z / 10.0;

  if (interpreter->Invoke() != kTfLiteOk) {
    Serial.println("Error inferencia");
    return;
  }

  float* output = interpreter->output(0)->data.f;
  int gesto = max_index(output, kOutputSize);

  Serial.print("Gesto: ");
  Serial.println(gesto);

  if (Firebase.pushInt(fbdo, "/movimientos", gesto)) {
    Serial.println("Firebase OK");
  } else {
    Serial.println(fbdo.errorReason());
  }

  delay(1000);
}
