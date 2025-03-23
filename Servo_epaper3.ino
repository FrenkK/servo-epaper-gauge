#define ENABLE_GxEPD2_GFX 0

#include <GxEPD2_BW.h> // Include library for monochrome e-paper display
#include <GxEPD2_3C.h> // Include library for tri-color e-paper display
#include <Fonts/FreeMonoBold18pt7b.h> // Include font library for bold text at 18pt
#include <Fonts/FreeMonoBold9pt7b.h> // Include font library for bold text at 9pt
#include <Fonts/FreeMonoBold24pt7b.h> // Include font library for bold text at 24pt
#include <Fonts/FreeMonoBold12pt7b.h> // Include font library for bold text at 12pt
#include <Fonts/FreeMono12pt7b.h> // Include font library for standard text at 12pt
#include "GxEPD2_display_selection_new_style.h" // Include custom display selection header file
#include <ESP32Servo.h> // Include servo library for motor control
#include <WiFi.h> // Include WiFi library for network communication
#include "Adafruit_MQTT.h" // Include MQTT library for messaging
#include "Adafruit_MQTT_Client.h" // Include MQTT client library

// Define WiFi access parameters
// Code credit: https://randomnerdtutorials.com/esp32-useful-wi-fi-functions-arduino/#3
const char *SSID = "ssid"; // Network name (SSID)
const char *wifiPass = "pass"; // Password for network connection
const char *hostName = "prikaz-lopa"; // Hostname for MQTT communication
WiFiClient wifiClient; // Create WiFi client object

// Define MQTT broker parameters
// Code credit: https://www.donskytech.com/adafruit-mqtt-library-tutorial/
const char *mqttBroker = "broker"; // Name of MQTT broker
const int mqttPort = 1883; // Port number for MQTT communication
const char *mqttUserName = "user"; // Username for MQTT connection
const char *mqttUserPassword = "pass"; // Password for MQTT connection

// Create Adafruit MQTT client object with WiFi client as underlying transport layer
Adafruit_MQTT_Client mqtt(&wifiClient, mqttBroker, mqttPort, mqttUserName, mqttUserPassword);

// Define MQTT subscription topics
const char *subscribeGridTopic = "power/grid"; // Topic for grid power data
const char *subscribeSolarTopic = "power/solar"; // Topic for solar production power data
const char *subscribeHouseTopic = "power/house"; // Topic for house power consumption data
const char *subscribeModeTopic = "power/mode"; // Topic for mode of operation (normal or emergency)
const char *subscribeBatteryTopic = "power/battery"; // Topic for battery voltage and state
const char *subscribeEnergyTopic = "power/energy"; // Topic for total energy produced/consumed

// Create MQTT subscriber objects for each topic
Adafruit_MQTT_Subscribe gridPowerRef = Adafruit_MQTT_Subscribe(&mqtt, subscribeGridTopic, MQTT_QOS_0); // Subscribe to grid power data at QoS 1
Adafruit_MQTT_Subscribe solarPowerRef = Adafruit_MQTT_Subscribe(&mqtt, subscribeSolarTopic, MQTT_QOS_0); // Subscribe to solar panel power data at QoS 1
Adafruit_MQTT_Subscribe housePowerRef = Adafruit_MQTT_Subscribe(&mqtt, subscribeHouseTopic, MQTT_QOS_0); // Subscribe to house power consumption data at QoS 1
Adafruit_MQTT_Subscribe modeRef = Adafruit_MQTT_Subscribe(&mqtt, subscribeModeTopic, MQTT_QOS_0); // Subscribe to mode of operation (normal or emergency) at QoS 1
Adafruit_MQTT_Subscribe batteryRef = Adafruit_MQTT_Subscribe(&mqtt, subscribeBatteryTopic, MQTT_QOS_0); // Subscribe to battery voltage and state data at QoS 1
Adafruit_MQTT_Subscribe energyRef = Adafruit_MQTT_Subscribe(&mqtt, subscribeEnergyTopic, MQTT_QOS_0); // Subscribe to total energy produced/consumed data at QoS 1

