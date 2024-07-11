/*********
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/esp32-esp8266-web-server-physical-button/
  
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
*********/

// Import required libraries
#ifdef ESP32
  #include <WiFi.h>
  #include <AsyncTCP.h>
#else
  #include <ESP8266WiFi.h>
  #include <ESPAsyncTCP.h>
#endif
#include <ESPAsyncWebServer.h>
#include <Preferences.h>

int result[5] = {0, 0, 0, 0, 0};
int gameState = 1; //'1' ready, '0' result;

// Replace with your network credentials

const char* PARAM_INPUT_1 = "output";
const char* PARAM_INPUT_2 = "state";
const char* PARAM_INPUT_3 = "team";
const char* PARAM_INPUT_4 = "ssid";
const char* PARAM_INPUT_5 = "pass";

const int output1 = 13;     //related with the White button
const int buttonPin1 = 25;  
const int output2 = 14;     //related with the Blue button
const int buttonPin2 = 32;  
const int output3 = 15;     //related with the Green button
const int buttonPin3 = 33;
const int output4 = 26;     //related with the Red button
const int buttonPin4 = 34;
const int output5 = 27;     //related with the Yellow button
const int buttonPin5 = 35;


int points[5] = {0, 0, 0, 0, 0};

// Variables will change:
int ledState1 = HIGH;          // the current state of the output pin
int ledState2 = HIGH;          // the current state of the output pin
int ledState3 = HIGH;          // the current state of the output pin
int ledState4 = HIGH;          // the current state of the output pin
int ledState5 = HIGH;          // the current state of the output pin
int buttonState;             // the current reading from the input pin
int lastButtonState = LOW;   // the previous reading from the input pin

// the following variables are unsigned longs because the time, measured in
// milliseconds, will quickly become a bigger number than can be stored in an int.
unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 50;    // the debounce time; increase if the output flickers

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);
Preferences preferences;

const char credentials_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <title>Credenciales</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    html {font-family: Arial; display: inline-block; text-align: center;}
    h2 {font-size: 3.0rem;}
    h3 {font-size: 2.0rem;}
    p {font-size: 3.0rem;}
    body {max-width: 600px; margin:0px auto; padding-bottom: 25px;}
  </style>
</head>

<body>
<h3>Configuration AP</h3><br>
<form action='' method='POST'>WiFi connection failed. Enter valid parameters, please.<br><br>
SSID:<br><input type='text' name='ssid'><br>
Password:<br><input type='password' name='pass'><br><br>
<input type='submit' value='Submit'></form>

  %BUTTONPLACEHOLDER%
