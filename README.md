# IOT_Project_Documentation

## ESP32 Sensor Integration with Firebase and Tkinter Data Monitor

This project involves two main components: an ESP32 microcontroller that reads data from various sensors and uploads it to Firebase, and a Python application that monitors and visualizes this data in real-time using Tkinter.

### Part 1: ESP32 Sensor Integration

#### Initial Setup

- `#define LED_BUILTIN 2`: Defines the pin number for the built-in LED, typically used for status indication.

#### Library Inclusions

- `#include <Arduino.h>`: Provides basic Arduino functions.
- `#include <WiFi.h>`: Enables WiFi functionality.
- `#include <FirebaseESP32.h>`: Facilitates integration with Firebase.
- `#include <EEPROM.h>`: Provides EEPROM functionality for storing data.
- `#include <addons/TokenHelper.h>`: Supports token generation for Firebase authentication.
- `#include <addons/RTDBHelper.h>`: Provides helper functions for Realtime Database operations.
- `#include <ArduinoJson.h>`: Enables manipulation of JSON data.

#### Global Variables

- `Preferences preferences;`: Instantiates a Preferences object for storing preferences.
- `const char* wifiSSIDs[];`: Array storing WiFi network SSIDs.
- `const char* wifiPasswords[];`: Array storing corresponding WiFi network passwords.
- `int numberOfNetworks;`: Stores the number of WiFi networks.
- `int currentNetworkIndex;`: Tracks the index of the current WiFi network being attempted.
- `FirebaseData fbdo;`: FirebaseData object for managing Firebase operations.
- `FirebaseAuth auth;`: FirebaseAuth object for Firebase authentication.
- `FirebaseConfig config;`: FirebaseConfig object for Firebase configuration.
- `unsigned long sendDataPrevMillis = 0;`: Tracks the last time data was sent.
- `unsigned int count = 0;`: Counter for data transmissions.

#### Setup Function

- `void connectToWiFi()`: Function for connecting to WiFi networks.
- `void setup()`: Setup function that runs once at startup.
  - Initializes serial communication.
  - Configures pins for ultrasonic, gas, and PIR sensors.
  - Initializes EEPROM and retrieves the count value.
  - Connects to WiFi networks using `connectToWiFi()`.
  - Configures Firebase with API key, user credentials, and database URL.
  - Initializes Firebase and sets buffer sizes.
  - Prints Firebase client version.

#### Loop Function

- `void loop()`: Main program loop.
  - Checks WiFi connection status and reconnects if necessary.
  - Performs sensor readings (ultrasonic, gas, PIR).
  - Sends sensor data to Firebase periodically.
  - Toggles built-in LED to indicate data transmission.

#### Key Functionalities

- **Multi-WiFi Support**: Supports connecting to multiple WiFi networks.
- **Persistent Connectivity**: Continuously checks and maintains WiFi connection.
- **Firebase Integration**: Performs basic Firebase Realtime Database operations.
- **Visual Feedback**: Provides visual indication of data transmission with built-in LED.

#### Additional Notes

- The code demonstrates basic usage of sensors and Firebase integration on an ESP32 platform.
- It includes error handling for WiFi connection and Firebase operations.

### Part 2: Tkinter Data Monitor Application

#### Library Inclusions

- `tkinter` and `tkinter.scrolledtext`: Provides the GUI framework.
- `firebase_admin`: Facilitates integration with Firebase.
- `matplotlib`: Used for plotting sensor data.
- `datetime`: Used for handling timestamps.

#### Firebase Initialization

The Firebase Admin SDK is initialized with a credentials file and database URL:
```python
databaseURL = 'your-database-url'
cred_obj = credentials.Certificate("path-to-your-credentials-file.json")
default_app = initialize_app(cred_obj, {'databaseURL': databaseURL})
```

#### Tkinter Application Setup

The `DataMonitorApp` class initializes the Tkinter application, sets up the text display and chart, and starts a listener for Firebase data changes.

#### Class Constructor

- **Text Frame**: Displays sensor data in a scrollable text box.
- **Chart Frame**: Displays sensor data in a real-time chart.
- **Firebase Listener**: Listens for changes in the Firebase Realtime Database and updates the display and chart.

#### Updating the Display

The `update_display` method updates the text box with new sensor data, keeping only the last 10 entries:
```python
def update_display(self, message):
    self.display.config(state=tk.NORMAL)
    self.display.insert(tk.END, message)
    self.display_contents = self.display.get('1.0', tk.END).split('\n\n')
    if len(self.display_contents) > 10:
        self.display.delete('1.0', tk.END)
        new_content = '\n\n'.join(self.display_contents[-11:])
        self.display.insert(tk.END, new_content)
    self.display.config(state=tk.DISABLED)
    self.display.see(tk.END)
```

#### Firebase Listener

The `firebase_listener` method processes incoming data from Firebase, updates the display, and the chart:
```python
def firebase_listener(self, event):
    try:
        if isinstance(event.data, dict) and 'GasSensor' in event.data and 'Ultrasonnic (Cm)' in event.data and 'timestamp' in event.data and 'PIR' in event.data:
            timestamp = datetime.fromtimestamp(event.data['timestamp'] / 1000)
            gas_sensor_value = event.data['GasSensor']
            ultrasonic_value = event.data['Ultrasonnic (Cm)']
            pir_value = event.data['PIR']

            event_data = f"Timestamp: {timestamp}\nGas Sensor Value: {gas_sensor_value}\nUltrasonic Value: {ultrasonic_value}\nPIR Value: {pir_value}\n\n"
            self.update_display(event_data)
            self.update_chart(timestamp, gas_sensor_value, ultrasonic_value, pir_value)
    except Exception as e:
        print(f"Error processing event data: {e}")
```

#### Updating the Chart

The `update_chart` method updates the chart with new sensor data, keeping only the last 10 entries:
```python
def update_chart(self, timestamp, gas_sensor_value, ultrasonic_value, pir_value):
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
```

### Running the Application

The application starts by creating a Tkinter root window and passing it to the `DataMonitorApp` class:
```python
if __name__ == "__main__":
    root = tk.Tk()
    app = DataMonitorApp(root)
    root.mainloop()
```

### Key Functionalities

- **Real-Time Data Monitoring**: Continuously listens for sensor data updates from Firebase.
- **Data Visualization**: Displays sensor data in a scrollable text box and plots the data in real-time charts.
- **Error Handling**: Includes basic error handling for Firebase events.

### License

This project is licensed under the MIT License. See the `LICENSE` file for details.

![ESP32 Setup](images/esp32_setup.jpg)
