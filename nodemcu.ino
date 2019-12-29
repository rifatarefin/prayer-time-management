/* ESP8266 Webserver obtaining arguments from a connected client
  The 'MIT License (MIT) Copyright (c) 2016 by David Bird'. Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
  documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish,
  distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the
  following conditions:
   The above copyright ('as annotated') notice and this permission notice shall be included in all copies or substantial portions of the Software and where the
   software use is visible to an end-user.
   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHOR OR COPYRIGHT HOLDER BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF, OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
  See more at http://dsbird.org.uk
*/

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>

// Date and time functions using a DS1307 RTC connected via I2C and Wire lib
//common anode
#include <Wire.h>
#include "RTClib.h"

struct MyObject {
  int fajr;
  int sunrise;
  int johr;
  int asr;
  int sunset;
  int esha;


};

WiFiClient client;
ESP8266WebServer server(80); // Start server on port 80 (default for a web-browser, change to your requirements, e.g. 8080 if your Router uses port 80
// To access server from the outsid of a WiFi network e.g. ESP8266WebServer server(8266); and then add a rule on your Router that forwards a
// connection request to http://your_network_ip_address:8266 to port 8266 and view your ESP server from anywhere.
// Example http://g6ejd.uk.to:8266 will be directed to http://192.168.0.40:8266 or whatever IP address your router gives to this server

String Argument_Name, Clients_Response1, Clients_Response2, hours = "0", minutes = "0";
int fajr = 0, johr = 0, asr = 0, magrib = 0, esha = 0;

MyObject today;
RTC_DS1307 RTC;
//Pin connected to ST_CP of 74HC595
int latchPin = 12;  //12
//Pin connected to SH_CP of 74HC595
int clockPin = 13; //SHIFT 11
////Pin connected to DS of 74HC595
int dataPin = 14;  //14
int tempPin = 10;
float vout;
float tempc;
float tempf;
int number;
//Defining variables for hour and minute
int h;
int m;
//Defining variables for thousands,hundreds,tens and units digit
int a;
int b;
int c;
int d;
//Led pins for separating hour and minute
int led = 0, state=1;
//int led2 = 9;
int thisDay = 0;
int numOfRegisters = 6;
byte* registerState;
const char* ssid = "FourTen";
const char* password = "?amravalo!!?";

void setup() {
  Serial.begin(115200);
  //WiFiManager intialisation. Once completed there is no need to repeat the process on the current board

  // At this stage the WiFi manager will have successfully connected to a network, or if not will try again in 180-seconds
  //----------------------------------------------------------------------------------------------------------------------
  //Setting pin mode as OUTPUT
  pinMode(0, OUTPUT);
  pinMode(2, OUTPUT);
  pinMode(1, OUTPUT);
  pinMode(3, OUTPUT);
  pinMode(15,OUTPUT);//   : pin
  //pinMode(10,INPUT);

  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT);

  delay(10);
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  IPAddress ip(192, 168, 0, 150); // this 3 lines for a fix IP-address
  IPAddress gateway(192, 168, 0, 1);
  IPAddress subnet(255, 255, 255, 0);
  WiFi.mode(WIFI_STA);
  WiFi.config(ip, gateway, subnet); // before or after Wifi.Begin(ssid, password);
  WiFi.begin(ssid, password);

  Serial.println("hello");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  // Start the server
  server.begin();
  Serial.println("Server started");

  // Print the IP address
  Serial.print("Use this URL to connect: ");
  Serial.print("http://");
  Serial.print(WiFi.localIP());
  Serial.println("/");

  EEPROM.begin(1024);

  // Next define what the server should do when a client connects
  server.on("/", HandleClient); // The client connected with no arguments e.g. http:192.160.0.40/

  Wire.begin(4,5);      //sda,scl
    RTC.begin();
    RTC.adjust(DateTime(__DATE__, __TIME__));
    if (! RTC.isrunning()) {
      Serial.println("RTC is NOT running!");
  
      // following line sets the RTC to the date & time this sketch was compiled
      
    }

  registerState = new byte[numOfRegisters];
  readMem(1);
  
}

void readMem(int day)
{

   int eeAddress = (day-1)*sizeof(today); //EEPROM address to start reading from



    EEPROM.get(eeAddress, today);

    Serial.println("Read custom object from EEPROM: ");
    Serial.println(today.fajr);
    Serial.println(today.sunrise);
    Serial.println(today.johr);
    Serial.println(today.asr);
    Serial.println(today.sunset);
    Serial.println(today.esha);

}