//Servo defs
//Left servo init
Servo leftServo;  // create servo object to control a servo
int leftServoStart = 20; // servo value for 0
int leftServoEnd = 177; // servo value for maxPower
int leftServoPin = 25; // Pin number for connecting left servo
//Servo 2 init
Servo centerServo;  // create servo object to control a servo
int centerServoStart = 177; // servo value for 0
int centerServoEnd = 31; // servo value for maxPower
int centerServoPin = 26; // pin number for connecting center servo
//Servo 3 init
Servo rightServo;  // create servo object to control a servo
int rightServoStart = 177; // servo value for 0
int rightServoEnd = 20; // servo value for maxPower
int rightServoPin = 27; // pin number for connecting right servo

//Globals
String mode1 = ""; // First line of status display
String mode2 = ""; // Second line of status display
float batteryVoltage = 58; // Battery voltage
float producedEnergy = 0; // Total energy produced today
long lastRefresh = 0; // Timestamp of last screen refresh
bool normalMode = false; // Normal mode flag

//Constants

// Status strings
// The following constants are used to display the current mode on the screen.
// These string values represent different modes of operation and their corresponding status messages.

// In emergency charging mode (deep discharge prevention), show the following messages on the first and second lines.
const String emergencyString1 = "Zasilno";
const String emergencyString2 = "polnjenje";
// If the battery is empty, show these strings on the first and second lines.
const String batEmptyString1 = "Prazna";
const String batEmptyString2 = "baterija";
// When in normal mode, display these strings on the first and second lines.
const String normalString1 = "Na";
const String normalString2 = "bateriji";
// If the battery is full, display these strings on the first and second lines.
const String fullString1 = "Polna";
const String fullString2 = "baterija";
// When there's no grid power available, display these strings on the first and second lines.
const String noGridString1 = "Izpad";
const String noGridString2 = "omrezja!";

// Number constants
// Maximum house power consumption
const float maxHousePower = 14000; // Watts (W)
// Maximum grid power consumption
const float maxGridPower = 14000; // W
// Maximum solar panel power generation
const float maxSolarPower = 11000; // W
// Maximum battery voltage
const float maxVoltage = 58; // Volts (V)
// Minimum battery voltage
const float minVoltage = 47; // V
// Low battery threshold, used to change display color to red when lower than this value
const float lowVoltage = 49; // V
// Minimum and maximum angles for left servo motor in radians
const float alphaMinLeft = 20 * DEG_TO_RAD; // rad
const float alphaMaxLeft = 160 * DEG_TO_RAD; // rad
// Minimum and maximum angles for center servo motor in radians
const float alphaMinCenter = 25 * DEG_TO_RAD; // rad
const float alphaMaxCenter = 155 * DEG_TO_RAD; // rad
// Minimum and maximum angles for right servo motor in radians
const float alphaMinRight = 20 * DEG_TO_RAD; // rad
const float alphaMaxRight = 160 * DEG_TO_RAD; // rad
// Coordinates of center points for left, center, and right servos on the screen
const int xCLeft = -49; // x-coordinate (pixels)
const int yCLeft = 150; // y-coordinate (pixels)
const int RLeft = 150; // radius (pixels)
const int xCCenter = 200; // x-coordinate (pixels)
const int yCCenter = 389; // y-coordinate (pixels)
const int RCenter = 190; // radius (pixels)
const int xCRight = 449; // x-coordinate (pixels)
const int yCRight = 150; // y-coordinate (pixels)
const int RRight = 150; // radius (pixels)
// Refresh time for display in milliseconds
const long refreshTime = 600000; // ms

/**
 * Initialize WiFi connection.
 *
 * Connects to the specified network with the given credentials and sets up WiFi configuration.
 */
