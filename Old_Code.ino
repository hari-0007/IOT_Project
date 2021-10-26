#include <DHT.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266WebServerSecure.h>
#include <ESP8266mDNS.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
//#include <Bounce2.h>
#include <TimeLib.h>
#include <ArduinoOTA.h>
//#include <WiFiManager.h>
#include <umm_malloc/umm_malloc.h>
#include <umm_malloc/umm_heap_select.h>

extern "C" {
#include<user_interface.h>
}

#define heatPin 13
#define humPin 1
#define rotatorPin1 4
#define rotatorPin2 12
#define rotatorPinOpp 5
#define AP_HIDDEN true
#define sw 3
#define DHTTYPE DHT22
DHT dht(14, DHTTYPE);

const byte ledPin = 15;

int BAT= A0;              //Analog channel A0 as used to measure battery voltage
float RatioFactor=3.34;

//uint8_t Pwm1 = D1;
//uint8_t Pwm2 = D2;

float hum;
float temp;
float f;
char temp_str[6];
char hum_str[6];
char f_str[6];
char b_str[6];

float battery;
float Vvalue=0.0,Rvalue=0.0;
  
String request = "";

WiFiClient client;

WiFiEventHandler gotIpEventHandler, disconnectedEventHandler;

unsigned long preMillis = 0;
const long period = 100;

int lastButtonState = HIGH;   // the previous reading from the input pin
unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 50;    // the debounce time; increase if the output flickers
bool buttonState = HIGH; //saving state of the switch
byte tapCounter; //for saving no. of times the switch is pressed
int timediff; //for saving the time in between each press and release of the switch
bool flag1, flag2; //just two variables
long double presstime, releasetime; //for saving millis at press and millis at release

//byte buttonState = 0;
//const byte ledPin = 15;
//const byte buttonPin = 10;
//const unsigned long debouncerInterval = 50;
//unsigned long buttonPressedTime = 0;
//unsigned long currentMillis = 0;
//Bounce debouncer = Bounce();

//int downButton = 3;
int menu = 1;
int counter = 30;

// Interval for DHT sensor
unsigned long previousMillis = 0;        // will store last temp was read
const long interval = 2000;              // interval at which to read sensor

unsigned long previousdataMillis = 0;
const long datainterval = 30000;

unsigned long previoustimeMillis = 0;
const long timeinterval = 1000;

// Interval for turning eggs
unsigned long turnPreviousMillis = 0;
const long turnInterval = 3600000;             // 3600000 - 1 hours
//10800000 - 3 hours
// Interval for toggling heater
unsigned long heatPreviousMillis = 0;
const long heatInterval = 15000;

unsigned long newMillis = millis();
const int watchdog = 60000;

// Webserver  
const char* ssid = "Hari";
const char* password = "hari0001";

//SSID and Password to your ESP Access Point
//const char* ssid_AP = "Incub";
//const char* password_AP = "$$1000$$";

const char* www_username = "admin";
const char* www_password = "admin";

const char* www_realm = "Custom Auth Realm";
String authFailResponse = "Authentication Failed";

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");

char Time[ ] = "Time:00:00:00";
char Date[ ] = "Date:00/00/2000";
byte last_second, second_, minute_, hour_, day_, month_;
int year_;

int data1 = 0;
int data2 = 0;
int data3 = 0;
int data4 = 0;
int data5 = 0;
int data6 = 0;
int data7 = 0;
int data8 = 0;

int rotate;
int temper;
int wireless;
int heat;
int timeperiod;
int humid;
byte downButtonstate = 0;

BearSSL::ESP8266WebServerSecure server(443);
BearSSL::ServerSessions serverCache(5);

static const char serverCert[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIIDSzCCAjMCCQD2ahcfZAwXxDANBgkqhkiG9w0BAQsFADCBiTELMAkGA1UEBhMC
VVMxEzARBgNVBAgMCkNhbGlmb3JuaWExFjAUBgNVBAcMDU9yYW5nZSBDb3VudHkx
EDAOBgNVBAoMB1ByaXZhZG8xGjAYBgNVBAMMEXNlcnZlci56bGFiZWwuY29tMR8w
HQYJKoZIhvcNAQkBFhBlYXJsZUB6bGFiZWwuY29tMB4XDTE4MDMwNjA1NDg0NFoX
DTE5MDMwNjA1NDg0NFowRTELMAkGA1UEBhMCQVUxEzARBgNVBAgMClNvbWUtU3Rh
dGUxITAfBgNVBAoMGEludGVybmV0IFdpZGdpdHMgUHR5IEx0ZDCCASIwDQYJKoZI
hvcNAQEBBQADggEPADCCAQoCggEBAPVKBwbZ+KDSl40YCDkP6y8Sv4iNGvEOZg8Y
X7sGvf/xZH7UiCBWPFIRpNmDSaZ3yjsmFqm6sLiYSGSdrBCFqdt9NTp2r7hga6Sj
oASSZY4B9pf+GblDy5m10KDx90BFKXdPMCLT+o76Nx9PpCvw13A848wHNG3bpBgI
t+w/vJCX3bkRn8yEYAU6GdMbYe7v446hX3kY5UmgeJFr9xz1kq6AzYrMt/UHhNzO
S+QckJaY0OGWvmTNspY3xCbbFtIDkCdBS8CZAw+itnofvnWWKQEXlt6otPh5njwy
+O1t/Q+Z7OMDYQaH02IQx3188/kW3FzOY32knER1uzjmRO+jhA8CAwEAATANBgkq
hkiG9w0BAQsFAAOCAQEAnDrROGRETB0woIcI1+acY1yRq4yAcH2/hdq2MoM+DCyM
E8CJaOznGR9ND0ImWpTZqomHOUkOBpvu7u315blQZcLbL1LfHJGRTCHVhvVrcyEb
fWTnRtAQdlirUm/obwXIitoz64VSbIVzcqqfg9C6ZREB9JbEX98/9Wp2gVY+31oC
JfUvYadSYxh3nblvA4OL+iEZiW8NE3hbW6WPXxvS7Euge0uWMPc4uEcnsE0ZVG3m
+TGimzSdeWDvGBRWZHXczC2zD4aoE5vrl+GD2i++c6yjL/otHfYyUpzUfbI2hMAA
5tAF1D5vAAwA8nfPysumlLsIjohJZo4lgnhB++AlOg==
-----END CERTIFICATE-----
)EOF";

