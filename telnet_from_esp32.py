import telnetlib
import threading
import tkinter as tk
from tkinter import ttk
import matplotlib.pyplot as plt
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
import numpy as np

# Telnet connection settings
HOST = '192.168.1.251'
PORT = 4989

# Initialize global variables
time_ms = 0
avg_loop_time_us = 0.0
max_loop_time_us = 0
speed = 0.0
position = 0
speed_data = []
position_data = []
max_loop_time_data = []

# Establish Telnet connection
tn = telnetlib.Telnet(HOST, PORT)

def update_data():
    global time_ms, avg_loop_time_us, max_loop_time_us, speed, position, speed_data, position_data, max_loop_time_data

    while True:
        # Read data from Telnet
        response = tn.read_until(b'\n').decode('ascii').strip()
        print(f"Raw data received: {response}")  # Debugging output
        try:
            # Split the comma-separated string into individual values
            values = response.split(',')
            if len(values) == 5:
                # Strip whitespace and convert to appropriate types
                time_ms = int(values[0].strip())
                avg_loop_time_us = float(values[1].strip())
                max_loop_time_us = int(values[2].strip())
                speed = float(values[3].strip())
                position = int(values[4].strip())
                
                speed_data.append(speed)
                position_data.append(position)
                max_loop_time_data.append(max_loop_time_us)
                
                if len(speed_data) > 100:
                    speed_data.pop(0)
                    position_data.pop(0)
                    max_loop_time_data.pop(0)
            else:
                print(f"Unexpected data format: {response}")
        except ValueError as e:
            print(f"ValueError: {e} - received data: {response}")
            continue

        # Update GUI
        time_label.config(text=f"Time: {time_ms} ms")
        loop_time_label.config(text=f"Avg Loop Time: {avg_loop_time_us} µs")
        max_loop_time_label.config(text=f"Max Loop Time: {max_loop_time_us} µs")
        canvas.draw()

def send_command(command):
    tn.write(command.encode('ascii') + b'\n')

def on_closing():
    tn.close()
    root.destroy()

# Create main window
root = tk.Tk()
root.title("Telnet Control and Plot")

# Create labels for time, avg loop time, and max loop time
time_label = ttk.Label(root, text="Time: 0 ms")
time_label.pack(pady=5)

loop_time_label = ttk.Label(root, text="Avg Loop Time: 0.0 µs")
loop_time_label.pack(pady=5)

max_loop_time_label = ttk.Label(root, text="Max Loop Time: 0 µs")
max_loop_time_label.pack(pady=5)

# Create plot for speed, position, and max loop time
fig, ax = plt.subplots(3, 1, figsize=(6, 6))
canvas = FigureCanvasTkAgg(fig, master=root)
canvas.get_tk_widget().pack(side=tk.TOP, fill=tk.BOTH, expand=1)

ax[0].set_title("Speed")
ax[0].set_xlabel("Sample")
ax[0].set_ylabel("Speed")

ax[1].set_title("Position")
ax[1].set_xlabel("Sample")
ax[1].set_ylabel("Position")

ax[2].set_title("Max Loop Time")
ax[2].set_xlabel("Sample")
ax[2].set_ylabel("Max Loop Time (µs)")

def update_plot():
    ax[0].cla()
    ax[0].set_title("Speed")
    ax[0].set_xlabel("Sample")
    ax[0].set_ylabel("Speed")
    ax[0].plot(speed_data, label="Speed")
    ax[0].legend()

    ax[1].cla()
    ax[1].set_title("Position")
    ax[1].set_xlabel("Sample")
    ax[1].set_ylabel("Position")
    ax[1].plot(position_data, label="Position")
    ax[1].legend()

    ax[2].cla()
    ax[2].set_title("Max Loop Time")
    ax[2].set_xlabel("Sample")
    ax[2].set_ylabel("Max Loop Time (µs)")
    ax[2].plot(max_loop_time_data, label="Max Loop Time")
    ax[2].legend()

    canvas.draw()
    root.after(100, update_plot)

# Create control buttons
button_frame = ttk.Frame(root)
button_frame.pack(pady=10)

run_button = ttk.Button(button_frame, text="RUN", command=lambda: send_command('r'))
run_button.grid(row=0, column=0, padx=5)

stop_button = ttk.Button(button_frame, text="STOP", command=lambda: send_command('s'))
stop_button.grid(row=0, column=1, padx=5)

home_button = ttk.Button(button_frame, text="HOME", command=lambda: send_command('h'))
home_button.grid(row=0, column=2, padx=5)

reverse_button = ttk.Button(button_frame, text="REVERSE", command=lambda: send_command('b'))
reverse_button.grid(row=0, column=3, padx=5)

# Start the data update thread
data_thread = threading.Thread(target=update_data, daemon=True)
data_thread.start()

# Start the plot update loop
root.after(100, update_plot)

# Set the close event
root.protocol("WM_DELETE_WINDOW", on_closing)

# Run the Tkinter event loop
root.mainloop()
