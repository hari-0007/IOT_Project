#include <DHT.h>
#include <ESP8266WiFi.h>
//#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <TimeLib.h>
#include <WiFiManager.h>

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

float hum;
float temp;
float f;
char temp_str[6];
char hum_str[6];
char f_str[6];
char b_str[6];

float battery;
float Vvalue=0.0,Rvalue=0.0;
  
ESP8266WebServer server(80);
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

int menu = 1;
int counter = 30;

unsigned long previousMillis = 0;        // will store last temp was read
const long interval = 2000;              // interval at which to read sensor

unsigned long previousdataMillis = 0;
const long datainterval = 30000;

unsigned long previoustimeMillis = 0;
const long timeinterval = 1000;

unsigned long turnPreviousMillis = 0;
const long turnInterval = 3600000;             // 3600000 - 1 hours

unsigned long heatPreviousMillis = 0;
const long heatInterval = 15000;

unsigned long newMillis = millis();
const int watchdog = 60000;

// Webserver  
const char* ssid = "Hari";
const char* password = "hari0001";

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
 
LiquidCrystal_I2C lcd(0x27, 16, 2);

void getTemperature() {

  unsigned long currentMillis = millis();

  if(currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;   
    
    temp = dht.readTemperature();     // Read temperature
    hum = dht.readHumidity();          // Read humidity (percent)
    f = dht.readTemperature(true);

    if (isnan(hum) && isnan(temp)) {
      Serial.println("Failed to read from DHT sensor!");
      data3 = 1;
    }
    else {
    data3 = 2;
    }
  }
  temper = data3;
}


void hold()
{
  digitalWrite(ledPin,!digitalRead(ledPin)); 
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

  dht.begin();
  Wire.begin(0,2);

  lcd.begin();
  lcd.backlight(); // Enable or Turn On the backlight  
  lcd.noCursor();  

//  WiFi.begin(ssid, password);

  pinMode(heatPin, OUTPUT);
  pinMode(rotatorPin1, OUTPUT);
  pinMode(rotatorPin2, OUTPUT);
  pinMode(humPin, OUTPUT);
  pinMode(rotatorPinOpp, OUTPUT);
//  pinMode(downButton, INPUT_PULLUP);
  pinMode(sw, INPUT_PULLUP); 
  pinMode(1, OUTPUT); 

    WiFiManager wifiManager;
//    wifiManager.resetSettings();
    wifiManager.autoConnect("Incubator");
    
      server.begin();

  gotIpEventHandler = WiFi.onStationModeGotIP([](const WiFiEventStationModeGotIP& event)
  {    
      timeClient.begin();
      timeClient.setTimeOffset(19800);
  });
  disconnectedEventHandler = WiFi.onStationModeDisconnected([](const WiFiEventStationModeDisconnected & event)
  { 
//    WiFiManager wifiManager;
//   wifiManager.autoConnect("Incubator");
  });
  wireless = data1;
}

void battery_volt() {

  for(unsigned int i=0;i<10;i++){
  Vvalue=Vvalue+analogRead(BAT);         //Read analog Voltage
  delay(5);                              //ADC stable
  }
  Vvalue=(float)Vvalue/10.0;            //Find average of 10 values
  Rvalue=(float)(Vvalue/1024.0)*5;      //Convert Voltage in 5v factor
  battery=Rvalue*RatioFactor;          //Find original voltage by multiplying with factor 
}

void Sending_To_phpmyadmindatabase() {

  unsigned long currentdataMillis = millis();
  battery_volt();  
  if(currentdataMillis - previousdataMillis >= watchdog) {
    previousdataMillis = currentdataMillis;  
    WiFiClient client;
//    WiFiManager wifiManager;
    hum = dht.readHumidity();          // Read humidity (percent)
    temp = dht.readTemperature();     // Read temperature
    f = dht.readTemperature(true);
    long sec = millis() / 1000;    
    int min = sec / 60;
    int hr = min / 60; 
    int day = (sec/(60*60*24))*100; 

    int mins = min % 60;
    String minu = String(mins);
    int hru = (min / 60) % 24; 
    String hrs = String(hru);
    String days = String(day);
    
    char server[] = "103.104.48.131";   //eg: 192.168.0.222
    const int   port = 80;  
    if (!client.connect(server, 80)) {        
      return;
    }
    else {
    String url = "http://103.104.48.131/pets/index.php?humidity=";
    url += hum;
    url += "&temperature=";
    url += f;
    url += "&battery=";
    url += battery;
    url += "&days=";
    url += days;
    url += "&hrs=";
    url += hrs;
    url += "&minu=";
    url += minu;
    
    client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: 103.104.48.131 \r\n" + 
               "Connection: close\r\n\r\n");
    unsigned long timeout = millis();
    while (client.available() == 0) {
      if (millis() - timeout > 5000) {
        client.stop();
        return;
      }
    }
  
    while(client.available()){
      String line = client.readStringUntil('\r');
    }
  }
 }
}