static const char serverKey[] PROGMEM =  R"EOF(
-----BEGIN RSA PRIVATE KEY-----
MIIEpQIBAAKCAQEA9UoHBtn4oNKXjRgIOQ/rLxK/iI0a8Q5mDxhfuwa9//FkftSI
IFY8UhGk2YNJpnfKOyYWqbqwuJhIZJ2sEIWp2301OnavuGBrpKOgBJJljgH2l/4Z
uUPLmbXQoPH3QEUpd08wItP6jvo3H0+kK/DXcDzjzAc0bdukGAi37D+8kJfduRGf
zIRgBToZ0xth7u/jjqFfeRjlSaB4kWv3HPWSroDNisy39QeE3M5L5ByQlpjQ4Za+
ZM2yljfEJtsW0gOQJ0FLwJkDD6K2eh++dZYpAReW3qi0+HmePDL47W39D5ns4wNh
BofTYhDHfXzz+RbcXM5jfaScRHW7OOZE76OEDwIDAQABAoIBAQDKov5NFbNFQNR8
djcM1O7Is6dRaqiwLeH4ZH1pZ3d9QnFwKanPdQ5eCj9yhfhJMrr5xEyCqT0nMn7T
yEIGYDXjontfsf8WxWkH2TjvrfWBrHOIOx4LJEvFzyLsYxiMmtZXvy6YByD+Dw2M
q2GH/24rRdI2klkozIOyazluTXU8yOsSGxHr/aOa9/sZISgLmaGOOuKI/3Zqjdhr
eHeSqoQFt3xXa8jw01YubQUDw/4cv9rk2ytTdAoQUimiKtgtjsggpP1LTq4xcuqN
d4jWhTcnorWpbD2cVLxrEbnSR3VuBCJEZv5axg5ZPxLEnlcId8vMtvTRb5nzzszn
geYUWDPhAoGBAPyKVNqqwQl44oIeiuRM2FYenMt4voVaz3ExJX2JysrG0jtCPv+Y
84R6Cv3nfITz3EZDWp5sW3OwoGr77lF7Tv9tD6BptEmgBeuca3SHIdhG2MR+tLyx
/tkIAarxQcTGsZaSqra3gXOJCMz9h2P5dxpdU+0yeMmOEnAqgQ8qtNBfAoGBAPim
RAtnrd0WSlCgqVGYFCvDh1kD5QTNbZc+1PcBHbVV45EmJ2fLXnlDeplIZJdYxmzu
DMOxZBYgfeLY9exje00eZJNSj/csjJQqiRftrbvYY7m5njX1kM5K8x4HlynQTDkg
rtKO0YZJxxmjRTbFGMegh1SLlFLRIMtehNhOgipRAoGBAPnEEpJGCS9GGLfaX0HW
YqwiEK8Il12q57mqgsq7ag7NPwWOymHesxHV5mMh/Dw+NyBi4xAGWRh9mtrUmeqK
iyICik773Gxo0RIqnPgd4jJWN3N3YWeynzulOIkJnSNx5BforOCTc3uCD2s2YB5X
jx1LKoNQxLeLRN8cmpIWicf/AoGBANjRSsZTKwV9WWIDJoHyxav/vPb+8WYFp8lZ
zaRxQbGM6nn4NiZI7OF62N3uhWB/1c7IqTK/bVHqFTuJCrCNcsgld3gLZ2QWYaMV
kCPgaj1BjHw4AmB0+EcajfKilcqtSroJ6MfMJ6IclVOizkjbByeTsE4lxDmPCDSt
/9MKanBxAoGAY9xo741Pn9WUxDyRplww606ccdNf/ksHWNc/Y2B5SPwxxSnIq8nO
j01SmsCUYVFAgZVOTiiycakjYLzxlc6p8BxSVqy6LlJqn95N8OXoQ+bkwUux/ekg
gz5JWYhbD6c38khSzJb0pNXCo3EuYAVa36kDM96k1BtWuhRS10Q1VXk=
-----END RSA PRIVATE KEY-----
)EOF";

String header = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";

String message;
 
LiquidCrystal_I2C lcd(0x27, 16, 2);

//ESP8266WebServer server(80);

void getTemperature() {

  unsigned long currentMillis = millis();

  if(currentMillis - previousMillis >= interval) {
    // save the last time you read the sensor 
    previousMillis = currentMillis;   
    
    temp = dht.readTemperature();     // Read temperature
    hum = dht.readHumidity();          // Read humidity (percent)
    f = dht.readTemperature(true);

    if (isnan(hum) && isnan(temp)) {
      Serial.println("Failed to read from DHT sensor!");
      data3 = 1;
//      lcd.clear();
//      lcd.setCursor(0,0);
//      lcd.print("No DHT Data!");
        //      return;
    }
    else {
    Serial.println("\nTemperature: ");
    Serial.print(temp);
    Serial.println("°C");
    Serial.println("Humidity: ");
    Serial.print(hum);
    Serial.println("%");
    Serial.println("Fahrenheit: ");
    Serial.print(f);
    Serial.println("°F");
    Serial.println("-------------");
    data3 = 2;
//    lcd.clear();
//    lcd.setCursor(0,0);
//    lcd.print("Humidity: ");
//    lcd.print(hum);
//    lcd.print("%");
//    lcd.setCursor(0,1);
//    lcd.print("Temp: "); 
//    lcd.print(f);
//    lcd.print((char)223);
//    lcd.print("F");
    }
  }
  temper = data3;
}

  // Time

