# Physics Lab Experiments

![Python](https://img.shields.io/badge/Python-3.12-blue.svg)
![Jupyter Notebook](https://img.shields.io/badge/Jupyter-Notebook-orange.svg)
![Arduino](https://img.shields.io/badge/Arduino-IDE-teal.svg)

This is a repository to store all my Experimental Physics projects, homeworks, exams, done by me and my colleagues during my Physics degree at the University of Antioquia (UdeA) in Medellín, Colombia, ranging from August 9, 2023 to today March 28, 2026 and still in progress. As context, Physics degree at UdeA has the following experimental physics curriculum:
- Experimental physics I, II, III, IV
- Advanced laboratory I, II, III
If you want to know more about the Physics curriculum at UdeA, visit: [Physics curriculum at UdeA](https://www.udea.edu.co/wps/portal/udea/web/inicio/unidades-academicas/ciencias-exactas-naturales/estudiar-facultad/pregrados/fisica)

In experimental physics (exp) II the main idea was to familiarize with measurement instruments, although this has been done in basically each of the experimental courses. In expIII, statistical concepts were applied to physical data obtained either by human measurements, as well as computational ones. In expIV the computational measurements were done more consistently, specifically using the software and hardware of Arduino, this also has been the case for advanced laboratory I, but the difference is that expIV is focused on understanding the principles of modern Physics, such as photo electric effect, spectrometry and so on... advanced laboratory I is mainly focused on the computational or technological usage of scientific software and hardware in advanced physics laboratories around the world.

Here is the project structure (only the main files are shown below for brevity), each folder contains its main notebook (.ipynb) or python (.py) file, there are also additional documents such as Arduino (.ino) files and data used in each analysis, either as .csv or .txt formats, as well as Latex (.tex) files:

## ADVANCED-LAB-I
- additional-reference-code/
  - connecting-arduino-python.py

## EXP-PHY-II
- 1.0-normal-poisson-distribution-CLT/
  - 1.0-geiger_detector_and_marbles_diameters.ipynb
- 2.0-uncertainty-report/
  - uncertainty-report-assignment.ipynb
- 3.0-data-adjustment/
  - data_adjustment_assignment.ipynb
- 5.0-multimeter-calibration/
  - multimeter_calibration.ipynb
- 6.0-final-project-magnetic-field-detection/
  - data_analysis.ipynb
  - guardar_datos_txt.py

## EXP-PHY-III
- exams/
  - Exam_2_Experimental_Physics_III_2024_1-EstivenCastrillon.ipynb
- homeworks/
  - hw1-geiger-counter/
    - notebook-hw1-geiger-count-analysis.ipynb
  - hw2-pendulum-t-student/
    - notebook-tarea2-distribucion-tstudent.ipynb
- pre-laboratories/
  - Prelaboratorio 1/
    - organización-de-archivos-y-medidas.ipynb
    - understanding_the_prelaboratory1.ipynb

## EXP-PHY-IV
- practice1-nitrogen-diode-10-08-2025/
  - practice1-diode-nitrogen.ipynb
- practice2-particle-counter-10-15-2025/
  - code-practice2.ipynb
- practice3-photoelectric-effect/
  - practica3-efecto-fotoelectrico.ipynb

## Installation
To run the Jupyter notebooks and Python scripts, install the required packages using:
```bash
pip install -r requirements.txt
```

## Usage / How to Run

This repository contains different types of files used for data collection and analysis. Here is how to use them:

* **Jupyter Notebooks (`.ipynb`)**: Used primarily for statistical analysis, data adjustment, and generating plots. You can open and execute these files using [Jupyter Lab](https://jupyter.org/), [Jupyter Notebook](https://jupyter.org/), or an IDE like [VS Code](https://code.visualstudio.com/) with the Jupyter extension.
* **Python Scripts (`.py`)**: Used mainly for automated data collection or auxiliary tasks. Run them from your terminal or command prompt. For example:
  ```bash
  python path/to/script.py
  ```
* **Arduino Sketches (`.ino`)**: Used to collect physical data directly from sensors (e.g., Hall effect sensors, Geiger counters). To compile and upload these to your Arduino board, download and use the [Arduino IDE](https://www.arduino.cc/en/software). Often, these sketches are meant to run in tandem with a Python script that reads the serial output and saves it to `.csv` or `.txt` files.

## License
This project is licensed under the terms of the MIT License - see the [LICENSE](LICENSE) file for details.

## Contact
**Estiven Castrillón** - [LinkedIn](www.linkedin.com/in/estiven-castrillon-72823a253)

---

*Note1: The addition of files such as Difracción_y_estructura_de_elementos_ópticos_periódicos.pdf or any other pdf in the GitHub repository is done solely on the purpose of showing the final results of important achievements during the courses such as paper-like structured experiments done in the laboratories. I am aware of the bad practice it is to upload binary files to GitHub repositories, but it is done for the sake of clarity.*

*Note2: Many of the code, commets, results, text have been translated to English using both Estiven's knowledge and Artificial Intelligence tools like Claude and ChatGPT, translations might not be completely accurate but were double revised to ensure they are. There are still parts of the repository (mainly data files) that have Spanish words, although they are simple and generally cognates.*

**Special thanks to Ana María Hurtado (<ana.hurtador@udea.edu.co>), Ana Sofía Mora (<ana.mora@udea.edu.co>), David Alejandro Arboleda (<dalejandro.arboleda1@udea.edu.co>), Sebastián Duque (<sebastian.duque6@udea.edu.co>), Sofía Moscoso (<sofia.moscoso@udea.edu.co>), for beign part of my experimental physics courses, for beign my team and learn together.**