#ifdef ESP32
  #include <WiFi.h>
  #include <AsyncTCP.h>
#else
  #include <ESP8266WiFi.h>
  #include <ESPAsyncTCP.h>
#endif
#include <ESPAsyncWebServer.h>
#include <Wire.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// REPLACE WITH YOUR NETWORK CREDENTIALS
const char* ssid = "E308";
const char* password = "98806829";

// Default Threshold Temperature Value
String inputMessage = "25.0";
String lastTemperature;
String enableArmChecked = "checked";
String inputMessage2 = "true";
String comfortMessage = "25.0";

// HTML web page to handle 2 input fields (threshold_input, enable_arm_input)
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html><head>
  <title>Temperature Threshold Output Control</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  </head><body>
  <h2>DS18B20 Temperature</h2> 
  <h3>%TEMPERATURE% &deg;C</h3>
  <h2>ESP Arm Trigger</h2>
  <form action="/get">
    Power-Saving Temperature <input type="number" step="0.1" name="PS_INPUT"  required><br>
    
    <input type="submit" value="Submit">
  </form>
    <form action="/get">
    Comfort Temperature<input type="number" step="0.1" name="COMFORT_INPUT"  required><br>
    
    <input type="submit" value="Submit">
  </form>
</body></html>)rawliteral";

void notFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Not found");
}

AsyncWebServer server(80);

// Replaces placeholder with DS18B20 values
String processor(const String& var){
  //Serial.println(var);
  if(var == "TEMPERATURE"){
    return lastTemperature;
  }
  else if(var == "THRESHOLD"){
    return inputMessage;
  }
  else if(var == "ENABLE_ARM_INPUT"){
    return enableArmChecked;
  }
  return String();
}

// Flag variable to keep track if triggers was activated or not
bool triggerActive = false;
String inputParam;

const char* Ps_input = "PS_INPUT";
const char* Comfort_Input = "COMFORT_INPUT";

// Interval between sensor readings. Learn more about ESP32 timers: https://RandomNerdTutorials.com/esp32-pir-motion-sensor-interrupts-timers/
unsigned long previousMillis = 0;     
const long interval = 5000;    

// GPIO where the output is connected to
const int output = 2;

// GPIO where the DS18B20 is connected to
const int oneWireBus = 25;     
// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(oneWireBus);
// Pass our oneWire reference to Dallas Temperature sensor 
DallasTemperature sensors(&oneWire);

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("WiFi Failed!");
    return;
  }
  Serial.println();
  Serial.print("ESP IP Address: http://");
  Serial.println(WiFi.localIP());
  
  pinMode(output, OUTPUT);
  digitalWrite(output, LOW);
  
  // Start the DS18B20 sensor
  sensors.begin();
  
  // Send web page to client
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });

  // Receive an HTTP GET request at <ESP_IP>/get?threshold_input=<inputMessage>&enable_arm_input=<inputMessage2>
  server.on("/get", HTTP_GET, [] (AsyncWebServerRequest *request) {
    // GET threshold_input value on <ESP_IP>/get?threshold_input=<inputMessage>
    if (request->hasParam(Ps_input) ) {
      inputMessage = request->getParam(Ps_input)->value();
      inputParam= Ps_input;

    }
      // GET enable_arm_input value on <ESP_IP>/get?enable_arm_input=<inputMessage2>
     else if  (request->hasParam(Comfort_Input)) {
        inputMessage= request->getParam(Comfort_Input)->value();
        inputParam== Comfort_Input;
        
      }
      else {
        inputMessage2 = "false";
        enableArmChecked = "";
      
    }
    Serial.println(inputMessage);
    Serial.println(inputMessage2);
    request->send(200, "text/html", "Temperature of " + inputParam+ " is " + inputMessage+ "<br><br><a href=\"/\">Return to Home Page</a>");
  });
  server.onNotFound(notFound);
  server.begin();
}

void loop() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    sensors.requestTemperatures();
    // Temperature in Celsius degrees 
    float temperature = sensors.getTempCByIndex(0);
    Serial.print(temperature);
    Serial.println(" *C");
    
    // Temperature in Fahrenheit degrees
    /*float temperature = sensors.getTempFByIndex(0);
    Serial.print(temperature);
    Serial.println(" *F");*/
    
    lastTemperature = String(temperature);
    
    // Check if temperature is above threshold and if it needs to trigger output
    if(temperature > inputMessage.toInt()){
      String message = String("Temperature above threshold. Current temperature: ") + 
                            String(temperature) + String("C");
      Serial.println(message);
      triggerActive = true;
      digitalWrite(output, HIGH);
    }
    // Check if temperature is below threshold and if it needs to trigger output
    else if((temperature < inputMessage.toInt()) ) {
      String message = String("Temperature below threshold. Current temperature: ") + 
                            String(temperature) + String(" C");
      Serial.println(message);
      triggerActive = false;
      digitalWrite(output, LOW);
    }
  }
}