void loop() {

  
  server.handleClient();
  byte mac[6];
  WiFi.macAddress(mac);
  Serial.print("MAC: ");
  Serial.print(mac[5],HEX);
  Serial.print(":");
  Serial.print(mac[4],HEX);
  Serial.print(":");
  Serial.print(mac[3],HEX);
  Serial.print(":");
  Serial.print(mac[2],HEX);
  Serial.print(":");
  Serial.print(mac[1],HEX);
  Serial.print(":");
  Serial.println(mac[0],HEX);
  
  
  led++;
  if(led>=40)
  {
    if(state==0)state=1;
    else state=0;
    vout=analogRead(A0);
    vout=(vout*330)/1023;
    tempc=vout; // Storing value in Degree Celsius
    if(tempc>=30)tempc-=5;
    led=0;
  }
  digitalWrite(15, state);

  DateTime now = RTC.now();
  if (now.day() != thisDay)
  {
    thisDay = now.day();
    readMem(thisDay);
  }
  int h = now.hour()%12;
  if(h==0)h=12;
  int m = now.minute();
  //Storing hour and minute in a form of 4 digit number
  number = h * 100 + m;
  
  
  digitOn(1);
  digitalWrite(0, HIGH);
  delay(10);
  digitalWrite(0, LOW);
  
  digitOn(2);
  digitalWrite(2, HIGH);
  delay(10);
  digitalWrite(2, LOW);
  
  digitOn(3);
  digitalWrite(1, HIGH);
  delay(10);
  digitalWrite(1, LOW);
  
  digitOn(4);
  digitalWrite(3, HIGH);
  delay(10);
  digitalWrite(3, LOW);
  
}

void digitOn(int pos)
{
  
  regShow(5,~mapDigit(number, pos));
  
  
  regShow(4,~mapDigit((int)(tempc*100), pos));
  
  
  regShow(3,~mapDigit(fajr, pos));
  
  
  regShow(2,~mapDigit(johr, pos));
  
  
  regShow(1,~mapDigit(today.sunrise, pos));
  
  
  regShow(0,~mapDigit(today.sunset, pos));
  
  
}



//starting from reg0
void regShow(int reg, int val) {
//  for (size_t i = 0; i < numOfRegisters; i++) {
//    registerState[i] = ~0;
//  }


  for (int i = 7; i >= 0; i--)
  {
    regWrite(reg, i, val & (1 << i));
  }
  //delay(1);

}
void regWrite(int reg, int actualPin, bool state) {
  //Determines register

  //Determines pin for actual register


  //Begin session
  digitalWrite(latchPin, LOW);

  for (int i = 0; i < numOfRegisters; i++) {
    //Get actual states for register
    byte* states = &registerState[i];

    //Update state
    if (i == reg) {
      bitWrite(*states, actualPin, state);
    }

    //Write
    shiftOut(dataPin, clockPin, MSBFIRST, *states);

  }

  //End session
  digitalWrite(latchPin, HIGH);
}



int mapDigit(int number, int pos)
{
  int a = number / 1000 % 10; //Separating thousands digit from the number
  int b = number / 100 % 10; //Separating hundreds digit from the number
  int c = number / 10 % 10; //Separating tens digit from the number
  int d = number % 10;    //Separating units digit from the number
  int t;
  //Converting the individual digits into its corresponding number for passing it through the shift register
  if(pos==1)t=a;
  else if(pos==2)t=b;
  else if(pos==3)t=c;
  else if(pos==4)t=d;
  
  switch (t) {
    case 0:
      d = 63;
      break;
    case 1:
      d = 06;
      break;
    case 2:
      d = 91;
      break;
    case 3:
      d = 79;
      break;
    case 4:
      d = 102;
      break;
    case 5:
      d = 109;
      break;
    case 6:
      d = 125;
      break;
    case 7:
      d = 07;
      break;
    case 8:
      d = 127;
      break;
    case 9:
      d = 111;
      break;
  }
  return d;
  

 
}


