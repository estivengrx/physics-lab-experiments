import serial
import csv
import time
import sys
import threading
import matplotlib.pyplot as plt

from datetime import datetime
from pathlib import Path
BASE_DIR = Path(__file__).resolve().parents[0]  # original project folder

PUERTO      = "/dev/ttyUSB0"
BAUD        = 115200
DISTANCIA_M = 0.29 # distancia real entre sensor y pared (metros)
ARCHIVO_CSV = f"{BASE_DIR}/sonido_{datetime.now().strftime('%Y%m%d_%H%M%S')}.csv"

# Almacenamiento compartido entre hilos
datos_brutos = [] # lista de tuplas (duracion_us, temp_c, humedad)
lock = threading.Lock()
corriendo = threading.Event()

def leer_serial(ser):
    """Hilo que lee lineas del ESP32 mientras corriendo este activo."""
    while corriendo.is_set():
        try:
            linea = ser.readline().decode(errors="ignore").strip()
        except Exception:
            break
        if not linea or linea.count(",") != 2:
            continue
        try:
            dur_us, temp_c, hum = map(float, linea.split(","))
        except ValueError:
            continue
        if dur_us <= 0:
            continue

        v = (2 * DISTANCIA_M) / (dur_us * 1e-6)  # m/s

        with lock:
            datos_brutos.append((dur_us, temp_c, hum, v))

        print(f"  t={dur_us:7.0f} µs | T={temp_c:5.1f}°C | H={hum:5.1f}% | v={v:7.2f} m/s")

def guardar_csv():
    with open(ARCHIVO_CSV, "w", newline="") as f:
        w = csv.writer(f)
        w.writerow(["duracion_us", "temperatura_C", "humedad_pct", "velocidad_m_s"])
        with lock:
            w.writerows(datos_brutos)
    print(f"\nDatos guardados en '{ARCHIVO_CSV}'  ({len(datos_brutos)} filas)")

def graficar():
    with lock:  
        if len(datos_brutos) < 2:
            print("Muy pocos datos para graficar.")
            return
        temps = [d[1] for d in datos_brutos]
        vels  = [d[3] for d in datos_brutos]

    import numpy as np
    temps = np.array(temps)
    vels  = np.array(vels)

    # Ajuste lineal
    a, b = np.polyfit(temps, vels, 1)

    # Modelo teorico
    # Según el libro "Fundamentals of Acoustics" de Kinsler et al., ecuación 5.6.6, página 121, edición 4:
    T_teo = np.linspace(temps.min() - 2, temps.max() + 2, 200)
    v_teo = 331.3 * np.sqrt(1 + T_teo / 273.15)

    plt.figure(figsize=(8, 5))
    plt.scatter(temps, vels, color="tab:blue", alpha=0.6, zorder=3, label="Datos experimentales")
    plt.plot(temps, a * temps + b, "r--",
             label=f"Ajuste lineal: v = {a:.3f}T + {b:.2f}")
    plt.plot(T_teo, v_teo, "g-",
             label="Modelo teórico: v = 331.3·√(1+T/273.15)")
    plt.xlabel("Temperatura del aire (°C)")
    plt.ylabel("Velocidad del sonido (m/s)")
    plt.title("Velocidad del sonido vs Temperatura")
    plt.legend()
    plt.grid(True)
    plt.tight_layout()
    nombre_fig = ARCHIVO_CSV.replace(".csv", ".png")
    plt.savefig(nombre_fig, dpi=150)
    print(f"Gráfica guardada en '{nombre_fig}'")
    print(f"\nResultados del ajuste:")
    print(f"  Pendiente : {a:.4f} m/s/°C  (teórico ≈ 0.606 m/s/°C cerca de 20°C)")
    print(f"  v a 0°C   : {b:.2f} m/s     (teórico ≈ 331.3 m/s)")
    plt.show()

print("Medicion de velocidad del sonido vs temperatura:")
print(f"  Puerto     : {PUERTO}")
print(f"  Distancia  : {DISTANCIA_M} m")
print(f"  Archivo CSV: {ARCHIVO_CSV}")
print()

try:
    ser = serial.Serial(PUERTO, BAUD, timeout=1)
except serial.SerialException as e:
    print(f"ERROR abriendo puerto: {e}")
    sys.exit(1)

time.sleep(2) # esperar reset del ESP32
ser.reset_input_buffer()

input("  Presiona ENTER para iniciar la medicion... ")
ser.write(b'a') # señal de inicio al ESP32
corriendo.set()

hilo = threading.Thread(target=leer_serial, args=(ser,), daemon=True)
hilo.start()

print("  Midiendo... Ctrl+C para detener.\n")

try:
    while True:
        time.sleep(0.1)
except KeyboardInterrupt:
    pass

# Detener
corriendo.clear()
ser.write(b's') # señal de parada al ESP32
time.sleep(0.3)
ser.close()

guardar_csv()
graficar()