void initWiFi() {
  // Set host name of ESP32 device
  WiFi.setHostname(hostName);

  // Set WiFi mode to station (STA) mode, which means we're a client, not an access point
  WiFi.mode(WIFI_STA);

  // Start WiFi connection with given credentials
  WiFi.begin(SSID, wifiPass);

  // Print dots while waiting for WiFi connection
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000); // wait 1 second between dots
  }

  // Print successful connection message and display local IP address
  Serial.println(WiFi.localIP());
}

// Function to connect and reconnect as necessary to the MQTT server.
// Should be called in the loop function and it will take care if connecting.
void connectToMQTTServer() {
  // Print a comment indicating that this function is being called
  Serial.println("Connecting to MQTT...");

  int8_t ret = mqtt.connect(); // Attempt connection to MQTT server

  while (ret != 0) { // Check if connection was successful
    // If connection failed, print an error message and wait for 10 seconds before retrying
    Serial.print("Failed to connect: ");
    Serial.println(mqtt.connectErrorString(ret));
    Serial.println("Retrying MQTT connection in 10 seconds...");
    delay(10000); // Wait for 10 seconds

    ret = mqtt.connect(); // Attempt connection again
  }

  // Print a comment indicating that the connection was successful
  Serial.println("MQTT Connected!");
}

// Function to draw the battery gauge on the e-paper display.
// The battery gauge is a rectangle with a filled part that represents the battery voltage.
// The filled part is colored black if the battery voltage is above the low voltage threshold, and red otherwise.
// The battery voltage is displayed in volts at the bottom of the gauge.
// The function takes the battery voltage as input and draws the gauge accordingly.
void drawBatteryGauge(float batteryVoltage)
{
  int gaugeWidth = 250; // Width of the battery gauge
  int gaugeHeight = 20; // Height of the battery gauge
  int gaugeX = 75; // X position of the battery gauge
  int gaugeY = 20; // Y position of the battery gauge

  // Draw the outline of the battery
  display.drawRect(gaugeX, gaugeY, gaugeWidth, gaugeHeight, GxEPD_BLACK);

  // Limit values for bar
  float tempBatteryVoltage=0;
  if ((batteryVoltage>minVoltage)&&(batteryVoltage<maxVoltage)) { // If battery voltage is within limits, use it
    tempBatteryVoltage=batteryVoltage;
  } else if (batteryVoltage<=minVoltage) { // If battery voltage is below minimum limit, use minimum
    tempBatteryVoltage=minVoltage;
  } else if (batteryVoltage>=maxVoltage) { // If battery voltage is above maximum limit, use maximum
    tempBatteryVoltage=maxVoltage;
  }

  // Calculate the width of the filled part based on battery voltage
  int filledWidth = ((tempBatteryVoltage-minVoltage)/(maxVoltage-minVoltage)) * gaugeWidth;

  // Draw text for battery
  display.setFont(&FreeMonoBold12pt7b);
  display.setTextColor(GxEPD_BLACK);
  display.setCursor(145, 15);
  display.print("Hranilnik:");

  // Draw the filled part of the battery
  // If battery voltage is above the low voltage threshold, fill the gauge with black, else red
  if (batteryVoltage > lowVoltage) {
    display.fillRect(gaugeX + 1, gaugeY + 1, filledWidth - 2, gaugeHeight - 2, GxEPD_BLACK);
  } else {
    display.fillRect(gaugeX + 1, gaugeY + 1, filledWidth - 2, gaugeHeight - 2, GxEPD_RED);
  }

  // Draw the battery voltage value in volts
  display.setTextColor(GxEPD_BLACK);
  display.setFont(&FreeMonoBold12pt7b);
  display.setCursor(170, 60);
  display.printf("%.1f V", batteryVoltage);
}

