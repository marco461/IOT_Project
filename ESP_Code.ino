/*
pins = {ultrasonic : (23 , 18) ,Gas : 34 , temp : 35 }

*/
#define LED_BUILTIN 2
#include <Arduino.h>
#include <WiFi.h>
#include <FirebaseESP32.h>
#include <EEPROM.h>

// Provide the token generation process info.
#include <addons/TokenHelper.h>

// Provide the RTDB payload printing info and other helper functions.
#include <addons/RTDBHelper.h>
#include <ArduinoJson.h>

// --------------------------------------------------------------------------------
// Including and calling the library
#include <Preferences.h>
Preferences preferences;
// ---------------------------------------------------------------------------------


/* 1. Define the WiFi credentials */

const char* wifiSSIDs[] = { "SSID1", "SSID2", "SSID3" };     // Add your SSIDs here
const char* wifiPasswords[] = { "password1", "password2", "password3" };  // Add the corresponding passwords here

int numberOfNetworks = sizeof(wifiSSIDs) / sizeof(wifiSSIDs[0]);
int currentNetworkIndex = 0;


// For the following credentials, see examples/Authentications/SignInAsUser/EmailPassword/EmailPassword.ino

/* 2. Define the API Key */
#define API_KEY ""

/* 3. Define the RTDB URL */
#define DATABASE_URL ""  //<databaseName>.firebaseio.com or <databaseName>.<region>.firebasedatabase.app

/* 4. Define the user Email and password that alreadey registerd or added in your project */
#define USER_EMAIL ""

#define USER_PASSWORD ""

// Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;


unsigned long sendDataPrevMillis = 0;  // Ensuring the time and delays
unsigned int count = 0;               // Count for each send/receive
// ----------------------------------------------------------------------
// define the number of bytes you want to access
#define EEPROM_SIZE 2
// --------------------------------Ultrasonic Sensor Variables--------------------------------------
// defines pins numbers
const int trigPin = 23;
const int echoPin = 18;
// -------------------------------------------------------------------------------------------------
#define GasSensor 34
// #define TempSensor 35
#define PIRSensor 15
// -------------------------------------------------------------------------------------------------

// -------------------------------------------------------------------------------------------------

void connectToWiFi() {
  Serial.println("Connecting to WiFi...");
  bool connected = false;
  for (int i = 0; i < numberOfNetworks; i++) {
    WiFi.begin(wifiSSIDs[currentNetworkIndex], wifiPasswords[currentNetworkIndex]);
    Serial.print("Attempting to connect to ");
    Serial.print(wifiSSIDs[currentNetworkIndex]);
    Serial.println("...");

    // Try to connect for a maximum of 10 seconds
    int attempts = 10;
    while (WiFi.status() != WL_CONNECTED && attempts > 0) {
      delay(1000);
      Serial.print(".");
      attempts--;
    }
    if (WiFi.status() == WL_CONNECTED) {
      connected = true;
      Serial.println("\nWiFi connected.");
      Serial.print("IP address: ");
      Serial.println(WiFi.localIP());
      break;
    } else {
      Serial.println("\nConnection failed.");
      currentNetworkIndex = (currentNetworkIndex + 1) % numberOfNetworks;
    }
  }

  if (!connected) {
    Serial.println("Failed to connect to WiFi. Please check your credentials");
    connectToWiFi();  // Recursion till connected
  }
}
void setup() {

  Serial.begin(115200);
  // -----------------------------------------------------------------
  pinMode(trigPin, OUTPUT); 
  pinMode(echoPin, INPUT); 
  pinMode(GasSensor, INPUT);
  // pinMode(TempSensor,INPUT);
  pinMode(PIRSensor,INPUT);
  // -----------------------------------------------------------------
  // preferences.begin("my-app", false);
  // unsigned int counter = preferences.getUInt("counter",0);

    // initialize EEPROM with predefined size
  EEPROM.begin(EEPROM_SIZE);
  count = EEPROM.read(0);

  connectToWiFi();
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);

  /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the user sign in credentials */
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;

  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback;  // see addons/TokenHelper.h

  // Comment or pass false value when WiFi reconnection will control by your code or third party library e.g. WiFiManager
  Firebase.reconnectNetwork(true);

  // Since v4.4.x, BearSSL engine was used, the SSL buffer need to be set.
  // Large data transmission may require larger RX buffer, otherwise connection issue or data read time out can be occurred.
  fbdo.setBSSLBufferSize(4096 /* Rx buffer size in bytes from 512 - 16384 */, 1024 /* Tx buffer size in bytes from 512 - 16384 */);

  Firebase.begin(&config, &auth);

  Firebase.setDoubleDigits(5);
  // --------------------------------------------------------------------
  // count = counter; //Store inside an updated variable
}
void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi connection lost. Reconnecting...");
    connectToWiFi();
  }  
  // ----------------------------Ultrasonic Sensor Part----------------------------------------
    // Clears the trigPin
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    // Sets the trigPin on HIGH state for 10 micro seconds
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);
    // Reads the echoPin, returns the sound wave travel time in microseconds
    long duration = pulseIn(echoPin, HIGH, 5000);
    // Calculating the distance
    float distance = duration * 0.034/2;
  // -----------------------------------Gas Sensor Part----------------------------------------
    int gasValue = digitalRead(GasSensor);
  // ----------------------------------Temp Sensor Part----------------------------------------
    // int tempValue = analogRead(TempSensor);
  // -----------------------------------PIR Sensor Part----------------------------------------
    int pirValue = digitalRead(PIRSensor);

  if (Firebase.ready() && (millis() - sendDataPrevMillis > 15000 || sendDataPrevMillis == 0)) {
    sendDataPrevMillis = millis();
    // Prepare the JSON object with the reading and timestamp
    FirebaseJson json;

    String path = "/sensors/" + String(count); // Unique path for each reading
    // json.set("value", count); // The actual reading
    Serial.print("Distance (cm): ");
    Serial.println(distance);
    // Serial.println(pirValue);
    json.set("timestamp/.sv", "timestamp"); // Firebase server-side timestamp
    json.set("Ultrasonnic (Cm) ",distance);
    json.set("PIR",pirValue);
    json.set("GasSensor",gasValue);
    // json.set("Temp ",tempValue);

    // Send the reading and its timestamp to Firebase
    if (Firebase.set(fbdo, path, json)) {
      // Serial.println("Reading and timestamp sent successfully.");
      Serial.print("Reading and timestamp sent successfully ID: ");
      Serial.println(count);

      digitalWrite(LED_BUILTIN, HIGH);
    } else {
      Serial.println("Failed to send reading and timestamp: " + fbdo.errorReason());
    }
    // count++; // Increment the count (reading value) for the next loop iteration
    count++;
    EEPROM.write(0,count);
    EEPROM.commit();
    delay(1000); // Small delay before the next loop iteration
    digitalWrite(LED_BUILTIN, LOW);
  }
}


