"""
Análisis promediado de velocidad del sonido vs temperatura
============================================================
Laboratorio Avanzado I - Universidad de Antioquia

Combina 4-5 repeticiones del mismo experimento (cilindro tapado en una cara,
calentado con secador, sensor ultrasónico ESP32) para:

  1. Agrupar todos los puntos experimentales por bins de temperatura y
     calcular el promedio y la incertidumbre (error estándar de la media,
     SEM) de la velocidad medida en cada bin.
  2. Hacer un ajuste lineal v = a*T + b usando TODOS los puntos crudos
     combinados (pooled), reportando la incertidumbre de "a" y "b"
     mediante la matriz de covarianza del ajuste (equivalente a
     scipy.stats.linregress con errores estándar).
  3. Comparar ese ajuste combinado con el modelo teórico de Kinsler et al.
     (v = 331.3 * sqrt(1 + T/273.15)), tanto en pendiente como en el
     valor de v a T=0°C.
  4. Generar residuos (experimental - teórico) para ver si el error crece
     o es sistemático con la temperatura.
  5. Producir una tabla comparativa por archivo (pendiente, intercepto,
     R², n de puntos) para ver la reproducibilidad entre repeticiones.

Uso:
    python3 analisis_sonido.py archivo1.csv archivo2.csv ... [--bin-width 1.0]

Salidas (en la carpeta de salida indicada):
    - resumen_ajustes.csv      tabla con ajuste por archivo + combinado
    - datos_binned.csv         promedios y SEM por bin de temperatura
    - fig1_todos_los_datos.png       dispersión combinada + ajuste + teórico
    - fig2_promedios_binned.png      promedios con barras de error + teórico
    - fig3_residuos.png              (v_exp - v_teo) vs T
"""

import argparse
import csv
import sys
from pathlib import Path

import numpy as np
import matplotlib.pyplot as plt

# ---------------------------------------------------------------------
# Constantes del modelo teórico (Kinsler et al., Fundamentals of
# Acoustics, 4a ed., ecuación 5.6.6, p. 121)
# ---------------------------------------------------------------------
V0_TEORICO = 331.3      # m/s a 0 °C
T0_KELVIN = 273.15      # K


def modelo_teorico(T_celsius):
    """v(T) = 331.3 * sqrt(1 + T/273.15)  [m/s], T en °C"""
    T_celsius = np.asarray(T_celsius, dtype=float)
    return V0_TEORICO * np.sqrt(1.0 + T_celsius / T0_KELVIN)


def pendiente_teorica_local(T_celsius):
    """dv/dT del modelo teórico evaluada en T (para comparar con el ajuste lineal local)."""
    T_celsius = np.asarray(T_celsius, dtype=float)
    return V0_TEORICO / (2.0 * T0_KELVIN) / np.sqrt(1.0 + T_celsius / T0_KELVIN)


# ---------------------------------------------------------------------
# Lectura de datos
# ---------------------------------------------------------------------
def leer_csv(path):
    """Lee un CSV del experimento y devuelve arrays (T, v)."""
    temps, vels = [], []
    with open(path, newline="") as f:
        reader = csv.DictReader(f)
        for row in reader:
            try:
                T = float(row["temperatura_C"])
                v = float(row["velocidad_m_s"])
            except (KeyError, ValueError):
                continue
            temps.append(T)
            vels.append(v)
    return np.array(temps), np.array(vels)


def cargar_todos(paths):
    """Devuelve dict {nombre_archivo: (T, v)} y arrays combinados con
    una etiqueta de origen (run_id) para poder colorear por repetición."""
    datasets = {}
    T_all, v_all, run_all = [], [], []
    for i, p in enumerate(paths):
        T, v = leer_csv(p)
        if len(T) == 0:
            print(f"  [aviso] {p} no tiene filas válidas, se omite.")
            continue
        datasets[Path(p).name] = (T, v)
        T_all.append(T)
        v_all.append(v)
        run_all.append(np.full(len(T), i))
    T_all = np.concatenate(T_all)
    v_all = np.concatenate(v_all)
    run_all = np.concatenate(run_all)
    return datasets, T_all, v_all, run_all