void HandleClient() {
  String webpage;
  webpage =  "<html>";
  webpage += "<head><title>Prayer Time</title>";
  webpage += "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">";
  webpage += "<style>";
  webpage += "body { background-color: #E6E6FA; font-family: Arial, Helvetica, Sans-Serif; Color: blue;}";
  webpage += "table, th, td {\
    border: 1px solid black;\
    border-collapse: collapse;\
    padding: 15px;\
    text-align: center;\
}\
table {\
    width:75%;\
}\
table tr:nth-child(even) {\
    background-color: #eee;\
}\
table tr:nth-child(odd) {\
   background-color:#fff;\
}\
table th {\
    background-color: purple;\
    color: white;\
}";
  webpage += "</style>";
  webpage += "</head>";
  webpage += "<body>";
  webpage += "<div align=\"center\"><h1><br>Set Prayer Times</h1>";
  String IPaddress = WiFi.localIP().toString();
  webpage += "<h4><form onsubmit=\"setTimeout(function () { window.location.reload(); }, 50)\" action='http://" + IPaddress + "' method='POST'>"; // omit  for just 'enter'
  webpage += "Select Prayer: <select name='jamat_name'> <option value='fajr'>Fajr</option><option value='johr'>Johr</option> <option value='asr'>Asr</option> <option value='magrib'>Maghrib</option><option value='esha'>Esha</option></select> ";
  webpage += "<br><br>Select Time: Hour <select name='jamat_hour'>";
  for (int i = 1; i <= 12; i++)
  {
    webpage += "<option value=" + (String)i + ">" + (String)i + "</option>";
  }
  webpage += "</select>&nbsp";
  webpage += "Minutes <select name='jamat_min'>";
  for (int i = 0; i <= 59; i++)
  {
    if(i<10)webpage += "<option value=0" +(String)i + ">" + (String)i + "</option>";
    else webpage += "<option value="+(String)i + ">" + (String)i + "</option>";
  }
  webpage += "</select>&nbsp<br>";

  webpage += "<br><input type='submit' value='Enter'>";
  webpage += "</form></h4></div>";
  webpage += "<div align=\"center\"><table >\
  <tr>\
    <th>Prayer</th>\
    <th>Time</th>\ 
    <th>Jamaat</th>\ 
  </tr>\
  <tr>\
    <td>Fajr</td>\
    <td>"+(String)(today.fajr/100)+":"+(String)(today.fajr%100)+"</td>\
    <td>"+(String)(fajr/100)+":"+(String)(fajr%100)+"</td>\
  </tr>\
  <tr>\
    <td>Johr</td>\
    <td>"+(String)(today.johr/100)+":"+(String)(today.johr%100)+"</td>\
    <td>"+(String)(johr/100)+":"+(String)(johr%100)+"</td>\
  </tr>\
  <tr>\
    <td>Asr</td>\
    <td>"+(String)(today.asr/100)+":"+(String)(today.asr%100)+"</td>\
    <td>"+(String)(asr/100)+":"+(String)(asr%100)+"</td>\
  </tr>\
  <tr>\
    <td>Maghrib</td>\
    <td>"+(String)(today.sunset/100)+":"+(String)(today.sunset%100)+"</td>\
    <td>"+(String)(magrib/100)+":"+(String)(magrib%100)+"</td>\
  </tr>\
  <tr>\
    <td>Esha</td>\
    <td>"+(String)(today.esha/100)+":"+(String)(today.esha%100)+"</td>\
    <td>"+(String)(esha/100)+":"+(String)(esha%100)+"</td>\
  </tr>\
  <tr>\
    <td>Sunrise</td>\
    <td>"+(String)(today.sunrise/100)+":"+(String)(today.sunrise%100)+"</td>\
    <td>"+"--"+"</td>\
  </tr>\
  <tr>\
    <td>Sunset</td>\
    <td>"+(String)(today.sunset/100)+":"+(String)(today.sunset%100)+"</td>\
    <td>"+"--"+"</td>\
  </tr>\
</table></div>";
  webpage += "</body>";
  webpage += "</html>";
  server.send(200, "text/html", webpage); // Send a response to the client asking for input
  if (server.args() > 0 ) { // Arguments were received
    for ( uint8_t i = 0; i < server.args(); i++ ) {
      Serial.print(server.argName(i)); // Display the argument
      Argument_Name = server.argName(i);
      if (server.argName(i) == "jamat_name") {
        Serial.print(" Input received was: ");
        Serial.println(server.arg(i));
        Clients_Response2 = server.arg(i);


      }
      if (server.argName(i) == "jamat_hour") {
        Serial.print(" Input received was: ");
        Serial.println(server.arg(i));
        hours = server.arg(i);
      }
      if (server.argName(i) == "jamat_min") {
        Serial.print(" Input received was: ");
        Serial.println(server.arg(i));
        minutes = server.arg(i);
      }



    }

    String a = hours + minutes;
    int b = atoi(a.c_str());

    if (Clients_Response2 == "fajr")
    {

      fajr = b;
      Serial.println(1);

    }
    else if (Clients_Response2 == "johr")
    {

      johr = b;
      Serial.println(2);

    }
    else if (Clients_Response2 == "asr")
    {

      asr = b;
      Serial.println(3);

    }
    else if (Clients_Response2 == "magrib")
    {

      magrib = b;
      Serial.println(4);

    }
    else if (Clients_Response2 == "esha")
    {

      esha = b;
      Serial.println(5);

    }

  }
}