// Function to draw the graphics on the e-paper display.
// The function draws the battery gauge, charging mode information, and power gauges for grid, solar, and house.
// The function uses the GxEPD2 library using Adafruit GFX graphics library for drawing shapes and text on the display.
// The function takes no input and returns nothing.
void drawGraphics()
{
  //Initialize the e-paper interface
  display.init(0, true, 2, true); // Initialize the display with full refresh, no partial refresh supported on 3-color displays
  display.setRotation(0); // Landscape orientation
  display.setTextWrap(false); // Disable text wrapping
  
  display.firstPage(); // Start drawing
  do {
    // Clear the screen
    display.fillScreen(GxEPD_WHITE);

    // Draw the battery gauge
    drawBatteryGauge(batteryVoltage);
    
    //Draw charging mode info
    //Draw title for charging mode
    display.setTextColor(GxEPD_BLACK);
    display.setFont(&FreeMonoBold12pt7b);
    display.setCursor(200-(6*14/2), 100);
    display.println("Nacin:");
    //Red color when abnormal mode or off, black otherwise
    if (!normalMode) {
      display.setTextColor(GxEPD_RED);
    }
    //Draw mode strings
    display.setFont(&FreeMonoBold12pt7b);
    display.setCursor(200-(mode1.length()*14/2), 125);
    display.print(mode1);
    display.setCursor(200-(mode2.length()*14/2), 150);
    display.print(mode2);

    //Draw total produced energy today
    display.setTextColor(GxEPD_BLACK);
    display.setFont(&FreeMonoBold12pt7b);
    display.setCursor(103, 185);
    display.printf("Sonce:%.1f kWh", producedEnergy);

    //Draw the left analog gauge
    display.fillCircle(xCLeft, yCLeft, RLeft, GxEPD_RED); //Draw the red circle of the gauge
    display.fillCircle(xCLeft, yCLeft, RLeft-5, GxEPD_BLACK); //Draw the black line inside the red circle
    display.fillCircle(xCLeft, yCLeft, RLeft-10, GxEPD_WHITE); // Delete everything under the black line by drawing a white circle

    //Draw gauge lines per kilowatt (triple lines for added thickness)
    for (int i=0;i<=round(maxGridPower/1000);i++) { // for each kilowatt of the gauge range
      int x = xCLeft + (int) (RLeft*sin(((alphaMaxLeft-alphaMinLeft)/round(maxGridPower/1000)*i)+alphaMinLeft)); // calculate x position of the line
      int y = yCLeft + (int) (RLeft*cos(((alphaMaxLeft-alphaMinLeft)/round(maxGridPower/1000)*i)+alphaMinLeft)); // calculate y position of the line
      display.drawLine(xCLeft, yCLeft, x, y-1, GxEPD_BLACK); // draw first line
      display.drawLine(xCLeft, yCLeft, x, y, GxEPD_BLACK); // draw second line
      display.drawLine(xCLeft, yCLeft, x, y+1, GxEPD_BLACK); // draw third line
    }
    
    display.fillCircle(xCLeft, yCLeft, RLeft-20, GxEPD_WHITE); // Delete everything under the gauge and the lines again
    
    //Draw black numbers on the gauge
    display.setTextColor(GxEPD_BLACK);
    display.setFont(&FreeMonoBold9pt7b);
    for (int i=1;i<round(maxGridPower/1000);i++) { // one number per kilowatt, omit zero and last number
      display.setCursor(xCLeft - 4 + (int) ((float)(RLeft-35)*sin((((alphaMaxLeft-alphaMinLeft)/round(maxGridPower/1000))*i)+alphaMinLeft)), yCLeft + 4 + (int) ((float)(RLeft-35)*cos((((alphaMaxLeft-alphaMinLeft)/round(maxGridPower/1000))*i)+alphaMinLeft))); // calculate position of the number
      display.print(i); // print the number
    }
    
    //Draw gauge legend
    display.setFont(&FreeMonoBold12pt7b);
    display.setCursor(0, 135);
    display.print("Omr");
    //Draw red kW
    display.setCursor(0, 170);
    display.setTextColor(GxEPD_RED);
    display.setFont(&FreeMonoBold18pt7b);
    display.print("kW");

    //Draw the center analog gauge, see comments for left gauge for explanation
    display.fillCircle(xCCenter, yCCenter, RCenter, GxEPD_RED);
    display.fillCircle(xCCenter, yCCenter, RCenter-5, GxEPD_BLACK);
    display.fillCircle(xCCenter, yCCenter, RCenter-10, GxEPD_WHITE);

    //Draw gauge lines per kilowatt (three for added thickness)
    for (int i=0;i<=round(maxSolarPower/1000);i++) {
      int x = xCCenter - (int) (RCenter*cos(((alphaMaxCenter-alphaMinCenter)/round(maxSolarPower/1000)*i)+alphaMinCenter));
      int y = yCCenter - (int) (RCenter*sin(((alphaMaxCenter-alphaMinCenter)/round(maxSolarPower/1000)*i)+alphaMinCenter));
      display.drawLine(xCCenter, yCCenter, x, y-1, GxEPD_BLACK);
      display.drawLine(xCCenter, yCCenter, x, y, GxEPD_BLACK);
      display.drawLine(xCCenter, yCCenter, x, y+1, GxEPD_BLACK);
    }
    //Delete area under gauge
    display.fillCircle(xCCenter, yCCenter, RCenter-20, GxEPD_WHITE);
    
    //Draw numbers
    display.setTextColor(GxEPD_BLACK);
    display.setFont(&FreeMonoBold9pt7b);
    for (int i=1;i<round(maxSolarPower/1000);i++) {
      display.setCursor(xCCenter - 4 - (int) ((float)(RCenter-30)*cos((((alphaMaxCenter-alphaMinCenter)/round(maxSolarPower/1000))*i)+alphaMinCenter)), yCCenter + 4 - (int) ((float)(RCenter-30)*sin((((alphaMaxCenter-alphaMinCenter)/round(maxSolarPower/1000))*i)+alphaMinCenter)));
      display.print(i);
    }
    
    //Draw gauge legend
    display.setFont(&FreeMonoBold18pt7b);
    display.setCursor(110, 299);
    display.print("Sonce ");
    //Draw red kW
    display.setTextColor(GxEPD_RED);
    display.setFont(&FreeMonoBold24pt7b);
    display.print("kW");

    //Draw the right analog gauge, see comments for left gauge for explanation
    display.fillCircle(xCRight, yCRight, RRight, GxEPD_RED);
    display.fillCircle(xCRight, yCRight, RRight-5, GxEPD_BLACK);
    display.fillCircle(xCRight, yCRight, RRight-10, GxEPD_WHITE);

    //Draw gauge lines per kilowatt (three for added thickness)
    for (int i=0;i<=round(maxHousePower/1000);i++) {
      int x = xCRight - (int) (RRight*sin(((alphaMaxRight-alphaMinRight)/round(maxHousePower/1000)*i)+alphaMinRight));
      int y = yCRight + (int) (RRight*cos(((alphaMaxRight-alphaMinRight)/round(maxHousePower/1000)*i)+alphaMinRight));
      display.drawLine(xCRight, yCRight, x, y-1, GxEPD_BLACK);
      display.drawLine(xCRight, yCRight, x, y, GxEPD_BLACK);
      display.drawLine(xCRight, yCRight, x, y+1, GxEPD_BLACK);
    }
    //Delete area under gauge
    display.fillCircle(xCRight, yCRight, RRight-20, GxEPD_WHITE);
    
    //Draw numbers
    display.setTextColor(GxEPD_BLACK);
    display.setFont(&FreeMonoBold9pt7b);
    for (int i=1;i<round(maxHousePower/1000);i++) {
      display.setCursor(xCRight - 5 - (int) ((float)(RRight-23)*sin((((alphaMaxRight-alphaMinRight)/round(maxHousePower/1000))*i)+alphaMinRight)), yCRight + 4 + (int) ((float)(RRight-35)*cos((((alphaMaxRight-alphaMinRight)/round(maxHousePower/1000))*i)+alphaMinRight)));
      display.print(i);
    }
    
    //Draw gauge legend
    display.setFont(&FreeMonoBold12pt7b);
    display.setCursor(340, 135);
    display.print("Hisa");
    //Draw red kW
    display.setCursor(345, 170);
    display.setTextColor(GxEPD_RED);
    display.setFont(&FreeMonoBold18pt7b);
    display.print("kW");
  }
  while (display.nextPage()); // Draw the buffer to display
  
  lastRefresh = millis(); // Update the timestamp of the last screen refresh
  display.powerOff(); // Turn off the display to save power and prevent artefacts
  display.end();  // End the display use
}