void hold()
{
  digitalWrite(ledPin,!digitalRead(ledPin)); 
//      server.sendHeader("header","/");
//      client.print( header );
//      handleWeb();    
//      client.print( message );
//      server.send(303);  


//  int Buttonhold;
//  int num = 1;
//  if(Buttonhold % 2 == 0){
//      digitalWrite(ledPin, LOW);
//      num++;
////      server.sendHeader("header","/");
//      client.print( header );
//      handleWeb();    
//      client.print( message );
//      server.send(303); 
//  }
//    else {
//      digitalWrite(ledPin, HIGH);
//      num++;
////      server.sendHeader("header","/"); 
//      client.print( header );
//      handleWeb();    
//      client.print( message );
//      server.send(303); 
//    }
//  Buttonhold = num;
}

void handleWeb(){
  
    long sec = millis() / 1000;    
    int min = sec / 60;
    int hr = min / 60; 
    int day = (sec/(60*60*24))*100; 
     
    String title = "Incubator Status";
    String cssClass = "mediumhot";    
    if (temp < 0)
      cssClass = "cold";
    else if (temp > 37)
      cssClass = "hot";   
//      <meta http-equiv=\"refresh\" content=\"20\" />
    String message = "<!DOCTYPE html><html><head><title>" + title + "</title><meta name=\"viewport\" content=\"width=device-width\" /><style>\n";
    message += "html {height: 100%;}";
    message += "div {color: #fff;font-family: 'Advent Pro';font-weight: 400;left: 50%;position: absolute;text-align: center;top: 50%;transform: translateX(-50%) translateY(-50%);}";
    message += "h2 {font-size: 70px;font-weight: 400; margin: 0}";
    message += "body {height: 100%;}";
    message += " p { text-align:center; margin: 5px 0px 10px 0px; font-size: 120%;}";
    message += "#time_P { margin: 10px 0px 15px 0px;}";
    message += "#main {display: table; margin: auto;  padding: 0 10px 0 10px; }";
    message += ".button { background-color: #4CAF50; /* Green */ margin:5px; border: none; color: white; padding: 15px 32px;  text-align: center;  text-decoration: none;  display: inline-block;  font-size: 16px;}";
    message += ".cold {background: linear-gradient(to bottom, #7abcff, #0665e0 );}";
    message += ".mediumhot {background: linear-gradient(to bottom, #81ef85,#057003);}";
    message += ".hot {background: linear-gradient(to bottom, #fcdb88,#d32106);}";
    message += "</style>  ";
//    <script> ";
//    message += "function updateTime() ";
//    message += "{  ";
//    message += "   var d = new Date();";
//    message += "   var t = '';";
//    message += "   t = d.toLocaleTimeString();";
//    message += "   document.getElementById('P_time').innerHTML = t;";
//    message += "}";
//    message += "var myVar2 = setInterval(updateTime, 1000); " ;
//    message +="</script>";
    message +="<script>\n";
message +="setInterval(loadDoc1,10000);\n";
message +="function loadDoc1() {\n";
message +="var xhttp = new XMLHttpRequest();\n";
message +="xhttp.onreadystatechange = function() {\n";
message +="if (this.readyState == 4 && this.status == 200) {\n";
message +="document.getElementById(\"webpage_uptime_m\").innerHTML =this.responseText}\n";
message +="};\n";
message +="xhttp.open(\"GET\", \"web_uptime_min\", true);\n";
message +="xhttp.send();\n";
message +="}\n";
message +="</script>\n";  
message +="<script>\n";
message +="setInterval(loadDoc2,10000);\n";
message +="function loadDoc2() {\n";
message +="var xhttp = new XMLHttpRequest();\n";
message +="xhttp.onreadystatechange = function() {\n";
message +="if (this.readyState == 4 && this.status == 200) {\n";
message +="document.getElementById(\"webpage_uptime_h\").innerHTML =this.responseText}\n";
message +="};\n";
message +="xhttp.open(\"GET\", \"web_uptime_hr\", true);\n";
message +="xhttp.send();\n";
message +="}\n";
message +="</script>\n"; 
message +="<script>\n";
message +="setInterval(loadDoc3,10000);\n";
message +="function loadDoc3() {\n";
message +="var xhttp = new XMLHttpRequest();\n";
message +="xhttp.onreadystatechange = function() {\n";
message +="if (this.readyState == 4 && this.status == 200) {\n";
message +="document.getElementById(\"webpage_uptime_day\").innerHTML =this.responseText}\n";
message +="};\n";
message +="xhttp.open(\"GET\", \"web_uptime_d\", true);\n";
message +="xhttp.send();\n";
message +="}\n";
message +="</script>\n"; 

message +="<script>\n";
message +="setInterval(loadDoc4,10000);\n";
message +="function loadDoc4() {\n";
message +="var xhttp = new XMLHttpRequest();\n";
message +="xhttp.onreadystatechange = function() {\n";
message +="if (this.readyState == 4 && this.status == 200) {\n";
message +="document.getElementById(\"webpage_t\").innerHTML =this.responseText}\n";
message +="};\n";
message +="xhttp.open(\"GET\", \"web_t\", true);\n";
message +="xhttp.send();\n";
message +="}\n";
message +="</script>\n";  
message +="<script>\n";
message +="setInterval(loadDoc5,10000);\n";
message +="function loadDoc5() {\n";
message +="var xhttp = new XMLHttpRequest();\n";
message +="xhttp.onreadystatechange = function() {\n";
message +="if (this.readyState == 4 && this.status == 200) {\n";
message +="document.getElementById(\"webpage_f\").innerHTML =this.responseText}\n";
message +="};\n";
message +="xhttp.open(\"GET\", \"web_f\", true);\n";
message +="xhttp.send();\n";
message +="}\n";
message +="</script>\n"; 
message +="<script>\n";
message +="setInterval(loadDoc6,10000);\n";
message +="function loadDoc6() {\n";
message +="var xhttp = new XMLHttpRequest();\n";
message +="xhttp.onreadystatechange = function() {\n";
message +="if (this.readyState == 4 && this.status == 200) {\n";
message +="document.getElementById(\"webpage_h\").innerHTML =this.responseText}\n";
message +="};\n";
message +="xhttp.open(\"GET\", \"web_h\", true);\n";
message +="xhttp.send();\n";
message +="}\n";
message +="</script>\n";   
message +="<script>\n";
message +="setInterval(loadDoc7,10000);\n";
message +="function loadDoc7() {\n";
message +="var xhttp = new XMLHttpRequest();\n";
message +="xhttp.onreadystatechange = function() {\n";
message +="if (this.readyState == 4 && this.status == 200) {\n";
message +="document.getElementById(\"webpage_b\").innerHTML =this.responseText}\n";
message +="};\n";
message +="xhttp.open(\"GET\", \"web_b\", true);\n";
message +="xhttp.send();\n";
message +="}\n";
message +="</script>\n";   
    message += "<script> ";    
    message += "function send(led_sts) ";
    message += "{";
    message += "  var xhttp = new XMLHttpRequest();";
    message += "  xhttp.onreadystatechange = function() {";
    message += "    if (this.readyState == 4 && this.status == 200) {";
    message += "      document.getElementById(\"state\").innerHTML = this.responseText;";
    message += "    }";
    message += "  };";
    message += "  xhttp.open(\"GET\", \"led_set?state=\"+led_sts, true);";
    message += "  xhttp.send();";
    message += "}";
    message +="</script>\n";
    message +="</head><body class=\"" + cssClass + "\"><div><h2>" + title +  "</h2><p>Time: <span id='P_time'>" + Time +" </span></p><p >Uptime: <span id=\"webpage_uptime_day\">0</span> days <span id=\"webpage_uptime_h\">0</span> hours <span id=\"webpage_uptime_m\">0</span> minutes</p> <p>Temperature: <span id=\"webpage_t\">0</span>&nbsp;<small>&deg;C - </small><span id=\"webpage_f\">0</span>&nbsp;<small>&deg;F</small></p><p>Humidity: <span id=\"webpage_h\">0</span>&nbsp;%</p> <p>Battery:  <span id=\"webpage_b\">0</span>V</p> <p>  <button class=\"button\" onclick=\"send(1)\">LIGHT ON</button>  <button class=\"button\" onclick=\"send(0)\">LIGHT OFF</button></p><p> Light State: <span id=\"state\">NA</span></p> </div>";
        message += "  <script> ";
    message += "function updateTime() ";
    message += "{  ";
    message += "   var d = new Date();";
    message += "   var t = '';";
    message += "   t = d.toLocaleTimeString();";
    message += "   var P_time = document.getElementById('P_time').innerHTML = t;";
    message += "};";
    message += "var myVar2 = setInterval(updateTime, 1000); " ;
    message +="</script></body></html>";
    server.send(200, "text/html", message);
 
}


