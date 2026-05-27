import pandas as pd
import tensorflow as tf

from keras import Sequential, layers
from pathlib import Path

BASE_DIR = Path(__file__).resolve().parents[2]  # original project folder

def model_definition(input_shape: tuple = (3,), 
                     activation1: str = 'relu',
                     activation2: str = 'relu',
                     activation3: str = 'softmax') -> Sequential:
    """
    Defines a Keras Sequential model for gesture recognition based on accelerometer data.
    Args:
    input_shape (tuple, optional): The shape of the input data. Defaults to (3,).
    activation1 (str, optional): Activation function for the first hidden layer. Defaults to 'relu'.
    activation2 (str, optional): Activation function for the second hidden layer. Defaults to 'relu'.
    activation3 (str, optional): Activation function for the output layer. Defaults to 'softmax'.
    Returns:
    Sequential: A Keras Sequential model instance.
    """
    # Assume input shape is (3,) representing x, y, z values of accelerometer
    model = Sequential([
        # Sequential model, layers go one by one, each after the other.
        layers.InputLayer(shape=input_shape),
        # Define the input layer of the model, each data point has 3 values
        # accelerometer → x, y, z
        layers.Dense(16, activation=activation1),
        # Dense: fully connected layer
        # 16: number of neurons
        # relu: activation function (negative values = 0, positive values remain)
        layers.Dense(16, activation=activation2),
        layers.Dense(3, activation=activation3)
        # 3 output classes for 3 gestures
        # Output layer, 3: number of classes, 3 different gestures
        # softmax: converts the output into probabilities.
    ])
    return model

def model_training(model: Sequential, 
                   train_data_name: str, 
                   train_labels_name: str,
                   epochs: int = 10,
                   metrics: list = ['accuracy']) -> None:
    """
    Trains the given model using the specified training data and labels.
    Args:
        model (Sequential): The Keras Sequential model to be trained.
        train_data_name (str): The name of the CSV file containing the training data.
        train_labels_name (str): The name of the CSV file containing the training labels.
        epochs (int, optional): The number of epochs to train the model. Defaults to 10.
        metrics (list, optional): A list of metrics to evaluate during training. Defaults to ['accuracy'].
    """
    # Model compilation
    model.compile(
        optimizer='adam',
        loss='sparse_categorical_crossentropy',
        metrics=metrics
    )

    labels_path = BASE_DIR / "data" / train_labels_name
    train_data_path = BASE_DIR / "data" / train_data_name

    # Reading data
    train_data = pd.read_csv(train_data_path, header=None).values
    train_labels = pd.read_csv(labels_path, header=None, delimiter='\t').values.flatten()
    
    # Model training
    model.fit(train_data, train_labels, epochs=epochs)

    # Saving model
    models_path = BASE_DIR / "models"
    model.save(models_path / "keras_model_gestures_original.keras")

def model_to_tflite(model_path: str, tflite_path: str) -> None:
    """Converts a Keras model to TensorFlow Lite format and saves it as a .tflite file and a C header file.
    Args:        
        model_path (str): The path to the directory containing the Keras model file.
        tflite_path (str): The path to the directory where the TFLite model and C header
        file will be saved.
    Returns:
        None (saves the files to the specified locations)
    """
    # Load the Keras model from the specified path
    # Doing this ensures the models is correctly saved by the function model_training()
    model = tf.keras.models.load_model(model_path / "keras_model_gestures_original.keras")

    # Convert to TFLite
    converter = tf.lite.TFLiteConverter.from_keras_model(model)
    tflite_model = converter.convert()

    # Save the TFLite model to a file
    with open(tflite_path / "gesture_model_tflite.tflite", "wb") as f:
        f.write(tflite_model)

    # Convert the model to a byte array for inclusion in C code
    with open(tflite_path / "gesture_model_tflite.tflite", "rb") as f:
        data = f.read()

    # Save the byte array as a C header file
    with open(tflite_path / "model.h", "w") as f:
        f.write("const unsigned char gesture_model_tflite[] = {")
        f.write(",".join(str(b) for b in data))
        f.write("};\n")
        f.write(f"const int gesture_model_tflite_len = {len(data)};")

def run_model_pipeline():
    """
    Runs the entire model pipeline: training and conversion to TFLite.
    """
    model = model_definition()
    model_training(model, "train_data.csv", "train_labels.txt")
    model_to_tflite(BASE_DIR / "models", BASE_DIR / "models")