// Function for moving the grid power needle on the gauge.
// The function takes a message string from the MQTT subscription and its length as input and moves the needle accordingly.
// The function also checks for limits and refreshes the display as needed.
void gridPowerNeedle(char *message, uint16_t len)
{
  int powerW = atof(message); // Convert the message string to a floating-point number (kilowatts)
  if (powerW>maxGridPower) { // If power is above the maximum limit, set it to the maximum
    powerW=maxGridPower;
  } else if (powerW<0) { // If power is below the minimum limit, set it to zero
    powerW=0;
  }
  
  //Debug messages
  /*Serial.print("Received grid power message: ");
  Serial.println(message);*/
  
  // Move the left servo motor needle based on the power value received from the MQTT subscription
  leftServo.write(leftServoStart - (((float)powerW/maxGridPower) * (leftServoStart-leftServoEnd))); // Move the needle to the corresponding angle
  
  //Debug messages
  /*Serial.print("Servo set to: ");
  Serial.println(leftServo.read());*/
  
  if ((lastRefresh + refreshTime)<millis()) { // If it's time to refresh the screen, refresh it
    drawGraphics();
  }
}

// Function for moving the solar power needle on the gauge.
// See comments for gridPowerNeedle for explanation. No display refresh is needed here.
void solarPowerNeedle(char *message, uint16_t len)
{
  int powerW = atof(message);
  if (powerW>maxSolarPower) {
    powerW=maxSolarPower;
  } else if (powerW<0) {
    powerW=0;
  }
 
  /*Serial.print("Received solar power message: ");
  Serial.println(message);*/
  centerServo.write(centerServoStart - (((float)powerW/maxSolarPower) * (centerServoStart-centerServoEnd)));
  /*Serial.print("Servo set to: ");
  Serial.println(centerServo.read());*/
}

