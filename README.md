Profit
======

A multithreaded C program that simulates a real-time data stream, performs simple moving average forecasting, and logs results to CSV for visualization. Built to demonstrate the use of threads, mutexes, and condition variables.

# Table of Contents
-----------------

1. [Overview](#overview)

2. [Architecture](#architecture)
   - [Threads and Synchronization](#threads-and-synchronization)
   - [Data Flow](#data-flow)

3. [Components](#components)

4. [Build Instructions](#build-instructions)
   - [Prerequisites](#prerequisites)
   - [Compilation](#compilation)

5. [Usage](#usage)
   - [Run the Producer-Consumer System](#run-the-producer-consumer-system)
   - [Generate the Plot](#generate-the-plot)

6. [Data Visualization](#data-visualization)

7. [File Descriptions](#file-descriptions)

8. [License](#license)

---

## Overview

In the financial trading industry, profit refers to real-time market analysis and trading platforms that process streaming financial data. These systems handle continuous feeds of price quotes, trade volumes, and market indicators, requiring robust concurrent processing to deliver timely insights and automated trading signals.
**Profit** is a small educational project that implements a producer-consumer pattern with an additional forecasting thread. It generates a random-walk data stream, calculates a moving average forecast from recent real values, and outputs all data (real and forecast) to a CSV file. A Python script then reads the CSV and creates a bar/line plot to compare real values with forecasts.


The project highlights:
- Multithreading with POSIX threads (`pthread`)
- Mutexes (`pthread_mutex_t`) for protecting shared data
- Condition variables (`pthread_cond_t`) for thread coordination
- A pipe for inter‑thread communication
- Simple moving average forecasting

---

## Architecture

### Threads and Synchronization

Four threads are created in the main function:
- **Producer** (`p_func`): Generates random data points (values between 0 and 100) and enqueues them into a shared queue.
- **Consumer** (`c_func`): Dequeues data, writes it to CSV, and passes **real** values to the forecast thread via a pipe.
- **Forecaster** (`f_func`): Reads real values from the pipe, maintains a sliding window of the last three real values, and when enough data is available, computes the moving average and enqueues the forecast as a new data point.
- **Plotter** (commented out): A thread that would continuously clear the console and draw a simple bar chart. It is disabled in the provided code because a Python plotting script is used instead.

Synchronization is handled with:
- **Mutexes**:  
  - `mutex` protects the shared queue.  
  - `plot_mutex` protects the `plot_buffer` (used by the disabled plotter).  
  - `file_mutex` ensures exclusive access to the CSV file.
- **Condition variables**:  
  - `not_empty` signals the consumer that the queue has data.  
  - `is_full` signals the producer that the queue has space available.

### Data Flow
```
Producer → Queue → Consumer → CSV
                         ↓ (pipe for real values only)
                    Forecaster → (forecast) → Queue
```

1. The producer generates a random data point and adds it to the queue.
2. The consumer takes the data, logs it to `output.csv`, and if the data is **real** (not a forecast), it writes it into the pipe.
3. The forecaster reads real values from the pipe, keeps a buffer of the last three real values, and when it has at least two values, computes the moving average. It then enqueues the forecast as a new data point (marked `is_forecast = 1`).
4. The consumer will eventually dequeue the forecast and log it to CSV as well.

All threads run continuously until the program is terminated (e.g., with `Ctrl+C`).

---

## Components

- `profit.c` – Main program with thread functions.
- `DataQueue.h` – Header file defining the queue structure and operations (enqueue, dequeue, peek, isEmpty, isFull). It is assumed to be provided separately.
- `plot_generator.py` – Python script that reads `output.csv` and generates a plot using `matplotlib`.

---

## Build Instructions

### Prerequisites

- A C compiler (GCC recommended)
- POSIX threads library (`pthread`)
- Python 3 with `pandas` and `matplotlib` installed for plotting

### Compilation

Compile the C program with the following command (on Linux/macOS):

```
gcc profit.c -o profit -lpthread
```

If you are on Windows and using MinGW, the command is similar:

```
gcc profit.c -o profit.exe -lpthread
```
Make sure `DataQueue.h` is in the same directory as `profit.c`.

* * * * *

Usage
-----

### Run the Producer-Consumer System

Execute the compiled program:

```
./profit 
```

The program will run indefinitely, producing, consuming, and forecasting data. It writes all records to `output.csv` in the current directory. Press `Ctrl+C` to stop the program.

### Generate the Plot

After the program has run long enough to collect some data, run the Python script:

```
python plot_generator.py
```

This will read `output.csv`, create a bar chart for real values and a line plot for forecasts, and display the graph.

* * * * *

Data Visualization
------------------

The Python script produces a plot that compares real and forecasted values over time. An example output is shown below:

[Plot Example](figures/plot_example.png)

*Example plot generated by `plot_generator.py`*

The script does the following:

-   Reads the CSV and normalizes timestamps (relative to the first entry).

-   Plots real values as blue bars.

-   Plots forecast values as a red dashed line.

-   Annotates every second data point with its value.

You can adjust the plot appearance or export it by modifying the script.

* * * * *

File Descriptions
-----------------

| File | Description |
| --- | --- |
| `profit.c` | Main C source code with thread logic, synchronization, and CSV writing. |
| `DataQueue.h` | Queue implementation (not shown) -- must be provided separately. |
| `plot_generator.py` | Python script to read `output.csv` and generate a plot. |
| `output.csv` | Output file generated by the program (created at runtime). |

* * * * *

License
-------

This project is intended for educational purposes. You are free to use and modify it as you wish.
* * * * *