</body>
</html>
)rawliteral";

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <title>ESP Web Server</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    html {font-family: Arial; display: inline-block; text-align: center;}
    h2 {font-size: 3.0rem;}
    h3 {font-size: 2.0rem;}
    p {font-size: 3.0rem;}
    body {max-width: 600px; margin:0px auto; padding-bottom: 25px;}
    .container{display: flex; margin:0px auto; padding-bottom: 25px;}
    .switch {position: relative; display: inline-block; width: 300px; height: 95px} 
    .switch input {display: none}
    .slider1 {position: absolute; top: 0; left: 0; right: 0; bottom: 0; background-color: #000000; border-radius: 34px}
    .slider1:before {position: absolute; content: ""; height: 80px; width: 80px; left: 8px; bottom: 8px; background-color: #fff; -webkit-transition: .4s; transition: .4s; border-radius: 68px}
    .slider2 {position: absolute; top: 0; left: 0; right: 0; bottom: 0; background-color: #4d8be8; border-radius: 34px}
    .slider2:before {position: absolute; content: ""; height: 80px; width: 80px; left: 8px; bottom: 8px; background-color: #fff; -webkit-transition: .4s; transition: .4s; border-radius: 68px}
    .slider3 {position: absolute; top: 0; left: 0; right: 0; bottom: 0; background-color: #51d44c; border-radius: 34px}
    .slider3:before {position: absolute; content: ""; height: 80px; width: 80px; left: 8px; bottom: 8px; background-color: #fff; -webkit-transition: .4s; transition: .4s; border-radius: 68px}
    .slider4 {position: absolute; top: 0; left: 0; right: 0; bottom: 0; background-color: #c94444; border-radius: 34px}
    .slider4:before {position: absolute; content: ""; height: 80px; width: 80px; left: 8px; bottom: 8px; background-color: #fff; -webkit-transition: .4s; transition: .4s; border-radius: 68px}
    .slider5 {position: absolute; top: 0; left: 0; right: 0; bottom: 0; background-color: #ebeb23; border-radius: 34px}
    .slider5:before {position: absolute; content: ""; height: 80px; width: 80px; left: 8px; bottom: 8px; background-color: #fff; -webkit-transition: .4s; transition: .4s; border-radius: 68px}
    .sliderReady {position: absolute; top: 0; left: 0; right: 0; bottom: 0; background-color: #7e36e3; border-radius: 34px}
    .sliderReady:before {position: absolute; content: ""; height: 80px; width: 80px; left: 8px; bottom: 8px; background-color: #fff; -webkit-transition: .4s; transition: .4s; border-radius: 68px}

    input:checked+.slider1 {background-color: #000000}
    input:checked+.slider1:before {-webkit-transform: translateX(200px); -ms-transform: translateX(200px); transform: translateX(200px)}
    input:checked+.slider2 {background-color: #4d8be8}
    input:checked+.slider2:before {-webkit-transform: translateX(200px); -ms-transform: translateX(200px); transform: translateX(200px)}
    input:checked+.slider3 {background-color: #51d44c}
    input:checked+.slider3:before {-webkit-transform: translateX(200px); -ms-transform: translateX(200px); transform: translateX(200px)}
    input:checked+.slider4 {background-color: #c94444}
    input:checked+.slider4:before {-webkit-transform: translateX(200px); -ms-transform: translateX(200px); transform: translateX(200px)}
    input:checked+.slider5 {background-color: #ebeb23}
    input:checked+.slider5:before {-webkit-transform: translateX(200px); -ms-transform: translateX(200px); transform: translateX(200px)}
    input:checked+.sliderReady {background-color: #7e36e3}
    input:checked+.sliderReady:before {-webkit-transform: translateX(200px); -ms-transform: translateX(200px); transform: translateX(200px)}
  </style>

  
</head>
<body>
  <h2>ESP Web Server</h2>

  <div class="container">
  %BUTTONPLACEHOLDER%
  </div>
<script>
function toggleCheckbox(element) {
  var xhr = new XMLHttpRequest();
  if(element.checked){ xhr.open("GET", "/update?output="+element.id+"&state=1", true); }
  else { xhr.open("GET", "/update?output="+element.id+"&state=0", true); }
  xhr.send();
}

function toggleCheckboxReady(element) {
  var xhr = new XMLHttpRequest();
  if(element.checked){ xhr.open("GET", "/ready?output="+element.id+"&state=1", true); }
  else { xhr.open("GET", "/ready?output="+element.id+"&state=0", true); }
  xhr.send();
}

</script>

<script>
document.addEventListener('keydown', function(event) {
    handleKeyPress(event);
    
});

function handleKeyPress(event) {
    const title = document.getElementById('equipoBlanco');
    var xhr = new XMLHttpRequest();
    // Example: alt + 1
    if (event.altKey && event.key === '1') {
      var puntosBlanco = document.getElementById("outputState13").innerHTML;
      document.getElementById("outputState13").innerHTML = Number(puntosBlanco) - 1;
      xhr.open("GET", "/dec?team=1", true)
      xhr.send();
    }

    // Example: Alt + 2
    if (event.altKey && event.key === '2') {
      var puntosAzul = document.getElementById("outputState14").innerHTML;
      document.getElementById("outputState14").innerHTML = Number(puntosAzul) - 1;
      xhr.open("GET", "/dec?team=2", true)
      xhr.send();
    }

    // Example: alt + 3
    if (event.altKey && event.key === '3') {
      var puntosVerde = document.getElementById("outputState15").innerHTML;
      document.getElementById("outputState15").innerHTML = Number(puntosVerde) - 1;
      xhr.open("GET", "/dec?team=3", true)
      xhr.send();
    }
    // Example: alt + 4
    if (event.altKey && event.key === '4') {
      var puntosRojo = document.getElementById("outputState26").innerHTML;
      document.getElementById("outputState26").innerHTML = Number(puntosRojo) - 1;
      xhr.open("GET", "/dec?team=4", true)
      xhr.send();
    }
    // Example: alt + 5
    if (event.altKey && event.key === '5') {
      var puntosAmarillo = document.getElementById("outputState27").innerHTML;
      document.getElementById("outputState27").innerHTML = Number(puntosAmarillo) - 1;
      xhr.open("GET", "/dec?team=5", true)
      xhr.send();
    }
    // Example: 1
    if (!event.altKey && event.key === '1') {
      var puntosBlanco = document.getElementById("outputState13").innerHTML;
      document.getElementById("outputState13").innerHTML = Number(puntosBlanco) + 1;
      xhr.open("GET", "/inc?team=1", true)
      xhr.send();
    }

    // Example: 2
    if (!event.altKey && event.key === '2') {
      var puntosAzul = document.getElementById("outputState14").innerHTML;
      document.getElementById("outputState14").innerHTML = Number(puntosAzul) + 1;
      xhr.open("GET", "/inc?team=2", true)
      xhr.send();
    }

    // Example: 3
    if (!event.altKey && event.key === '3') {
      var puntosVerde = document.getElementById("outputState15").innerHTML;
      document.getElementById("outputState15").innerHTML = Number(puntosVerde) + 1;
      xhr.open("GET", "/inc?team=3", true)
      xhr.send();
    }
    // Example: 4
    if (!event.altKey && event.key === '4') {
      var puntosRojo = document.getElementById("outputState26").innerHTML;
      document.getElementById("outputState26").innerHTML = Number(puntosRojo) + 1;
      xhr.open("GET", "/inc?team=4", true)
      xhr.send();
    }
    // Example: 5
    if (!event.altKey && event.key === '5') {
      var puntosAmarillo = document.getElementById("outputState27").innerHTML;
      document.getElementById("outputState27").innerHTML = Number(puntosAmarillo) + 1;
      xhr.open("GET", "/inc?team=5", true)
      xhr.send();
    }
}
</script>

<script>
setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      var inputChecked;
      var outputStateM;
      var posicion = ['posicionBlanco', 'posicionAzul', 'posicionVerde', 'posicionRojo', 'posicionAmarillo'];

      console.log(this.responseText.split(","));
      var response = this.responseText.split(",");
      if( response[0] == 0 || response[1] == 0 || response[2] == 0 || response[3] == 0 || response[4] == 0 ){
        document.getElementById("readyButton").innerHTML = "Waiting<br\><br\>";
      }
      else{
        for(var i = 0;i < 5; i++){
        document.getElementById(posicion[i]).innerHTML = 0;
        }
        document.getElementById("readyButton").innerHTML = "Ready<br\><br\>";
      }

      if( response[0] == 1){ 
        inputChecked = true;
        outputStateM = "On";
      }
      else { 
        inputChecked = false;
        outputStateM = "Off";
      }
      document.getElementById("13").checked = inputChecked;

      if( response[1] == 1){ 
        inputChecked = true;
        outputStateM = "On";
      }
      else { 
        inputChecked = false;
        outputStateM = "Off";
      }
      document.getElementById("14").checked = inputChecked;
      
      
      if( response[2] == 1){ 
        inputChecked = true;
        outputStateM = "On";
      }
      else { 
        inputChecked = false;
        outputStateM = "Off";
      }
      document.getElementById("15").checked = inputChecked;
      
      
      
      if( response[3] == 1){ 
        inputChecked = true;
        outputStateM = "On";
      }
      else { 
        inputChecked = false;
        outputStateM = "Off";
      }
      document.getElementById("26").checked = inputChecked;
      
      
      
      if( response[4] == 1){ 
        inputChecked = true;
        outputStateM = "On";
      }
      else { 
        inputChecked = false;
        outputStateM = "Off";
      }
      document.getElementById("27").checked = inputChecked;
      document.getElementById("outputState13").innerHTML = response[10];
      document.getElementById("outputState14").innerHTML = response[11];
      document.getElementById("outputState15").innerHTML = response[12];
      document.getElementById("outputState26").innerHTML = response[13];
      document.getElementById("outputState27").innerHTML = response[14];

      
      
      for(var i = 0;i < 5; i++){
        if(response[i+5] > 0){
          document.getElementById(posicion[response[i + 5]-1]).innerHTML = i + 1;
        }
      }
      
    }
  };
  xhttp.open("GET", "/state", true);
  xhttp.send();
}, 1000 ) ;
</script>

</body>
</html>
)rawliteral";



// Replaces placeholder with button section in your web page
String processor(const String& var){
  //Serial.println(var);
  if(var == "BUTTONPLACEHOLDER"){
    String buttons ="";
    buttons += "<div id=\"divButtons\">";
    buttons += "<h3>Blanco <span id=\"posicionBlanco\"> 0 </span> <br\> Puntos:  <span id=\"outputState13\"> 0 </span></h3><label class=\"switch\"><input type=\"checkbox\" onchange=\"toggleCheckbox(this)\" id=\"13\" " + outputState(13) + "><span class=\"slider slider1\"></span></label>";
    buttons += "<h3>Azul <span id=\"posicionAzul\"> 0 </span> <br\> Puntos:  <span id=\"outputState14\"> 0 </span></h3><label class=\"switch\"><input type=\"checkbox\" onchange=\"toggleCheckbox(this)\" id=\"14\" " + outputState(14) + "><span class=\"slider slider2\"></span></label>";
    buttons += "</div>";
    buttons += "<div>";
    buttons += "<h3>Verde <span id=\"posicionVerde\"> 0 </span> <br\> Puntos:  <span id=\"outputState15\"> 0 </span></h3><label class=\"switch\"><input type=\"checkbox\" onchange=\"toggleCheckbox(this)\" id=\"15\" " + outputState(15) + "><span class=\"slider slider3\"></span></label>";
    buttons += "<h3>Rojo <span id=\"posicionRojo\"> 0 </span> <br\> Puntos:  <span id=\"outputState26\"> 0 </span></h3><label class=\"switch\"><input type=\"checkbox\" onchange=\"toggleCheckbox(this)\" id=\"26\" " + outputState(26) + "><span class=\"slider slider4\"></span></label>";
    buttons += "</div>";
    buttons += "<div>";
    buttons += "<h3>Amarillo <span id=\"posicionAmarillo\"> 0 </span> <br\> Puntos:  <span id=\"outputState27\"> 0 </span></h3><label class=\"switch\"><input type=\"checkbox\" onchange=\"toggleCheckbox(this)\" id=\"27\" " + outputState(27) + "><span class=\"slider slider5\"></span></label>";
    buttons += "<h3 id=\"readyButton\">Ready<br\><br\><span id=\"outputReady\"></span></h3><label class=\"switch\"><input type=\"checkbox\" onchange=\"toggleCheckboxReady(this)\" id=\"ready\" " + String("Ready") + "><span class=\"slider sliderReady\"></span></label>";
    buttons += "</div>";
    return buttons;
  }
  return String();
}

// Replaces placeholder with button section in your web page
String processorCredentials(const String& var){
  //Serial.println(var);
  if(var == "BUTTONPLACEHOLDER"){
    String buttons ="";
    return buttons;
  }
  return String();
}

String outputState(int value){
  if(digitalRead(value)){
    return "checked";
  }
  else {
    return "";
  }
  return "";
}


void IRAM_ATTR interruptHndlerButton1() {
    gameState = 0;
    digitalWrite(output1, LOW);
    for(int i = 0; i < 5; i++){
      if(result[i] == 0){
        result[i] = 1;
        detachInterrupt(buttonPin1);
        break;
      }
    }
}

void IRAM_ATTR interruptHndlerButton2() {
    digitalWrite(output2, LOW);
    for(int i = 0; i < 5; i++){
      if(result[i] == 0){
        result[i] = 2;
        detachInterrupt(buttonPin2);
        break;
      }
    }
}

void IRAM_ATTR interruptHndlerButton3() {
    digitalWrite(output3, LOW);
    for(int i = 0; i < 5; i++){
      if(result[i] == 0){
        result[i] = 3;
        detachInterrupt(buttonPin3);
        break;
      }
    }
}

void IRAM_ATTR interruptHndlerButton4() {
    digitalWrite(output4, LOW);
    for(int i = 0; i < 5; i++){
      if(result[i] == 0){
        result[i] = 4;
        detachInterrupt(buttonPin4);
        break;
      }
    }
}

void IRAM_ATTR interruptHndlerButton5() {
    digitalWrite(output5, LOW);
    for(int i = 0; i < 5; i++){
      if(result[i] == 0){
        result[i] = 5;
        detachInterrupt(buttonPin5);
        break;
      }
    }
}

void configAP() {

  //WiFiServer configWebServer(80);
  // Create AsyncWebServer object on port 80
  

  WiFi.mode(WIFI_AP_STA); // starts the default AP (factory default or setup as persistent)
  
  Serial.print("Connect your computer to the WiFi network ");
#ifdef ESP32
  Serial.print("to SSID of you ESP32"); // no getter for SoftAP SSID
#else
  Serial.print(WiFi.softAPSSID());
#endif
  Serial.println();
  IPAddress ip = WiFi.softAPIP();
  Serial.print("and enter http://");
  Serial.print(ip);
  Serial.println(" in a Web browser");
}

void setup(){
  // Serial port for debugging purposes
  Serial.begin(115200);

  pinMode(output1, OUTPUT);
  digitalWrite(output1, HIGH);
  pinMode(buttonPin1, INPUT);

  pinMode(output2, OUTPUT);
  digitalWrite(output2, HIGH);
  pinMode(buttonPin2, INPUT);
  
  pinMode(output3, OUTPUT);
  digitalWrite(output3, HIGH);
  pinMode(buttonPin3, INPUT);
  
  pinMode(output4, OUTPUT);
  digitalWrite(output4, HIGH);
  pinMode(buttonPin4, INPUT);
  
  pinMode(output5, OUTPUT);
  digitalWrite(output5, HIGH);
  pinMode(buttonPin5, INPUT);
  
  

 

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });

   // Iniciar las preferencias
  preferences.begin("my-app", false); // "my-app" es el namespace, false es para lectura/escritura

  String ssid = preferences.getString("ssid", "noSSIDStored");
  String pass = preferences.getString("pass", "noPassStored");

  Serial.println(ssid);
  Serial.println(pass);
  // Connect to Wi-Fi
  WiFi.begin(ssid, pass);
  // while (WiFi.status() != WL_CONNECTED) {
  //   delay(1000);
  //   Serial.println("Connecting to WiFi..");
  // }

  // waiting for connection to remembered  Wifi network
  Serial.println("Waiting for connection to WiFi");
  WiFi.waitForConnectResult(10000);

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println();
    Serial.println("Could not connect to WiFi. Starting configuration AP...");
    configAP();
  } else {
    Serial.println("WiFi connected");
    // Print ESP Local IP Address
    Serial.println(WiFi.localIP());
  }

  // Route for root / web page
  server.on("/connect", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", credentials_html, processorCredentials);
  });

  server.on("/connect", HTTP_POST, [] (AsyncWebServerRequest *request) {
    String inputMessage1;
    String inputMessage2;
    // GET input1 value on <ESP_IP>/update?output=<inputMessage1>&state=<inputMessage2>
    if (request->hasParam(PARAM_INPUT_4, true)&&request->hasParam(PARAM_INPUT_5, true)) {
      inputMessage1 = request->getParam(PARAM_INPUT_4, true)->value();
      inputMessage2 = request->getParam(PARAM_INPUT_5, true)->value();
    }
    Serial.print("SSID: ");
    Serial.print(inputMessage1);
    Serial.print("PASS: ");
    Serial.print(inputMessage2);
    preferences.putString("ssid", inputMessage1);
    preferences.putString("pass", inputMessage2);
    esp_restart();
    request->redirect("/");
    
  });

   // Route for root / web page
  server.on("/ready", HTTP_GET, [](AsyncWebServerRequest *request){
    attachInterrupt(buttonPin1, interruptHndlerButton1, FALLING);
    attachInterrupt(buttonPin2, interruptHndlerButton2, FALLING);
    attachInterrupt(buttonPin3, interruptHndlerButton3, FALLING);
    attachInterrupt(buttonPin4, interruptHndlerButton4, FALLING);
    attachInterrupt(buttonPin5, interruptHndlerButton5, FALLING);
    digitalWrite(output1, HIGH);
    digitalWrite(output2, HIGH);
    digitalWrite(output3, HIGH);
    digitalWrite(output4, HIGH);
    digitalWrite(output5, HIGH);
    for(int i = 0; i < 5; i++){
      result[i] = 0;
    }
    request->send(200, "text/plain", "Juego Ready");
  });

  // Send a GET request to <ESP_IP>/update?output=<inputMessage1>&state=<inputMessage2>
  server.on("/update", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String inputMessage1;
    String inputMessage2;
    // GET input1 value on <ESP_IP>/update?output=<inputMessage1>&state=<inputMessage2>
    if (request->hasParam(PARAM_INPUT_1) && request->hasParam(PARAM_INPUT_2)) {
      inputMessage1 = request->getParam(PARAM_INPUT_1)->value();
      inputMessage2 = request->getParam(PARAM_INPUT_2)->value();
      digitalWrite(inputMessage1.toInt(), inputMessage2.toInt());
      
      
      if(inputMessage1.toInt()==1){
        ledState1 = !ledState1;  
      }
      else if(inputMessage1.toInt()==2){
        ledState2 = !ledState2;
      }
      else if(inputMessage1.toInt()==3){
        ledState3 = !ledState3;
      }
      else if(inputMessage1.toInt()==4){
        ledState4 = !ledState4;
      }
      else if(inputMessage1.toInt()==5){
        ledState5 = !ledState5;
      }
    }
    else {
      inputMessage1 = "No message sent";
      inputMessage2 = "No message sent";
    }
    Serial.print("GPIO: ");
    Serial.print(inputMessage1);
    Serial.print(" - Set to: ");
    Serial.println(inputMessage2);
    request->send(200, "text/plain", "OK");
  });

  server.on("/inc", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String inputMessage1;
    // GET input1 value on <ESP_IP>/update?output=<inputMessage1>&state=<inputMessage2>
    if (request->hasParam(PARAM_INPUT_3)) {
      inputMessage1 = request->getParam(PARAM_INPUT_3)->value();
      
      if(inputMessage1.toInt()==1){
        points[0] = points[0] + 1; 
      }
      else if(inputMessage1.toInt()==2){
        points[1] = points[1] + 1;
      }
      else if(inputMessage1.toInt()==3){
        points[2] = points[2] + 1;
      }
      else if(inputMessage1.toInt()==4){
        points[3] = points[3] + 1;
      }
      else if(inputMessage1.toInt()==5){
        points[4] = points[4] + 1;
      }
    }
    else {
      inputMessage1 = "No message sent";
    }
    Serial.print("Inc Team: ");
    Serial.print(inputMessage1);
    request->send(200, "text/plain", "OK");
  });



  server.on("/dec", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String inputMessage1;
    // GET input1 value on <ESP_IP>/update?output=<inputMessage1>&state=<inputMessage2>
    if (request->hasParam(PARAM_INPUT_3)) {
      inputMessage1 = request->getParam(PARAM_INPUT_3)->value();
      
      if(inputMessage1.toInt()==1){
        if(points[0] > 0){
          points[0] = points[0] - 1;
        }
      }
      else if(inputMessage1.toInt()==2){
        if(points[1] > 0){
          points[1] = points[1] - 1;
        }      }
      else if(inputMessage1.toInt()==3){
        if(points[2] > 0){
          points[2] = points[2] - 1;
        }
      }
      else if(inputMessage1.toInt()==4){
        if(points[3] > 0){
          points[3] = points[3] - 1;
        }
      }
      else if(inputMessage1.toInt()==5){
        if(points[4] > 0){
          points[4] = points[4] - 1;
        }
      }
    }
    else {
      inputMessage1 = "No message sent";
    }
    Serial.print("Inc Team: ");
    Serial.print(inputMessage1);
    request->send(200, "text/plain", "OK");
  });



  // Send a GET request to <ESP_IP>/state
  server.on("/state", HTTP_GET, [] (AsyncWebServerRequest *request) {
    int state[5] = {digitalRead(output1), digitalRead(output2), digitalRead(output3), digitalRead(output4), digitalRead(output5)};
    String stateStr = "";


    //construiremos un mensaje de la siguiente característica
    //[1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
    //los primeros 5 son el estado actual de las salidas
    //los 5 siguietnes representan el orden en que se presionaron los pulsadores
    //El tercero almacena los puntos acumulados por cada equipo

    for (int i = 0; i < 5; i++) {
      stateStr += String(state[i]);
      if (i < 4) {
          stateStr += ",";  // Agrega una coma y un espacio entre los números
      }
    }
    stateStr += ",";
    for (int i = 0; i < 5; i++) {
      stateStr += String(result[i]);
      if (i < 4) {
          stateStr += ",";  // Agrega una coma y un espacio entre los números
      }
    }
    stateStr += ",";
    for (int i = 0; i < 5; i++) {
      stateStr += String(points[i]);
      if (i < 4) {
          stateStr += ",";  // Agrega una coma y un espacio entre los números
      }
    }
    
    request->send(200, "text/plain", stateStr);
  });
  // Start server

  attachInterrupt(buttonPin1, interruptHndlerButton1, FALLING);
  attachInterrupt(buttonPin2, interruptHndlerButton2, FALLING);
  attachInterrupt(buttonPin3, interruptHndlerButton3, FALLING);
  attachInterrupt(buttonPin4, interruptHndlerButton4, FALLING);
  attachInterrupt(buttonPin5, interruptHndlerButton5, FALLING);

  server.begin();
}
  
void loop() {
  // read the state of the switch into a local variable:
  // int reading = digitalRead(buttonPin1);

  // // check to see if you just pressed the button
  // // (i.e. the input went from LOW to HIGH), and you've waited long enough
  // // since the last press to ignore any noise:

  // // If the switch changed, due to noise or pressing:
  // if (reading != lastButtonState) {
  //   // reset the debouncing timer
  //   lastDebounceTime = millis();
  // }

  // if ((millis() - lastDebounceTime) > debounceDelay) {
  //   // whatever the reading is at, it's been there for longer than the debounce
  //   // delay, so take it as the actual current state:

  //   // if the button state has changed:
  //   if (reading != buttonState) {
  //     buttonState = reading;

  //     // only toggle the LED if the new button state is HIGH
  //     // if (buttonState == HIGH) {
  //     //   ledState = !ledState;
  //     // }
  //   }
  // }

  // // set the LED:
  // digitalWrite(output1, ledState1); 
  // digitalWrite(output2, ledState2);
  // digitalWrite(output3, ledState3);
  // digitalWrite(output4, ledState4);
  // digitalWrite(output5, ledState5);

  // // save the reading. Next time through the loop, it'll be the lastButtonState:
  // lastButtonState = reading;
}