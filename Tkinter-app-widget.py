import tkinter as tk
from tkinter import scrolledtext, Frame
from firebase_admin import credentials, initialize_app, db
import matplotlib.pyplot as plt
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
from datetime import datetime
import matplotlib.dates as mdates

# Initialize Firebase admin SDK
databaseURL = ''
cred_obj = credentials.Certificate("")
default_app = initialize_app(cred_obj, {'databaseURL': databaseURL})

# Set up the Tkinter application
class DataMonitorApp:
    def __init__(self, root):
        self.root = root
        self.root.title("Firebase Data Monitor")

        # Create a frame for the text display
        self.text_frame = Frame(root)
        self.text_frame.pack(side="right", fill="both", expand=True)
        
        self.display = scrolledtext.ScrolledText(self.text_frame, width=70, height=10)
        self.display.pack(pady=20)

        # Create a frame for the chart
        self.chart_frame = Frame(root)
        self.chart_frame.pack(side="left", fill="both", expand=True)

        # Setting up the chart
        self.fig, self.ax = plt.subplots(figsize=(5, 4))
        self.canvas = FigureCanvasTkAgg(self.fig, master=self.chart_frame)
        self.canvas.draw()
        self.canvas.get_tk_widget().pack(side="top", fill="both", expand=True)

        self.dates = []
        self.gas_sensor_values = []      
        self.ultrasonic_values = []  
        self.pir_values = []

        # Initialize the Firebase listener in the app
        self.ref = db.reference("/sensors/")
        self.ref.listen(self.firebase_listener)

    def update_display(self, message):
        # Update the display with new Firebase event data, keeping only last 10 entries
        self.display.config(state=tk.NORMAL)
        self.display.insert(tk.END, message)
        self.display_contents = self.display.get('1.0', tk.END).split('\n\n')
        if len(self.display_contents) > 10:  # Keep only the last 10 records
            self.display.delete('1.0', tk.END)
            new_content = '\n\n'.join(self.display_contents[-11:])
            self.display.insert(tk.END, new_content)
        self.display.config(state=tk.DISABLED)
        self.display.see(tk.END)

    def firebase_listener(self, event):
        try:
            if isinstance(event.data, dict) and 'GasSensor' in event.data and 'Ultrasonnic (Cm)' in event.data and 'timestamp' in event.data and 'PIR' in event.data:
                timestamp = datetime.fromtimestamp(event.data['timestamp'] / 1000)
                gas_sensor_value = event.data['GasSensor']
                ultrasonic_value = event.data['Ultrasonnic (Cm)']
                pir_value = event.data['PIR']

                # Update the display with sensor data
                event_data = f"Timestamp: {timestamp}\nGas Sensor Value: {gas_sensor_value}\nUltrasonic Value: {ultrasonic_value}\nPIR Value: {pir_value}\n\n"
                self.update_display(event_data)

                # Update the chart with the new sensor data
                self.update_chart(timestamp, gas_sensor_value, ultrasonic_value, pir_value)
        except Exception as e:
            print(f"Error processing event data: {e}")

    def update_chart(self, timestamp, gas_sensor_value, ultrasonic_value ,pir_value):
        # Add new data to the chart and redraw, keeping only last 10 entries
        self.dates.append(timestamp)
        self.gas_sensor_values.append(gas_sensor_value)
        self.ultrasonic_values.append(ultrasonic_value)
        self.pir_values.append(pir_value)

        if len(self.dates) > 10:
            self.dates = self.dates[-10:]
            self.gas_sensor_values = self.gas_sensor_values[-10:]
            self.ultrasonic_values = self.ultrasonic_values[-10:]
            self.pir_values = self.pir_values[-10:]

        self.ax.clear()
        self.ax.plot(self.dates, self.gas_sensor_values, label='Gas Sensor')
        self.ax.plot(self.dates, self.ultrasonic_values, label='Ultrasonic')
        self.ax.plot(self.dates, self.pir_values, label='PIR')
        self.ax.legend()
        self.ax.xaxis.set_major_formatter(mdates.DateFormatter('%H:%M:%S'))
        self.ax.xaxis.set_major_locator(mdates.SecondLocator(interval=10))
        self.ax.set_xlabel("Time")
        self.ax.set_ylabel("Value")
        self.fig.autofmt_xdate()
        self.canvas.draw()


# Create the main window and pass it to the DataMonitorApp class
if __name__ == "__main__":
    root = tk.Tk()
    app = DataMonitorApp(root)
    root.mainloop()