// Function for moving the house power needle on the gauge.
// See comments for solarPowerNeedle for explanation.
void housePowerNeedle(char *message, uint16_t len)
{
  int powerW = atof(message);
  if (powerW>maxHousePower) {
    powerW=maxHousePower;
  } else if (powerW<0) {
    powerW=0;
  }
  /*Serial.print("Received house power message: ");
  Serial.println(message);*/
  rightServo.write(rightServoStart - (((float)powerW/maxHousePower) * (rightServoStart-rightServoEnd)));
  /*Serial.print("Servo set to: ");
  Serial.println(rightServo.read());*/
}

// Function for displaying the mode of operation on the screen.
// The function takes a message string from the MQTT subscription and its length as input and sets the mode strings accordingly.
// Then it refreshes the display.
void modeDisplay(char *message, uint16_t len)
{
  /*Serial.print("Received mode message: ");
  Serial.println(message);*/

  int tempMode = atoi(message);
  switch (tempMode) { // Set the mode strings.
    case 0:
      mode1 = batEmptyString1;
      mode2 = batEmptyString2;
      normalMode = true;
      break;
    case 1:
      mode1 = emergencyString1;
      mode2 = emergencyString2;
      normalMode = false;
      break;
    case 2:
      mode1 = batEmptyString1;
      mode2 = batEmptyString2;
      normalMode = true;
      break;
    case 3:
      mode1 = normalString1;
      mode2 = normalString2;
      normalMode = true;
      break;
    case 4:
      mode1 = fullString1;
      mode2 = fullString2;
      normalMode = true;
      break;
    case 5:
      mode1 = noGridString1;
      mode2 = noGridString2;
      normalMode = false;
      break;
  }

  drawGraphics(); // Refresh the display to show the changed mode strings.
}

