import serial
import keyboard  # Necesitarás instalar la biblioteca keyboard si aún no lo has hecho

# Configura el puerto serie, asegúrate de cambiar el nombre del puerto según tu configuración
ser = serial.Serial('COM3', 9600)

# Abre un archivo para escribir los datosq
with open('datos_recolectados.txt', 'w') as archivo:
    while True:
        # Lee una línea desde el puerto serie
        linea = ser.readline().decode('utf-8').strip()

        # Escribe la línea en el archivo
        archivo.write(linea + '\n')

        # Muestra la línea en la consola (opcional)
        print(linea)

        # Verifica si se presiona la tecla 'q' para salir del bucle
        if keyboard.is_pressed('q'):
            break