import serial
import time
import matplotlib.pyplot as plt

# Configuración del puerto serial
esp = serial.Serial("/dev/ttyUSB0", 115200, timeout=1)
time.sleep(2)
esp.write(b'a')
distancias = []

# Leer 10 líneas
for i in range(10):
    linea = esp.readline().decode().strip()

    while linea == "":
        linea = esp.readline().decode().strip()

    print("Arduino:", linea)

    # Extraer solo el número
    try:
        valor = float(linea.split(":")[1].replace("cm", "").strip())
        distancias.append(valor)
    except:
        print("error", linea)

esp.close()
tiempo = list(range(1, len(distancias) + 1)) # eje de 1 a 10 segundos
plt.figure(figsize=(8, 5))
plt.plot(tiempo, distancias, marker='o', linestyle='-', color='blue')
plt.ylim(0, 300)
plt.title("Distancia medida por el sensor (10 muestras)")
plt.xlabel("Número de la medida")
plt.ylabel("Distancia (cm)")
plt.grid(True)

plt.show()