void uptime()  {
  
  unsigned long currenttimeMillis = millis();
 
  if(currenttimeMillis - previoustimeMillis >= timeinterval) {
    previoustimeMillis = currenttimeMillis; 
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
  }
}

void getTime() {
    
    timeClient.update();
    unsigned long epochTime = timeClient.getEpochTime();
  
    String formattedTime = timeClient.getFormattedTime(); 
    
    int currentHour = timeClient.getHours(); 

    int currentMinute = timeClient.getMinutes();
   
    int currentSecond = timeClient.getSeconds();

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

      last_second = second_;
      data6 = 1;
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(Time);
      lcd.clear();
      lcd.setCursor(0, 1);
      lcd.print(Date);
  }
  timeperiod = data6;
}
}

void ButtonPress() {

  int reading = digitalRead(sw);

  if (reading != lastButtonState) {

    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {

    if (reading != buttonState) {
      buttonState = reading;
    }
  }

  if (buttonState == 0 && flag2 == 0)
  {
    presstime = millis(); 
    flag1 = 0;
    flag2 = 1;
    tapCounter++; 
  }

  if (buttonState == 1 && flag1 == 0)
  {
    releasetime = millis(); 
    flag1 = 1;
    flag2 = 0;

    timediff = releasetime - presstime; //here we find the time gap between press and release and stored to timediff var

  }

  if ((millis() - presstime) > 400 && buttonState == 1) //wait for some time and if sw is in release position
  {
    if (tapCounter == 1) //if tap counter is 1
    {
      if (timediff >= 400) //if time diff is larger than 400 then its a hold
      {
        hold(); //fn to call when the button is hold
      }
      else //if timediff is less than 400 then its a single tap
      {
        singleTap(); //fn to call when the button is single taped
      }
    }

    tapCounter = 0;
  }
  lastButtonState = reading;

}

void Heater(){
  
  unsigned long currentMillis = millis();

  if(currentMillis - heatPreviousMillis >= heatInterval) {

    heatPreviousMillis = currentMillis;

    getTemperature();
    
    if (f > 70 && f < 99.5)  {  
      digitalWrite(heatPin, HIGH);
      data2 = 1;

    }

    else if (f == temp) {
      digitalWrite(heatPin, LOW);
      data2 = 2;
    }
    else {
      digitalWrite(heatPin, LOW);
      data2 = 3;
    }
    if((hum > 40) && (hum < 54)){
      digitalWrite(humPin, HIGH);
      data8 = 1;
    }
    else if (hum > 54 && hum < 70)  {
      digitalWrite(humPin, LOW);
      data8 = 2;
    }

    else if (hum > 70)  {
      digitalWrite(humPin, LOW);
      data8 = 3;
    }
    else {
      digitalWrite(humPin, LOW);
      data8 = 4;
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

    if((currentHour % 2) == 0 || currentHour == 0) {
      digitalWrite(rotatorPin1, HIGH);
      digitalWrite(rotatorPin2, LOW);  
      digitalWrite(rotatorPinOpp, LOW);    
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
  unsigned long currentMillis = millis();
  
  switch (menu) {
    case 0: {
      menu = 1;
    }
      break;
    case 1: 
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
      }
      break;

    case 2: 
      if (heat == 1)  {  
        lcd.setCursor(0,0);
        lcd.print("Heater ON!");
      }

      else if (heat == 2) {
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
    
      break;
    
    case 3: 
    
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
       
    case 5: 
    getdate();
    if (timeperiod == 1) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(Time);
      lcd.setCursor(0, 1);
      lcd.print(Date);
    }    
      break;
      
    case 6: {
        menu = 1;
    }
        break;
  }
}

void singleTap() {

     static unsigned long backlightTime;
     static boolean backlightOn;

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
  server.handleClient();
  Heater();
  ButtonPress();
  Sending_To_phpmyadmindatabase(); 
  rotator();
}
  
