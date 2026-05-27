import serial
import numpy as np
import pandas as pd

from pathlib import Path

def setup(serial_port: str, 
          baudrate: int = 115200) -> serial.Serial:
    """
    Initializes and sets up a serial connection.

    Args:
        serial_port (str): The name of the serial port to connect to (e.g., '/dev/ttyUSB0').
        baudrate (int, optional): The baud rate for the serial connection. Defaults to 115200.

    Returns:
        serial.Serial: A configured serial connection object.

    Raises:
        serial.SerialException: If the serial connection cannot be established.
    """
    print(f"Setting up serial connection on port {serial_port} with baudrate {baudrate}...")
    return serial.Serial(serial_port, baudrate, timeout=1)

# Dictionary to store the collected data for each gesture
data = {}

def take_data(take_gesture: int, 
              take_samples: int = 300) -> None:
    """
    Collects data for a specific gesture from a serial device.

    Args:
        take_gesture (int): The gesture identifier for which data is being collected.
        take_samples (int, optional): The number of data samples to collect. Defaults to 300.

    Returns:
        None

    Raises:
        serial.SerialException: If the serial connection cannot be established.
        ValueError: If the incoming data cannot be parsed into floats.

    Notes:
        - The function sends a signal to the microcontroller to start sending data.
        - Data is collected, scaled down by a factor of 10, and stored in the global `data` dictionary.
        - The serial connection is closed after data collection.
    """
    from time import sleep
    data[take_gesture] = []
    ser = setup('/dev/ttyUSB0')
    sleep(2)
    print(f"Collecting data for gesture {take_gesture}...")   
    ser.write(b"a")
    sleep(0.1)

    samples_collected = 0
    while samples_collected < take_samples:
        line = ser.readline().decode().strip()
        if not line:
            continue
        if line == "FIN":
            print("ESP32 finished sending data")
            break
        try:
            values = list(map(float, line.split(',')))
            if len(values) == 3:
                data[take_gesture].append(values)
                samples_collected += 1
        except ValueError:
            print("Invalid line:", line)

    print(f"Collected {len(data[take_gesture])} samples for gesture {take_gesture}")
    data[take_gesture] = np.array(data[take_gesture]) / 10
    ser.close()

def save_data(labels_file_name: str, 
              train_data_file_name: str, 
              data_per_run: int = 300) -> None:
    """
    Saves gesture data and corresponding labels to files.

    Args:
        labels_file_name (str): The name of the file to save gesture labels.
        train_data_file_name (str): The name of the file to save gesture data.
        data_per_run (int, optional): The number of samples to collect per gesture. Defaults to 300.

    Returns:
        None

    Raises:
        FileNotFoundError: If the base directory or data folder does not exist.
        ValueError: If data collection or saving encounters issues.

    Notes:
        - Creates a labels file if it does not already exist.
        - Collects data for three gestures, waiting 20 seconds between each gesture.
        - Saves the collected data to a CSV file.
        - The `data` dictionary is used to store the collected data globally.
    """
    BASE_DIR = Path(__file__).resolve().parents[2]  # original project folder
    labels_path = BASE_DIR / "data" / labels_file_name
    train_data_path = BASE_DIR / "data" / train_data_file_name

    if not labels_path.exists():
        # Create the labels file with the desired content
        labels = [0] * data_per_run + [1] * data_per_run + [2] * data_per_run
        np.savetxt(labels_path, labels, fmt="%d")
        print(f"File '{labels_path}' created successfully.")
    else:
        print(f"The file '{labels_path}' already exists.")

    for gesture in range(3):  # collect data for 3 gestures
        take_data(gesture, data_per_run)

        # Wait 20 seconds to put the accelerometer in the next gesture position
        print("Waiting 20 seconds for the next gesture...")
        for _ in range(1, 21):
            print(f"{_}", end=" ", flush=True)
            from time import sleep
            sleep(1)
        print() # Move to the next line after the countdown

    # Save the collected data to csv file
    all_data = np.vstack([data[g] for g in range(3)])  # shape (900, 3)
    pd.DataFrame(all_data).to_csv(train_data_path, index=False, header=False)
    print(f"Data saved to '{train_data_path}' successfully.")

def run_data_collection():
    """
    Runs the data collection pipeline.
    """
    save_data("train_labels.txt", "train_data.csv")