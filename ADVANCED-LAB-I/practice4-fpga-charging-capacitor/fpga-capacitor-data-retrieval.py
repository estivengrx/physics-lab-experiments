import time
import numpy as np
import matplotlib.pyplot as plt
import pandas as pd
import os

from pynq import allocate, Overlay

# Cargar bitstream
dma_dato = Overlay("cargaDescarga.bit")

dma = dma_dato.axi_dma
gp = dma_dato.axi_gpio_0

# Parámetros
numero_de_datos = 1024
frecuency = 0x6
tiempo = 3

# Configurar frecuencia
gp.write(0x0, frecuency)

# Esperar el tiempo de adquisición
inicio = time.perf_counter()
while (time.perf_counter() - inicio) < tiempo:
    pass

# Buffer de entrada
input_buffer = allocate(shape=(numero_de_datos,), dtype=np.uint32)
dma.recvchannel.transfer(input_buffer)

# Normalización
datos_norm = input_buffer / 65535

# Crear carpeta figures si no existe
os.makedirs("figures", exist_ok=True)

# Guardar la figura sin mostrarla
plt.figure(figsize=(10, 6))
plt.plot(datos_norm, color='blue', linewidth=1.2)
plt.title("Carga y descarga del condensador", fontsize=14)
plt.xlabel("Número de muestra", fontsize=12)
plt.ylabel("Voltaje normalizado (V/Vmax)", fontsize=12)
plt.grid(True, which='both', linestyle='--', linewidth=0.7)
plt.minorticks_on()

plt.savefig("carga_descarga.png", dpi=300, bbox_inches='tight')
plt.close()  # Cierra la figura para no mostrarla

# Guardar CSV
df = pd.DataFrame(input_buffer, columns=["Valor"])
df["Muestra"] = df.index
df = df[["Muestra", "Valor"]]
df.to_csv("datosASE.csv", index=False)

print("Proceso completado. Figura guardada en 'carga_descarga.png' y datos en 'datosASE.csv'.")