void web_uptime_m() {
  long sec = millis() / 1000; 
 int min = sec / 60;
 int mins = min % 60;
 String minu = String(mins);
 server.send(200,"text/plane",minu);
}

void web_uptime_h() {
  long sec = millis() / 1000; 
 int min = sec / 60;
 int hr = (min / 60) % 24; 
 String hrs = String(hr);
 server.send(200,"text/plane",hrs);
}

void web_uptime_days() {
 long sec = millis() / 1000;
 int min = sec / 60;
 int hr = min / 60; 
 int day = (sec/(60*60*24))*100;
 String days = String(day);
 server.send(200,"text/plane",days);
}

void sensor_t() {
 dtostrf(temp, 5, 2, temp_str);
 server.send(200,"text/plane",temp_str);
}

void sensor_f() {
 dtostrf(f, 5, 2, f_str); 
 server.send(200,"text/plane",f_str);
}

void sensor_h() {
 dtostrf(hum, 5, 2, hum_str);
 server.send(200,"text/plane",hum_str);
}

void sensor_b() {
 dtostrf(battery, 5, 2, b_str);
 server.send(200,"text/plane",b_str);
}

void handleNotFound(){
  String message = "Page Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

void led_control() 
{
 String state = "OFF";
 String act_state = server.arg("state");
 if(act_state == "1")
 {
  digitalWrite(ledPin,HIGH); //LED ON
  state = "ON";
 }
 else
 {
  digitalWrite(ledPin,LOW); //LED OFF
  state = "OFF";
 }
 server.send(200, "text/plane", state);
}

void setup(void){

  pinMode(1, FUNCTION_3);
  pinMode(3, FUNCTION_3);
  pinMode(12, FUNCTION_3);
  pinMode(13, FUNCTION_3);
  pinMode(14, FUNCTION_3);
  pinMode(15, FUNCTION_3);
  pinMode(0, FUNCTION_0);
  pinMode(2, FUNCTION_0);
  pinMode(4, FUNCTION_0);
  pinMode(5, FUNCTION_0);
    
  pinMode(ledPin, OUTPUT);
//  digitalWrite(ledPin, LOW);
//  debouncer.attach(buttonPin, INPUT_PULLUP);
//  debouncer.interval(debouncerInterval);
  Serial.begin(9600);
  dht.begin();
  Wire.begin(0,2);
//  lcd.init();   // initializing the LCD
  lcd.begin();
  lcd.backlight(); // Enable or Turn On the backlight  
  lcd.noCursor();  
//  updateMenu();
//  lcd.scrollDisplayLeft(); 
//  delay(350);
//  WiFi.mode(WIFI_AP_STA);           //Only Access point and Station
//  WiFi.softAP(ssid_AP, password_AP);
  WiFi.begin(ssid, password);
 // Serial.println("");
//  Serial.println("Setting Access Point");

//  Serial.println("AP connected :)");
//  IPAddress myIP = WiFi.softAPIP(); //Get IP address
//  Serial.print("HotSpot IP: ");
//  Serial.println(myIP);
//  uint8_t macAddr[6];
//  WiFi.softAPmacAddress(macAddr);
//  Serial.printf("MAC address = %02x:%02x:%02x:%02x:%02x:%02x\n", macAddr[0], macAddr[1], macAddr[2], macAddr[3], macAddr[4], macAddr[5]);
//  Serial.println("");

  Serial.printf("Connecting to %s ...\n", ssid);
//  delay(10000);
  
  // Output Pin
  pinMode(heatPin, OUTPUT);
  pinMode(rotatorPin1, OUTPUT);
  pinMode(rotatorPin2, OUTPUT);
  pinMode(humPin, OUTPUT);
  pinMode(rotatorPinOpp, OUTPUT);
//  pinMode(downButton, INPUT_PULLUP);
  pinMode(sw, INPUT_PULLUP); 
  pinMode(1, OUTPUT);
//  digitalWrite(4, LOW);
//  digitalWrite(5, LOW);
//  digitalWrite(1, LOW);

  ArduinoOTA.begin();  
  
  server.getServer().setRSACert(new BearSSL::X509List(serverCert), new BearSSL::PrivateKey(serverKey));

  server.getServer().setCache(&serverCache);
  
  server.on("/", []() {

    if (!server.authenticate(www_username, www_password))  {
      return server.requestAuthentication(DIGEST_AUTH, www_realm, authFailResponse);     
    }
   
    getTemperature();
    battery_volt();
//    client.print( message );
    handleWeb();
    
//    server.on(handleWeb);
  });


  server.on("/led_set", led_control);
  server.on("/web_t", sensor_t);  
  server.on("/web_f", sensor_f); 
  server.on("/web_h", sensor_h); 
  server.on("/web_b", sensor_b);
  
  server.on("/web_uptime_min", web_uptime_m);
  server.on("/web_uptime_hr", web_uptime_h);
  server.on("/web_uptime_d", web_uptime_days);
  server.onNotFound(handleNotFound);  
  
  // Set up mDNS responder:
  // - first argument is the domain name, in this example
  //   the fully-qualified domain name is "esp8266.local"
  // - second argument is the IP address to advertise
  //   we send our IP address on the WiFi network
  
      if (!MDNS.begin("incubator")) {
        Serial.println("Error setting up MDNS responder!");
        while(1) { 
        delay(1000);
        }
      }  
      Serial.println("mDNS responder started");
      server.begin();
      Serial.println("TCP server started");
      MDNS.addService("https", "tcp", 443);

  
  gotIpEventHandler = WiFi.onStationModeGotIP([](const WiFiEventStationModeGotIP& event)
  {
    Serial.println("");
    Serial.print("Connected to ");
    Serial.println(ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    data1 = 2;
//    lcd.clear(); 
//    lcd.setCursor(0,0);
//    lcd.print("Connected..");
//    lcd.print(ssid);
//    lcd.setCursor(0,1);
//    lcd.print("");
//    lcd.print(WiFi.localIP());
            
      timeClient.begin();
      timeClient.setTimeOffset(19800);
      Serial.println("NTP client connected");
  });
////  disconnectedEventHandler = WiFi.onStationModeDisconnected([](
  disconnectedEventHandler = WiFi.onStationModeDisconnected([](const WiFiEventStationModeDisconnected & event)
  { 
    Serial.println("WiFi Not Connected");
//    WiFiManager wifiManager;
//    wifiManager.autoConnect(ssid_AP, password_AP);
    data1 = 1;
//    lcd.clear();
//    lcd.setCursor(0,0);
//    lcd.print("WiFi Not Connected");
  });
  wireless = data1;
}

//void showConnectedDevices(){
//  unsigned long currentMillis = millis();
//  if ( currentMillis - newMillis > watchdog ) {
//    newMillis = currentMillis;
//    auto client_count = wifi_softap_get_station_num();
//    Serial.print("");
//    Serial.printf("Total devices connected = %d \n", client_count);
//    data5 = 0;        
//    auto i = 1;
//    struct station_info *station_list = wifi_softap_get_station_info();
//    while (station_list != NULL) {
//        auto station_ip = IPAddress((&station_list->ip)->addr).toString().c_str();
//        char station_mac[18] = {0};
//        sprintf(station_mac, "%02X:%02X:%02X:%02X:%02X:%02X", MAC2STR(station_list->bssid));
//        Serial.printf("%d. %s %s \n", i++, station_ip, station_mac);
//        station_list = STAILQ_NEXT(station_list, next);
//    }
//    wifi_softap_free_station_info();
//  }
//}

void battery_volt() {

  for(unsigned int i=0;i<10;i++){
  Vvalue=Vvalue+analogRead(BAT);         //Read analog Voltage
  delay(5);                              //ADC stable
  }
  Vvalue=(float)Vvalue/10.0;            //Find average of 10 values
  Rvalue=(float)(Vvalue/1024.0)*5;      //Convert Voltage in 5v factor
  battery=Rvalue*RatioFactor;          //Find original voltage by multiplying with factor
    /////////////////////////////////////Battery Voltage//////////////////////////////////
//    value = HIGH;  
}

void Sending_To_phpmyadmindatabase() {

  unsigned long currentdataMillis = millis();
  battery_volt();  
  if(currentdataMillis - previousdataMillis >= watchdog) {
    previousdataMillis = currentdataMillis;  
    WiFiClient client;
    hum = dht.readHumidity();          // Read humidity (percent)
    temp = dht.readTemperature();     // Read temperature
    f = dht.readTemperature(true);
    char server[] = "103.104.48.131";   //eg: 192.168.0.222
    const int   port = 80;  
    if (!client.connect(server, 80)) {        
//      Serial.print("Humidity: ");
//      Serial.print(hum);
//      Serial.print(F("%    Temperature: "));
//      Serial.print(temp);
//      Serial.print(F("°C "));
//      Serial.print(f);
//      Serial.print(F("°F    "));
    // if you didn't get a connection to the server:
      Serial.println("Failed to upload data");
      return;
    }
    else {
    String url = "http://103.104.48.131/pets/index.php?humidity=";
    url += hum;
    url += "&temperature=";
    url += f;
    url += "&battery=";
    url += battery;
    
    client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: 103.104.48.131 \r\n" + 
               "Connection: close\r\n\r\n");
    unsigned long timeout = millis();
    while (client.available() == 0) {
      if (millis() - timeout > 5000) {
        Serial.println(">>> Client Timeout !");
        client.stop();
        return;
      }
    }
  
    while(client.available()){
      String line = client.readStringUntil('\r');
      Serial.print(line);
    }
  }
 }
}

void uptime()  {
  
//  unsigned long currenttimeMillis = millis();
// 
//  if(currenttimeMillis - previoustimeMillis >= timeinterval) {
//    previoustimeMillis = currenttimeMillis; 
      lcd.clear(); 
      int sec = millis() / 1000;
      int min = sec / 60;
      int hr = min / 60; 
      int day = (sec/(60*60*24))*100;    
      lcd.setCursor(0,0); 
      lcd.print("Uptime: ");
      lcd.print(hr % 24);
      lcd.print(":");
      lcd.print(min % 60) ; 
      lcd.print(":");
      lcd.print(sec % 60);
      
//      delay(1000);
//  }
}

void getTime() {
    
//  unsigned long currenttimeMillis = millis();
// 
//  if(currenttimeMillis - previoustimeMillis >= timeinterval) {
//    previoustimeMillis = currenttimeMillis;  
    
    timeClient.update();
    Serial.println("");
    unsigned long epochTime = timeClient.getEpochTime();
  
    String formattedTime = timeClient.getFormattedTime();
    Serial.print("Formatted Time: ");
    Serial.println(formattedTime);  
    
    int currentHour = timeClient.getHours(); 

    int currentMinute = timeClient.getMinutes();
   
    int currentSecond = timeClient.getSeconds();

//  }
}

void getdate() {

  getTime();
  timeClient.update();
  unsigned long currenttimeMillis = millis();
 
  if(currenttimeMillis - previoustimeMillis >= timeinterval) {
    previoustimeMillis = currenttimeMillis;  
    unsigned long unix_epoch = timeClient.getEpochTime();    // Get Unix epoch time from the NTP server

    second_ = second(unix_epoch);
    if (last_second != second_) {


      minute_ = minute(unix_epoch);
      hour_   = hour(unix_epoch);
      day_    = day(unix_epoch);
      month_  = month(unix_epoch);
      year_   = year(unix_epoch);



      Time[12] = second_ % 10 + 48;
      Time[11] = second_ / 10 + 48;
      Time[9]  = minute_ % 10 + 48;
      Time[8]  = minute_ / 10 + 48;
      Time[6]  = hour_   % 10 + 48;
      Time[5]  = hour_   / 10 + 48;



      Date[5]  = day_   / 10 + 48;
      Date[6]  = day_   % 10 + 48;
      Date[8]  = month_  / 10 + 48;
      Date[9]  = month_  % 10 + 48;
      Date[13] = (year_   / 10) % 10 + 48;
      Date[14] = year_   % 10 % 10 + 48;

//      Serial.println(Time);
//      Serial.println(Date);

      last_second = second_;
      data6 = 1;
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(Time);
      lcd.clear();
      lcd.setCursor(0, 1);
      lcd.print(Date);
//      delay(500);
  }
  timeperiod = data6;
}
}

//void ledButton() {
//
// unsigned long curMillis = millis(); // store the current time
//  if (curMillis - preMillis >= period) { // check if 1000ms passed
//   preMillis = curMillis;
////    // Get the current time.
//  unsigned long aMillis = millis();
//
//  // Check if button was pressed or released.
//  if(debouncer.update()){
//
//    // Pressed.
//    if(debouncer.read() == 0){
//      if(buttonState == 0){
//        buttonState = 1;
//        buttonPressedTime = aMillis;
//      }
//    }
//
//    // Released.
//    else{
//      buttonState = 0;
//    }
//  }
//
//  // Turn the LED ON or OFF.
//  if(buttonState == 1){
//    if(aMillis - buttonPressedTime >= 1000){
//      digitalWrite(ledPin, LOW);
//      buttonState = 0;
//    }
//    else if(aMillis - buttonPressedTime >= 100){
//      digitalWrite(ledPin, HIGH);
//    }
//  }
//}
//}

//void webfunction() {
//
//    case 'R': {
//      out.printf_P(PSTR("Restart, ESP.restart(); ...\r\n"));
//      ESP.restart();
//      break;
//    }
//}
   
//void temhum(){
//  unsigned long currentMillis = millis();
// 
//  if(currentMillis - previousMillis >= interval) {
//    // save the last time you read the sensor 
//    previousMillis = currentMillis;   
//
//        lcd.clear();
//        lcd.setCursor(0,0);
//        lcd.print("Humidity: ");
//        lcd.print(hum);
//        lcd.print("%");
//        lcd.setCursor(0,1);
//        lcd.print("Temp: "); 
//        lcd.print(f);
//        lcd.print((char)223);
//        lcd.print("F");
//
//  }
//}



void ButtonPress() {

  int reading = digitalRead(sw);

  if (reading != lastButtonState) {
    // reset the debouncing timer
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    // whatever the reading is at, it's been there for longer than the debounce
    // delay, so take it as the actual current state:

    // if the button state has changed:
    if (reading != buttonState) {
      buttonState = reading;
    }
  }
  //Serial.println(buttonState);

  //when switch is pressed
  if (buttonState == 0 && flag2 == 0)
  {
    presstime = millis(); //time from millis fn will save to presstime variable
    flag1 = 0;
    flag2 = 1;
    tapCounter++; //tap counter will increase by 1
    //delay(10); //for avoiding debouncing of the switch
  }
  //when sw is released
  if (buttonState == 1 && flag1 == 0)
  {
    releasetime = millis(); //time from millis fn will be saved to releasetime var
    flag1 = 1;
    flag2 = 0;

    timediff = releasetime - presstime; //here we find the time gap between press and release and stored to timediff var
    //Serial.println(timediff);
    //delay(10);
  }

  if ((millis() - presstime) > 400 && buttonState == 1) //wait for some time and if sw is in release position
  {
    if (tapCounter == 1) //if tap counter is 1
    {
      if (timediff >= 400) //if time diff is larger than 400 then its a hold
      {
        Serial.println("Hold");
        hold(); //fn to call when the button is hold
      }
      else //if timediff is less than 400 then its a single tap
      {
        Serial.println("single tap");
        singleTap(); //fn to call when the button is single taped
      }
    }
//    else if (tapCounter == 2 ) //if tapcounter is 2
//    {
//      if (timediff >= 1000) // if timediff is greater than  400 then its single tap and hold
//      {
//        Serial.println("single tap and hold");
//        tapAndHold(); //fn to call when the button is single tap and hold
//      }
//      else // if timediff is less than 400 then its just double tap
//      {
//        Serial.println("double tap");
//        doubleTap(); //fn to call when doubletap
//      }
//    }
//    else if (tapCounter == 3) //if tapcounter is 3 //then its triple tap
//    {
//      Serial.println("triple tap");
//      tripleTap(); //fn to call when triple tap
//    }
//    else if (tapCounter == 4) //if tapcounter is 4 then its 4 tap
//    {
//      Serial.println("four tap");
//      fourTap();//fn to call when four tap
//    }
    tapCounter = 0;
  }
  lastButtonState = reading;

}

void Heater(){
  
  unsigned long currentMillis = millis();

  if(currentMillis - heatPreviousMillis >= heatInterval) {

    heatPreviousMillis = currentMillis;

    getTemperature();
//    lcd.clear();
    
    if (f > 70 && f < 99.5)  {  
      digitalWrite(heatPin, HIGH);
      Serial.println("Heater and Fan ON!");
      data2 = 1;
//      lcd.setCursor(0,0);
//      lcd.print("Heater ON!");
    }

    else if (f == temp) {
      digitalWrite(heatPin, LOW);
      Serial.println("No value from DHT Sensor!");
      data2 = 2;
    }
    else {
      digitalWrite(heatPin, LOW);
      Serial.println("Heater and Fan OFF!");
      data2 = 3;
//      lcd.clear();
//      lcd.setCursor(0,0);
//      lcd.print("Heater OFF!");
    }
    if((hum > 40) && (hum < 54)){
      digitalWrite(humPin, HIGH);
      Serial.println("Humidifier on!"); 
      data8 = 1;
//      lcd.clear();
//      lcd.setCursor(0,1);
//      lcd.print("Humidifier on!"); 
    }
    else if (hum > 54 && hum < 70)  {
      digitalWrite(humPin, LOW);
      Serial.println("Humidifier off!");
      data8 = 2;
//      lcd.clear();
//      lcd.setCursor(0,1); 
//      lcd.print("Humidifier off!");
    }

    else if (hum > 70)  {
      digitalWrite(humPin, LOW);
      Serial.println("Humidity is too high!");
      data8 = 3;
//      lcd.clear();
//      lcd.setCursor(0,1);
//      lcd.print("Humidity High!");
    }
    else {
      digitalWrite(humPin, LOW);
      Serial.println("Humidity is very low. Please fill water");
      data8 = 4;
//      lcd.clear();
//      lcd.setCursor(0,1);
//      lcd.print("Check Water");
    }
  }
  heat = data2;
  humid = data8;
}

void rotator() {

  unsigned long currentMillis = millis();
  if(currentMillis - turnPreviousMillis >= turnInterval) {

    turnPreviousMillis = currentMillis;

    timeClient.update();
    int currentHour = timeClient.getHours();
    Serial.print("Hour: ");
    Serial.println(currentHour); 
//    int currentMinute = timeClient.getMinutes();
//    Serial.print("Min: ");
//    Serial.println(currentMinute);
    
    if((currentHour % 2) == 0 || currentHour == 0) {
      digitalWrite(rotatorPin1, HIGH);
      digitalWrite(rotatorPin2, LOW);  
      digitalWrite(rotatorPinOpp, LOW);    
      Serial.println("Turning eggs!");
      delay(60000);
      digitalWrite(rotatorPin1, LOW);
      digitalWrite(rotatorPin2, LOW);  
      digitalWrite(rotatorPinOpp, LOW); 
      data7 = 1;
      }
    else if((currentHour % 2) != 0) {
      digitalWrite(rotatorPin1, LOW);
      digitalWrite(rotatorPin2, HIGH);  
      digitalWrite(rotatorPinOpp, HIGH);    
      Serial.println("Turning eggs Opp!");
      delay(60000);   
      digitalWrite(rotatorPin1, LOW);
      digitalWrite(rotatorPin2, LOW);  
      digitalWrite(rotatorPinOpp, LOW);
      data7 = 2;
      } 
  }
  rotate = data7;
}

void updateMenu() {

//  getdate();
  getTemperature();
  temp = dht.readTemperature();     // Read temperature
  hum = dht.readHumidity();          // Read humidity (percent)
  f = dht.readTemperature(true);
//  auto client_count = wifi_softap_get_station_num();
  unsigned long currentMillis = millis();
  
  switch (menu) {
    case 0: {
      menu = 1;
    }
      break;
    case 1: 
//      if (wireless == 2) {
//        lcd.clear(); 
//        lcd.setCursor(0,0);
//        lcd.print("Connected..");
//        lcd.print(ssid);
//        lcd.setCursor(0,1);
//        lcd.print("");
//        lcd.print(WiFi.localIP());
//      }
//      else if(wireless == 1) {
//        lcd.clear();
//        lcd.setCursor(0,0);
//        lcd.print("WiFi Not Connected");
//      }
//    
//      break;

//      lcd.clear(); 
      if (WiFi.status() == WL_CONNECTED) {
        lcd.setCursor(0,0);
        lcd.print("Connected..");
        lcd.print(ssid);
        lcd.setCursor(0,1);
        lcd.print("");
        lcd.print(WiFi.localIP());
//        delay(5000);
      }
      else {
        lcd.setCursor(0,0);
        lcd.print("WiFi Not Connected");
//      delay(5000); 
      }
      break;

    case 2: 
//    Heater();
//    for(int x = 1; x <= 2; x++) {
//      lcd.clear();
      if (heat == 1)  {  
//        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Heater ON!");
      }

      else if (heat == 2) {
//      lcd.clear();
        menu = 3;
      }
     
      else if(heat == 3){
//      lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Heater OFF!");
      }
    
      if(humid == 1){
        lcd.setCursor(0,1);
        lcd.print("Humidifier on!"); 
 //       delay(5000);
      }
      else if (humid == 2)  {
        lcd.setCursor(0,1); 
        lcd.print("Humidifier off!");
//    delay(5000);
      }
      else if (humid == 3)  {
        lcd.setCursor(0,1);
        lcd.print("Humidity High!");
//      delay(5000);
      }
      else if(humid = 4) {
        lcd.setCursor(0,1);
        lcd.print("Check Water");
      }
//      x = 1;
//      delay(10000);
    
      break;
    
    case 3: 
////    for(int y = 0; y <= 2; y++) {  
//      getTemperature();
//      if (temper == 1) {
//        lcd.clear();
//        lcd.setCursor(0,0);
//        lcd.print("No DHT Data!");
//      }
//      else if(temper == 2) {
//        lcd.clear();
//        lcd.setCursor(0,0);
//        lcd.print("Humidity: ");
//        lcd.print(hum);
//        lcd.print("%");
//        lcd.setCursor(0,1);
//        lcd.print("Temp: "); 
//        lcd.print(f);
//        lcd.print((char)223);
//        lcd.print("F");
////        delay(2000);
//     }
////      y = 0;
////      delay(1000); 
    
    temp = dht.readTemperature();     // Read temperature
    hum = dht.readHumidity();          // Read humidity (percent)
    f = dht.readTemperature(true);

    if (isnan(hum) && isnan(temp)) {
//      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("No DHT Data!");
      return;
    }
    else {
//    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Humidity: ");
    lcd.print(hum);
    lcd.print("%");
    lcd.setCursor(0,1);
    lcd.print("Temp: "); 
    lcd.print(f);
    lcd.print((char)223);
    lcd.print("F");
    }
    
      break;
    
    case 4: 
      uptime();
      break;
      
//    case 5: 
////      lcd.clear(); 
////      if (data5 = 0) {
//      lcd.setCursor(0,0);
//      lcd.print("No AP Configured");
////        lcd.print(ssid_AP);
////        lcd.setCursor(0,1);
////        lcd.print("Devices: ");
////        lcd.print(client_count);
//      
//      break;  
////    }    
    case 5: 
    getdate();
    if (timeperiod == 1) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(Time);
      lcd.setCursor(0, 1);
      lcd.print(Date);
    }
//       z=0;
//       delay(1000);
     
    break;
   
//    case 6: 
////      rotator();
//      if(rotate == 1) {
////        lcd.clear();
//        lcd.setCursor(0, 0);
//        lcd.print("Turning eggs!");
//      }
//      else if(rotate == 2) {
////        lcd.clear();
//        lcd.setCursor(0, 0);
//        lcd.print("Turning eggs Opp!");
//      }
//    else {
//      menu = 1;
//    }
    
      break;
    case 6: {
        menu = 1;
    }
        break;
  }
}

