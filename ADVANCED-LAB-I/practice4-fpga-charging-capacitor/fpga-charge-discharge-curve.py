# Código creado con la ayuda de Claude LLM [https://claude.ai]

import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
import os

from pathlib import Path

BASE_DIR = Path(__file__).resolve().parents[0]

INPUT_CSV  = BASE_DIR / "data" / "datosASE.csv"
OUTPUT_CSV = BASE_DIR / "data" / "datos_procesados.csv"
OUTPUT_FIG         = BASE_DIR / "plots" / "carga_descarga.png"
OUTPUT_FIG_ANOTADO = BASE_DIR / "plots" / "carga_descarga_anotado.png"

NUM_DATOS = 1024
V_MAX     = 65535   # fondo de escala del ADC de 16 bits: 2^16 - 1

# Umbrales para la derivada discreta dV[n] = V[n] - V[n-1].
# Un flanco de subida real supera fácilmente 0.002 V/muestra;
# el ruido de fondo se mantiene por debajo de ±0.001, así que
# hay margen suficiente para no generar falsas detecciones.
# MIN_DIST evita contar dos veces el mismo flanco cuando la
# señal tarda varias muestras en cruzar el umbral.
UMBRAL_SUBIDA = 0.002
UMBRAL_BAJADA = -0.002
MIN_DIST      = 30

os.makedirs("plots", exist_ok=True)

df = pd.read_csv(INPUT_CSV)
df = df.head(NUM_DATOS)

# Normalización: lleva los cuentas ADC al intervalo [0, 1]
# para poder comparar directamente con V_C(t)/V_0 del modelo teórico
df["Voltaje"] = df["Valor"] / V_MAX
df.to_csv(OUTPUT_CSV, index=False)

v  = df["Voltaje"].to_numpy()

# prepend=v[0] replica el primer valor para que dv tenga la misma
# longitud que v; de lo contrario np.diff devuelve N-1 elementos
dv = np.diff(v, prepend=v[0])


def detectar_flancos(deriv, umbral, min_dist):
    """
    Devuelve los índices donde la derivada discreta supera el umbral,
    garantizando una separación mínima de min_dist muestras entre eventos.

    El criterio de separación mínima es necesario porque un flanco real
    no es instantáneo: la señal puede permanecer varios ciclos por encima
    del umbral durante la misma transición, generando múltiples candidatos
    que corresponden al mismo evento físico.
    """
    if umbral > 0:
        candidatos = np.where(deriv > umbral)[0]
    else:
        candidatos = np.where(deriv < umbral)[0]

    if len(candidatos) == 0:
        return []

    eventos = [int(candidatos[0])]
    for idx in candidatos[1:]:
        if idx - eventos[-1] >= min_dist:
            eventos.append(int(idx))
    return eventos


inicios_carga    = detectar_flancos(dv, UMBRAL_SUBIDA, MIN_DIST)
inicios_descarga = detectar_flancos(dv, UMBRAL_BAJADA, MIN_DIST)

print("EVENTOS DETECTADOS")
print(f"\n  Inicio de carga   (dV/dn > {UMBRAL_SUBIDA}):")
for i, idx in enumerate(inicios_carga, 1):
    print(f"    Evento {i}: muestra {idx:4d}  |  V = {v[idx]:.5f} V/Vmax")

print(f"\n  Inicio de descarga (dV/dn < {UMBRAL_BAJADA}):")
for i, idx in enumerate(inicios_descarga, 1):
    print(f"    Evento {i}: muestra {idx:4d}  |  V = {v[idx]:.5f} V/Vmax")


# Figura original
plt.figure(figsize=(10, 6))
plt.plot(df["Voltaje"], color='blue', linewidth=1.2)
plt.title("Curva de carga y descarga del condensador (datos importados)", fontsize=18)
plt.xlabel("Número de muestra", fontsize=15)
plt.ylabel("Voltaje normalizado (V/Vmax)", fontsize=15)
plt.grid(True, which='both', linestyle='--', linewidth=0.7)
plt.minorticks_on()
plt.savefig(OUTPUT_FIG, dpi=300, bbox_inches='tight')
plt.close()