# ---------------------------------------------------------------------
# Ajuste lineal con incertidumbre (equivalente a scipy.stats.linregress,
# implementado a mano para no depender de scipy)
# ---------------------------------------------------------------------
def ajuste_lineal_con_error(T, v):
    """
    Ajusta v = a*T + b por mínimos cuadrados y devuelve:
      a, b, sigma_a, sigma_b, r2, n
    Las incertidumbres se calculan con las fórmulas estándar de regresión
    lineal simple (ver p.ej. Taylor, "An Introduction to Error Analysis").
    """
    n = len(T)
    if n < 3:
        return dict(a=np.nan, b=np.nan, sa=np.nan, sb=np.nan, r2=np.nan, n=n)

    Tm, vm = T.mean(), v.mean()
    Stt = np.sum((T - Tm) ** 2)
    a = np.sum((T - Tm) * (v - vm)) / Stt
    b = vm - a * Tm

    v_pred = a * T + b
    residuos = v - v_pred
    # varianza residual (grados de libertad = n-2)
    s2 = np.sum(residuos ** 2) / (n - 2)
    sa = np.sqrt(s2 / Stt)
    sb = np.sqrt(s2 * (1.0 / n + Tm ** 2 / Stt))

    ss_res = np.sum(residuos ** 2)
    ss_tot = np.sum((v - vm) ** 2)
    r2 = 1 - ss_res / ss_tot if ss_tot > 0 else np.nan

    return dict(a=a, b=b, sa=sa, sb=sb, r2=r2, n=n)


# ---------------------------------------------------------------------
# Binning por temperatura
# ---------------------------------------------------------------------
def promediar_por_bins(T, v, ancho_bin=1.0):
    """
    Agrupa los puntos en bins de temperatura de ancho `ancho_bin` (°C) y
    calcula, para cada bin con al menos 2 puntos:
      T_centro, v_media, v_sem (error estándar de la media), n_puntos
    """
    borde_inf = np.floor(T.min())
    borde_sup = np.ceil(T.max())
    bordes = np.arange(borde_inf, borde_sup + ancho_bin, ancho_bin)

    idx_bin = np.digitize(T, bordes) - 1

    centros, medias, sems, ns = [], [], [], []
    for b in range(len(bordes) - 1):
        mask = idx_bin == b
        n_b = mask.sum()
        if n_b < 2:
            continue
        v_bin = v[mask]
        centros.append((bordes[b] + bordes[b + 1]) / 2.0)
        medias.append(v_bin.mean())
        sems.append(v_bin.std(ddof=1) / np.sqrt(n_b))
        ns.append(n_b)

    return (np.array(centros), np.array(medias), np.array(sems), np.array(ns))


# ---------------------------------------------------------------------
# Gráficas
# ---------------------------------------------------------------------
def fig_todos_los_datos(datasets, T_all, v_all, ajuste_combinado, outpath):
    plt.figure(figsize=(9, 6))
    cmap = plt.get_cmap("tab10")
    for i, (nombre, (T, v)) in enumerate(datasets.items()):
        etiqueta = nombre.replace("sonido_", "").replace(".csv", "")
        plt.scatter(T, v, s=22, alpha=0.55, color=cmap(i % 10),
                    label=f"Repetición {etiqueta}")

    T_line = np.linspace(T_all.min() - 1, T_all.max() + 1, 200)
    a, b = ajuste_combinado["a"], ajuste_combinado["b"]
    plt.plot(T_line, a * T_line + b, "r--", lw=2,
              label=f"Ajuste combinado: v = ({a:.3f}±{ajuste_combinado['sa']:.3f})T "
                    f"+ ({b:.1f}±{ajuste_combinado['sb']:.1f})")
    plt.plot(T_line, modelo_teorico(T_line), "g-", lw=2,
              label="Modelo teórico (Kinsler): v = 331.3·√(1+T/273.15)")

    plt.xlabel("Temperatura del aire (°C)")
    plt.ylabel("Velocidad del sonido (m/s)")
    plt.title("Velocidad del sonido vs Temperatura — todas las repeticiones combinadas")
    plt.legend(fontsize=8, loc="best")
    plt.grid(True, alpha=0.4)
    plt.tight_layout()
    plt.savefig(outpath, dpi=150)
    plt.close()