//void displayrepeat() {
//
//     static unsigned long backlightTime;
//     static boolean backlightOn;
//
//  if (!digitalRead(downButton)){
//    menu++;
//    Serial.println("downButton");
//    lcd.clear();
//    lcd.backlight();
//    updateMenu();
//    downButtonstate = 1;
////    delay(100);
//    delay(20);     //primitive debounce button. You forgot that too
//    backlightOn = true;
//    backlightTime = millis();
//    while (!digitalRead(downButton));
//  }   
//     if(backlightOn == true && millis() - backlightTime > 10000)
//     {
//          lcd.noBacklight();
//          backlightOn = false;
//     }
//  downButtonstate = 0; 
//}

void singleTap() {

     static unsigned long backlightTime;
     static boolean backlightOn;

    Serial.println("downButton");
    lcd.clear();
    menu++;
    lcd.backlight();
    updateMenu();
    delay(80);    
    backlightOn = true;
    backlightTime = millis();
      
     if(backlightOn == true && millis() - backlightTime > 10000)
     {
          lcd.noBacklight();
          backlightOn = false;
     }  
}

void loop(void){ 
  ArduinoOTA.handle();  
  server.handleClient();
  MDNS.update();
  Heater();
  ButtonPress();
  singleTap();
  Sending_To_phpmyadmindatabase(); 
  rotator();
//  displayrepeat(); 
//  getdate();
//  uptime();
//  showConnectedDevices();
//  for (int positionCounter = 0; positionCounter < 13; positionCounter++) {
//    lcd.scrollDisplayLeft();
//    delay(500);
//  }

}
  