# Figura anotada
# Se marcan los flancos detectados con líneas verticales para verificar
# visualmente que el algoritmo identifica las transiciones correctas.
# Los eventos intermedios (carga #2, descarga #2) son detecciones válidas
# del umbral dentro del mismo flanco; no representan ciclos independientes.
fig, ax1 = plt.subplots(figsize=(12, 6))

ax1.plot(v, color='blue', linewidth=1.2, label="V normalizado")

for i, idx in enumerate(inicios_carga, 1):
    ax1.axvline(x=idx, color='green', linestyle='--', linewidth=1.4,
                label=f"Inicio carga #{i}  (m={idx})")
    ax1.annotate(f" ↑ carga #{i}\n m={idx}",
                 xy=(idx, v[idx]), xytext=(idx + 8, v[idx] + 0.015),
                 fontsize=11, color='green',
                 arrowprops=dict(arrowstyle='->', color='green', lw=0.8))

for i, idx in enumerate(inicios_descarga, 1):
    ax1.axvline(x=idx, color='red', linestyle='--', linewidth=1.4,
                label=f"Inicio descarga #{i}  (m={idx})")
    ax1.annotate(f" ↓ desc. #{i}\n m={idx}",
                 xy=(idx, v[idx]), xytext=(idx + 8, v[idx] - 0.025),
                 fontsize=11, color='red',
                 arrowprops=dict(arrowstyle='->', color='red', lw=0.8))

ax1.set_xlabel("Número de muestra", fontsize=15)
ax1.set_ylabel("Voltaje normalizado (V/Vmax)", fontsize=15)
ax1.set_title("Detección automática de transiciones de carga/descarga", fontsize=18)
ax1.legend(fontsize=12, loc='upper right')
ax1.grid(True, linestyle='--', linewidth=0.6)

plt.tight_layout()
plt.savefig(OUTPUT_FIG_ANOTADO, dpi=300, bbox_inches='tight')
plt.close()

print(f"\nFigura original guardada en:  {OUTPUT_FIG}")
print(f"Figura anotada guardada en:   {OUTPUT_FIG_ANOTADO}")


# Estimación de tau
# Por definición, tau es el tiempo en que V_C alcanza (1 - 1/e) * V0 ≈ 63.2% V0.
# Se busca la primera muestra dentro del primer semiciclo de carga (256–513)
# donde V supera ese nivel. El resultado es un estimador por defecto: el valor
# real de tau cae entre esta muestra y la anterior.
V0    = v.max()
V_tau = V0 * (1 - np.exp(-1))   # ≈ 0.632 * V0

inicio_carga = 256
fin_carga    = 513
v_ciclo1 = v[inicio_carga:fin_carga]

# argmax sobre un array booleano devuelve el índice del primer True;
# si ningún elemento cumple la condición devuelve 0, caso que no
# ocurre aquí porque el condensador sí alcanza V_tau dentro del tramo
idx_tau_local  = np.argmax(v_ciclo1 >= V_tau)
idx_tau_global = inicio_carga + idx_tau_local

tau_muestras = idx_tau_global - inicio_carga
tau_segundos = tau_muestras / 1000.0   # f_s = 1000 Hz, registro 0x6 de la FPGA

print("ESTIMACIÓN DE TAU")
print(f"  V0                       = {V0:.5f} V/Vmax")
print(f"  V(tau) = 0.632 * V0      = {V_tau:.5f} V/Vmax")
print(f"  Muestra donde V >= V(tau): {idx_tau_global}")
print(f"  tau (muestras)           = {tau_muestras}")
print(f"  tau (segundos)           = {tau_segundos:.4f} s")
print(f"  tau_teo                  = 0.1000 s")
print(f"  Error relativo           = {abs(tau_segundos - 0.1) / 0.1 * 100:.1f} %")