#include <Arduino.h>
/*********
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com
*********/

// Import required libraries
#ifdef ESP32
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#else
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Hash.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#endif
#include <OneWire.h>
#include <DallasTemperature.h>

// Data wire is connected to GPIO 25
#define ONE_WIRE_BUS 25

// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature sensor
DallasTemperature sensors(&oneWire);

// Variables to store temperature values
String temperatureF = "";
String temperatureC = "";

int inputComfort = 0;
int inputPS = 0;

// Timer variables
unsigned long lastTime = 0;
unsigned long timerDelay = 1000;

// Replace with your network credentials
const char *ssid = "E308";
const char *password = "98806829";

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

String readDSTemperatureC()
{
  // Call sensors.requestTemperatures() to issue a global temperature and Requests to all devices on the bus
  sensors.requestTemperatures();
  float tempC = sensors.getTempCByIndex(0);

  if (tempC == -127.00)
  {
    Serial.println("Failed to read from DS18B20 sensor");
    return "--";
  }
  else
  {
    Serial.print("Temperature Celsius: ");
    Serial.println(tempC);
  }
  return String(tempC);
}

String readDSTemperatureF()
{
  // Call sensors.requestTemperatures() to issue a global temperature and Requests to all devices on the bus
  sensors.requestTemperatures();
  float tempF = sensors.getTempFByIndex(0);

  if (int(tempF) == -196)
  {
    Serial.println("Failed to read from DS18B20 sensor");
    return "--";
  }
  else
  {
    Serial.print("Temperature Fahrenheit: ");
    Serial.println(tempF);
  }
  return String(tempF);
}

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="stylesheet" href="https://use.fontawesome.com/releases/v5.7.2/css/all.css" integrity="sha384-fnmOCqbTlWIlj8LyTjo7mOUStjsKC4pOpQbqyi7RrhN7udi9RwhKkMHpvLbHG9Sr" crossorigin="anonymous">
  <style>
    html {
     font-family: Arial;
     display: inline-block;
     margin: 0px auto;
     text-align: center;
    }
    h2 { font-size: 3.0rem; }
    p { font-size: 3.0rem; }
    .units { font-size: 1.2rem; }
    .ds-labels{
      font-size: 1.5rem;
      vertical-align:middle;
      padding-bottom: 15px;
    }
  </style>
</head>
<body>
  <h2>ESP DS18B20 Server</h2>
  <p>
    <i class="fas fa-thermometer-half" style="color:#059e8a;"></i> 
    <span class="ds-labels">Temperature Celsius</span> 
    <span id="temperaturec">%TEMPERATUREC%</span>
    <sup class="units">&deg;C</sup>
  </p>
  <p>
    <i class="fas fa-thermometer-half" style="color:#059e8a;"></i> 
    <span class="ds-labels">Temperature Fahrenheit</span>
    <span id="temperaturef">%TEMPERATUREF%</span>
    <sup class="units">&deg;F</sup>
  </p>


 <label for=input1>
  Comfort:
 </label>
  <br>
  <input type="number" id="input1" min="0"><br>
  
   %inputComfort%

  <br>
  <label>
  Power-Saving: 
  </label>
  <br>
  <input type="number" id="input2" min="0">
  %inputPS%

  <br>
  <br>

  <button onclick="sendData()">Send</button>

 <br>
 <br>

  <label>Output1</label>
  <input type="number" id="Output1" readonly>

<br>
<br>

  <label>Output2</label>
  <input type="number" id="Output2" readonly>

</body>
<script>
setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("temperaturec").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/temperaturec", true);
  xhttp.send();
}, 10000) ;
setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("temperaturef").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/temperaturef", true);
  xhttp.send();
}, 10000) ;


function sendData(){
  inputComfort = document.getElementById("input1")
  inputPS = document.getElementById("input2")

  if (inputComfort.value > 0){
      sendComfort();
  }
  else
  {
  alert("The Comfort field is empty")
  }

  if(inputPS.value > 0){
    sendPS();
  }
  else{
    alert("The PowerSaving field is empty")
  }
}

function sendComfort() {
  const xhr = new XMLHttpRequest();
  xhr.onreadystatechange = function() {
    if(this.readystate === 4 && this.status === 200){
      document.getElementById("Output1").value = this.responseText;
    }
  };
  xhr.open("POST", "text/html", index_html);
  xhr.setRequestHeader("Content-type", "application/x-www-form-urlencoded");
  const data = "Outpu1=" + encodeURIComponent(document.getElementById("Output2").value);
  xhr.send(data);
}

function sendPS() {
  const xhr = new XMLHttpRequest();
  xhr.onreadystatechange = function() {
    if (this.readyState === 4 && this.status === 200) {
      document.getElementById("Output2").value = this.responseText;
    }
  };
  xhr.open("POST", "text/html", index_html);
  xhr.setRequestHeader("Content-type", "application/x-www-form-urlencoded");
  const data = "Output2=" + encodeURIComponent(document.getElementById("Output2").value);
  xhr.send(data);
}
</script>
</html>)rawliteral";

// Replaces placeholder with DS18B20 values
String processor(const String &var)
{
  // Serial.println(var);
  if (var == "TEMPERATUREC")
  {
    return temperatureC;
  }
  else if (var == "TEMPERATUREF")
  {
    return temperatureF;
  }
  return String();
}

void setup()
{
  // Serial port for debugging purposes
  Serial.begin(115200);
  Serial.println();

  // Start up the DS18B20 library
  sensors.begin();

  temperatureC = readDSTemperatureC();
  temperatureF = readDSTemperatureF();

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  Serial.println("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  // Print ESP Local IP Address
  Serial.println(WiFi.localIP());

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send_P(200, "text/html", index_html, processor); });
  server.on("/temperaturec", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send_P(200, "text/plain", temperatureC.c_str()); });
  server.on("/temperaturef", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send_P(200, "text/plain", temperatureF.c_str()); });
      // Start server
      server.begin();
}

void loop()
{
      if ((millis() - lastTime) > timerDelay)
      {
        temperatureC = readDSTemperatureC();
        temperatureF = readDSTemperatureF();
        lastTime = millis();
      }
}


// sæt input til normal/komfort mode for at sætte værdi for når den er valgt
//sæt input til spare mode for at sætte værdi for når den er valgt
// du sætter grænsen for både normal/komfort og spare, ved at indtaste værdierne du ønsker for begge. altså minimum og maximum.