def fig_promedios_binned(T_bin, v_bin, sem_bin, n_bin, ajuste_binned, T_all, outpath):
    plt.figure(figsize=(9, 6))
    plt.errorbar(T_bin, v_bin, yerr=sem_bin, fmt="o", color="tab:blue",
                 ecolor="tab:blue", elinewidth=1.3, capsize=3, ms=6,
                 label="Promedio por bin de 1°C (± error estándar de la media)")

    T_line = np.linspace(T_all.min() - 1, T_all.max() + 1, 200)
    a, b = ajuste_binned["a"], ajuste_binned["b"]
    plt.plot(T_line, a * T_line + b, "r--", lw=2,
              label=f"Ajuste a los promedios: v = ({a:.3f}±{ajuste_binned['sa']:.3f})T "
                    f"+ ({b:.1f}±{ajuste_binned['sb']:.1f})")
    plt.plot(T_line, modelo_teorico(T_line), "g-", lw=2,
              label="Modelo teórico (Kinsler): v = 331.3·√(1+T/273.15)")

    plt.xlabel("Temperatura del aire (°C)")
    plt.ylabel("Velocidad del sonido promedio (m/s)")
    plt.title("Promedio de velocidad del sonido por bin de temperatura (4-5 repeticiones)")
    plt.legend(fontsize=8, loc="best")
    plt.grid(True, alpha=0.4)
    plt.tight_layout()
    plt.savefig(outpath, dpi=150)
    plt.close()


def fig_residuos(T_bin, v_bin, sem_bin, outpath):
    residuos = v_bin - modelo_teorico(T_bin)
    plt.figure(figsize=(9, 4.5))
    plt.axhline(0, color="green", lw=2, label="Modelo teórico (referencia)")
    plt.errorbar(T_bin, residuos, yerr=sem_bin, fmt="o", color="tab:orange",
                 ecolor="tab:orange", elinewidth=1.3, capsize=3, ms=6,
                 label="Residuo: v_experimental − v_teórico")
    plt.xlabel("Temperatura del aire (°C)")
    plt.ylabel("Diferencia (m/s)")
    plt.title("Residuos respecto al modelo teórico (por bin de temperatura)")
    plt.legend(fontsize=9, loc="best")
    plt.grid(True, alpha=0.4)
    plt.tight_layout()
    plt.savefig(outpath, dpi=150)
    plt.close()


# ---------------------------------------------------------------------
# Tablas de salida
# ---------------------------------------------------------------------
def _fmt2(x):
    """Formatea un valor numérico con dos cifras decimales (deja pasar strings/'-')."""
    try:
        return f"{float(x):.2f}"
    except (TypeError, ValueError):
        return x


def guardar_resumen_ajustes(datasets, ajuste_combinado, ajuste_binned, outpath):
    filas = []
    for nombre, (T, v) in datasets.items():
        r = ajuste_lineal_con_error(T, v)
        filas.append((nombre, r["n"], r["a"], r["sa"], r["b"], r["sb"], r["r2"]))

    with open(outpath, "w", newline="") as f:
        w = csv.writer(f)
        w.writerow(["dataset", "n_puntos", "pendiente_a", "sigma_a",
                    "intercepto_b", "sigma_b", "R2"])
        for nombre, n, a, sa, b, sb, r2 in filas:
            w.writerow([nombre, n, _fmt2(a), _fmt2(sa), _fmt2(b), _fmt2(sb), _fmt2(r2)])
        w.writerow([])
        w.writerow(["combinado_todos_los_puntos", ajuste_combinado["n"],
                    _fmt2(ajuste_combinado["a"]), _fmt2(ajuste_combinado["sa"]),
                    _fmt2(ajuste_combinado["b"]), _fmt2(ajuste_combinado["sb"]),
                    _fmt2(ajuste_combinado["r2"])])
        w.writerow(["ajuste_a_promedios_binned", ajuste_binned["n"],
                    _fmt2(ajuste_binned["a"]), _fmt2(ajuste_binned["sa"]),
                    _fmt2(ajuste_binned["b"]), _fmt2(ajuste_binned["sb"]),
                    _fmt2(ajuste_binned["r2"])])
        w.writerow([])
        w.writerow(["modelo_teorico_Kinsler", "-", "0.61 (aprox. cerca de 20°C, no lineal)",
                    "-", _fmt2(V0_TEORICO), "-", "-"])
    return filas


def guardar_binned_csv(T_bin, v_bin, sem_bin, n_bin, outpath):
    with open(outpath, "w", newline="") as f:
        w = csv.writer(f)
        w.writerow(["T_centro_bin_C", "v_media_m_s", "v_sem_m_s", "n_puntos",
                    "v_teorico_m_s", "residuo_m_s"])
        v_teo = modelo_teorico(T_bin)
        for Tc, vm, sm, n, vt in zip(T_bin, v_bin, sem_bin, n_bin, v_teo):
            w.writerow([f"{Tc:.2f}", f"{vm:.3f}", f"{sm:.3f}", n,
                        f"{vt:.3f}", f"{vm - vt:.3f}"])