// This function takes the value from the MQTT battery voltage subscription and sets the global variable accordingly.
// Displaying the value is done in drawGraphics() only when a display refresh is triggered.
// This prevents the display blinking all the time.
void batteryDisplay(char *message, uint16_t len)
{
  /*Serial.print("Received battery message: ");
  Serial.println(message);*/
  batteryVoltage = atof(message);
}

// This function takes the value from the MQTT energy subscription and sets the global variable accordingly.
// Refresh is same as with batteryDisplay(). 
void energyDisplay(char *message, uint16_t len)
{
  /*Serial.print("Produced energy message: ");
  Serial.println(message);*/
  String msg=message;
  msg.trim();
  producedEnergy = msg.toFloat();
  Serial.println(producedEnergy);
}

// Setup function
void setup()
{
  //Init serial monitoring for debugging
  /*Serial.begin(115200);
  Serial.println();
  Serial.println("setup");
  delay(100);*/
  
  //Servo init
	// Allow allocation of all timers
	ESP32PWM::allocateTimer(0);
	ESP32PWM::allocateTimer(1);
	ESP32PWM::allocateTimer(2);
	ESP32PWM::allocateTimer(3);
  //Servo 1
	leftServo.setPeriodHertz(50);    // standard 50 hz servo
	leftServo.attach(leftServoPin, 540, 2000);
  leftServo.write(leftServoStart);
  //Servo 2
	centerServo.setPeriodHertz(50);    // standard 50 hz servo
	centerServo.attach(centerServoPin, 540, 2000);
  centerServo.write(centerServoStart);
  //Servo 3
	rightServo.setPeriodHertz(50);    // standard 50 hz servo
	rightServo.attach(rightServoPin, 540, 2000);
  rightServo.write(rightServoStart);

  //Init wifi
  initWiFi();

  //Sweep servos to check function and calibration
  for (int value = 0; value<=round(maxGridPower/1000); value++) { // Sweep left and right servos at kilowatt steps
    leftServo.write(leftServoStart - (((float)value/round(maxGridPower/1000)) * (leftServoStart-leftServoEnd)));
    rightServo.write(rightServoStart - (((float)value/round(maxGridPower/1000)) * (rightServoStart-rightServoEnd)));
    delay (500);
  }
  for (int value = 0; value<=round(maxSolarPower/1000); value++) { // Sweep center servo at kilowatt steps
    centerServo.write(centerServoStart - (((float)value/round(maxSolarPower/1000)) * (centerServoStart-centerServoEnd)));
    delay (500);
  }
  
  // Set MQTT callback functions
  gridPowerRef.setCallback(&gridPowerNeedle);
  solarPowerRef.setCallback(&solarPowerNeedle);
  housePowerRef.setCallback(&housePowerNeedle);
  modeRef.setCallback(&modeDisplay);
  batteryRef.setCallback(&batteryDisplay);
  energyRef.setCallback(&energyDisplay);

  // Setup MQTT subscriptions
  mqtt.subscribe(&batteryRef);
  mqtt.subscribe(&energyRef);
  mqtt.subscribe(&modeRef);
  mqtt.subscribe(&gridPowerRef);
  mqtt.subscribe(&solarPowerRef);
  mqtt.subscribe(&housePowerRef);

  //Serial.print("setup done");
}

// Main loop
void loop()
{
  // Connect to MQTT
  connectToMQTTServer();

  // this is our 'wait for incoming subscription packets and callback em' busy subloop
  // try to spend your time here:
  mqtt.processPackets(10000);

  mqtt.ping(); // keep the mqtt connection alive

  if (WiFi.status() != WL_CONNECTED) { // If WiFi connection is lost, restart the ESP32
    ESP.restart();
  }
}
