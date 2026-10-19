const int PIN_AO = 34;
const int PIN_DO = 27;
const int PIN_LED = 2;
const int umbral = 20;
const int PIN_BOTON = 0;
const int PIN_LED_EXT = 26;



void setup() {
  Serial.begin(115200);

  // Tarea para imprimir un mensaje cada segundo
  xTaskCreate(task1s, "1s", 2048, NULL, 1, NULL);

  // Tarea para imprimir un mensaje cada 200 milisegundos
  xTaskCreate(task200ms, "200ms", 2048, NULL, 1, NULL);

  // Tarea para leer un sensor hall cada 100ms y escribir el valor,
  // si el valor es mayor a XXXXXX se enciende el led del arduino, sino se apaga
  pinMode(PIN_DO, INPUT);
  pinMode(PIN_LED, OUTPUT);
  analogReadResolution(12);
  analogSetPinAttenuation(PIN_AO, ADC_11db);

  xTaskCreate(taskSonido, "LedHall", 2048, NULL, 1, NULL);

  // Tarea para presionar boton y generar numero aleatorioó
  pinMode(PIN_BOTON, INPUT_PULLUP);
  pinMode(PIN_LED_EXT, OUTPUT);
  xTaskCreate(taskBoton, "Boton", 2048, NULL, 1, NULL);
}

void loop() {
  // No se necesita hacer nada aquí porque las actividades hacen todo
}

void task200ms(void *param200ms) {
  while(true) {
    Serial.println("Este es el mensaje cada 200 milisegundos");
    vTaskDelay(pdMS_TO_TICKS(200));
    }
}

void task1s(void *param1s) {
  while(true) {
      Serial.println("Este es el mensaje cada segundo");
      vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void taskSonido(void *paramSonido) {
  while(true) {
    int valor = analogRead(PIN_AO);
    Serial.print("El valor del sensor es: ");
    Serial.println(valor);
    if (valor >= umbral) {
      digitalWrite(PIN_LED, HIGH);
    } else {
        digitalWrite(PIN_LED, LOW);        
    }
    
    vTaskDelay(pdMS_TO_TICKS(100));
    }  
}

void taskBoton(void *paramBoton) {
  while(true) {
    bool pulsado = (digitalRead(PIN_BOTON) == LOW);
    if (pulsado) {
      long numeroRandom = random(1, 21);
      Serial.print("----------------------------Numero de veces a titilar: ");
      Serial.println(numeroRandom);
      for (int i = 0; i < numeroRandom; i++) {
        digitalWrite(PIN_LED_EXT, HIGH);
        vTaskDelay(pdMS_TO_TICKS(500));             // encendido 0.5 s
        digitalWrite(PIN_LED_EXT, LOW);
        vTaskDelay(pdMS_TO_TICKS(500));             // apagado 0.5 s
      }
       // Espera a que suelta el botón para no repetir la secuencia
      while (digitalRead(PIN_BOTON) == LOW) {
        vTaskDelay(pdMS_TO_TICKS(20));
      }
    }
    vTaskDelay(pdMS_TO_TICKS(20)); 
    }
}