# ---------------------------------------------------------------------
# Programa principal
# ---------------------------------------------------------------------
def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("archivos", nargs="+", help="Rutas a los archivos CSV")
    parser.add_argument("--bin-width", type=float, default=1.0,
                         help="Ancho del bin de temperatura en °C (default 1.0)")
    parser.add_argument("--outdir", type=str, default=".",
                         help="Carpeta de salida para figuras y tablas")
    args = parser.parse_args()

    outdir = Path(args.outdir)
    outdir.mkdir(parents=True, exist_ok=True)

    print(f"Leyendo {len(args.archivos)} archivos...")
    datasets, T_all, v_all, run_all = cargar_todos(args.archivos)
    print(f"Total de puntos combinados: {len(T_all)}")

    # Ajuste combinado (todos los puntos crudos juntos)
    ajuste_combinado = ajuste_lineal_con_error(T_all, v_all)

    # Binning y ajuste sobre los promedios
    T_bin, v_bin, sem_bin, n_bin = promediar_por_bins(T_all, v_all, args.bin_width)
    ajuste_binned = ajuste_lineal_con_error(T_bin, v_bin)

    # Figuras
    fig_todos_los_datos(datasets, T_all, v_all, ajuste_combinado,
                         outdir / "fig1_todos_los_datos.png")
    fig_promedios_binned(T_bin, v_bin, sem_bin, n_bin, ajuste_binned, T_all,
                          outdir / "fig2_promedios_binned.png")
    fig_residuos(T_bin, v_bin, sem_bin, outdir / "fig3_residuos.png")

    # Tablas
    filas_resumen = guardar_resumen_ajustes(datasets, ajuste_combinado, ajuste_binned,
                                             outdir / "resumen_ajustes.csv")
    guardar_binned_csv(T_bin, v_bin, sem_bin, n_bin, outdir / "datos_binned.csv")

    # Reporte en consola
    print("\n=== Ajuste por archivo (repetición) ===")
    for nombre, n, a, sa, b, sb, r2 in filas_resumen:
        print(f"  {nombre:30s}  n={n:4d}  a=({a:.3f}±{sa:.3f}) m/s/°C  "
              f"b=({b:.1f}±{sb:.1f}) m/s  R2={r2:.3f}")

    print("\n=== Ajuste combinado (todos los puntos crudos) ===")
    print(f"  a = ({ajuste_combinado['a']:.4f} ± {ajuste_combinado['sa']:.4f}) m/s/°C")
    print(f"  b = ({ajuste_combinado['b']:.2f} ± {ajuste_combinado['sb']:.2f}) m/s")
    print(f"  R2 = {ajuste_combinado['r2']:.4f}   n = {ajuste_combinado['n']}")

    print("\n=== Ajuste sobre promedios por bin ===")
    print(f"  a = ({ajuste_binned['a']:.4f} ± {ajuste_binned['sa']:.4f}) m/s/°C")
    print(f"  b = ({ajuste_binned['b']:.2f} ± {ajuste_binned['sb']:.2f}) m/s")
    print(f"  R2 = {ajuste_binned['r2']:.4f}   n_bins = {ajuste_binned['n']}")

    Tm = T_all.mean()
    pendiente_local = pendiente_teorica_local(Tm)
    print(f"\n=== Comparación con el modelo teórico ===")
    print(f"  Pendiente teórica local (en T={Tm:.1f}°C): {pendiente_local:.4f} m/s/°C")
    print(f"  v teórico a 0°C: {V0_TEORICO:.2f} m/s")
    diff_b = ajuste_combinado['b'] - V0_TEORICO
    print(f"  Diferencia intercepto (exp - teo): {diff_b:.2f} m/s "
          f"({diff_b/ajuste_combinado['sb']:.1f} sigma)" if ajuste_combinado['sb'] > 0 else "")

    print(f"\nArchivos generados en: {outdir.resolve()}")
    print("  fig1_todos_los_datos.png")
    print("  fig2_promedios_binned.png")
    print("  fig3_residuos.png")
    print("  resumen_ajustes.csv")
    print("  datos_binned.csv")


if __name__ == "__main__":
    main()