/*
Initial Setup
#define LED_BUILTIN 2: This defines the built-in LED pin number. It's used for basic feedback on the device's status (e.g., connection status).
#include statements: These include necessary libraries for the program. Arduino.h for basic Arduino functions, WiFi.h for WiFi functionality, FirebaseESP32.h for Firebase integration, and the addons for specific Firebase functionalities like token handling and RTDB helpers.
WiFi Credentials and Connection Management
The original single SSID and password definitions have been commented out and replaced with arrays (wifiSSIDs[] and wifiPasswords[]) to store multiple network credentials. This allows the ESP32 to attempt connections to multiple networks.
numberOfNetworks calculates the total number of networks available by dividing the size of the wifiSSIDs array by the size of its first element.
currentNetworkIndex is initialized to 0, indicating the ESP32 will try to connect to the first network in the list initially.
connectToWiFi() function attempts to connect to WiFi networks in a loop, iterating through the provided SSIDs and passwords. If a connection fails, it tries the next one until a connection is established or all options are exhausted.
Firebase Configuration
Firebase credentials and settings (API key, database URL, user email, and password) are defined. These are necessary to authenticate and communicate with Firebase.
FirebaseData fbdo, FirebaseAuth auth, and FirebaseConfig config objects are instantiated for managing Firebase data, authentication, and configuration, respectively.
Timing and Count Variables
sendDataPrevMillis and count are used to manage the timing of data sending operations and to demonstrate database operations with incrementing values.
setup() Function
Initializes serial communication and calls connectToWiFi() to establish a network connection.
Sets up the LED pin as an output for visual feedback.
Configures Firebase with the previously defined settings and credentials. Also sets buffer sizes and initiates the Firebase connection.
The Firebase.reconnectNetwork(true); statement allows Firebase to manage network reconnection automatically. However, additional logic for WiFi reconnection is also implemented manually.
loop() Function
Checks WiFi connection status at the start of each loop iteration. If the connection is lost, it attempts to reconnect using connectToWiFi().
Periodically (every 15 seconds as defined by the timing logic), it toggles the built-in LED and performs a series of Firebase database operations (set/get bool, int, float, double, string, and JSON values), demonstrating basic CRUD operations with Firebase.
The process is repeated, with the count variable incrementing on each loop to change the data being sent.
Key Functionalities
Multi-WiFi Support: By iterating over an array of SSIDs and passwords, the ESP32 can connect to multiple WiFi networks, enhancing connectivity resilience.
Persistent Connectivity: The loop continuously checks for WiFi connectivity and attempts reconnection if necessary, ensuring the device remains connected to the internet.
Firebase Integration: Demonstrates basic Firebase Realtime Database operations, including writing and reading various data types.
Visual Feedback
The built-in LED toggles state with each data transmission cycle, providing simple visual feedback on the device's operation.
*/