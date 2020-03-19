#include <Arduino.h>
#include <SoftwareSerial.h>
#include <EasyTransfer.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <TinyGPS++.h>
#include "defaulthtml.h"

// COM Serial Setup
SoftwareSerial TSerial(5, 4); // RX,TX (5,4) = D1,D2
int comInterval = 10000; // Check every 10 secs.
unsigned int comCheck = 0; // Next service timer.
EasyTransfer ETIn, ETOut;
struct ESP_DATA{ //put your variable definitions here for the data you want to receive
  double destLon;
  double destLat;
};
struct TEENSY_DATA{ //put your variable definitions here for the data you want to receive
  float battPct;
  byte mode;
  double myLon;
  double myLat;
  double course;
};
ESP_DATA espData;
TEENSY_DATA tnsyData;

// GPS Monitor Setup
//double distance = 0;
TinyGPSPlus gps;
double courseTo = 0;
double course = 0;
double distance = 0;

// Web Server Setup
#include "secrets.h"
IPAddress apLocal(192,168,4,1);
IPAddress apGW(192,168,4,1);
IPAddress apMask(255,255,255,0); // 6 hosts max or a /8
IPAddress clLocal(10,10,31,70);
IPAddress clGW(10,10,31,1);
IPAddress clMask(255,255,255,128); // 6 hosts max or a /8
ESP8266WebServer webServ(80);
bool webOn = true;

char const * getMode(byte modeVal) {
  switch (modeVal) { //0=setup, 1=findcache, 2=headlight walking, 3=headlight warn walking, 4= area light, 
  case 0: return "Setup";
  break;
  case 1: return "Find Cache";
  break;
  case 2: return "Walking";
  break;
  case 3: return "Walking Warn";
  break;
  case 4: return "Area Light";
  break;
  default: return "mode error";
  }
}

void handle_OnConnect() {
  Serial.println("Incoming Default Page Client.");
  char value[12]="";
  char htmlIndexAll[1100]; // create one big string (watch the page size!)
  strcpy_P(htmlIndexAll, PAGE_DefaultTop); // pull the static 1st part into the big string
  dtostrf(espData.destLat,4,6,value);
  strcat(htmlIndexAll, value); // pull the value into the big string
  strcat(htmlIndexAll, "'><br /><label>Lon: </label><input type='text' id='lon' name='lon' value='"); // Close the first field, start next.
  dtostrf(espData.destLon,4,6,value);
  strcat(htmlIndexAll, value); // pull the value into the big string
  strcat(htmlIndexAll, "'><br /><input type='submit' value='Update'> <input type='button' value='Copy My Location' onclick='copyLoc(this.form)'></form><br /><label>Mode:</label>");
  strcat(htmlIndexAll, getMode(tnsyData.mode));
  strcat(htmlIndexAll, "<br /><label>Battery: </label>");
  dtostrf(tnsyData.battPct,3,2,value);
  strcat(htmlIndexAll, value); // pull the value into the big string
  strcat(htmlIndexAll, "<br /><label>My Lat: </label><input id='mylat' readonly='readonly' value='");
  dtostrf(tnsyData.myLat,4,6,value);
  strcat(htmlIndexAll, value); // pull the value into the big string
  strcat(htmlIndexAll, "'></input><br /><label>My Lon: </label><input id='mylon' readonly='readonly' value='");
  dtostrf(tnsyData.myLon,4,6,value);
  strcat(htmlIndexAll, value); // pull the value into the big string
  courseTo = gps.courseTo(tnsyData.myLat,tnsyData.myLon,espData.destLat,espData.destLon);
  distance = gps.distanceBetween(tnsyData.myLat,tnsyData.myLon,espData.destLat,espData.destLon) / 1000.0;
  strcat(htmlIndexAll, "'></input><br /><label>Distance: </label>");
  dtostrf(distance,6,2,value);
  strcat(htmlIndexAll, value); // pull the value into the big string
  strcat(htmlIndexAll," M<br/>Cardinal To: ");
  strcat(htmlIndexAll, gps.cardinal(courseTo));
  strcat(htmlIndexAll," <br/>Present Course: ");
  strcat(htmlIndexAll, gps.cardinal(course));
  strcat_P(htmlIndexAll, PAGE_DefaultBot);
  webServ.send(200, "text/html", htmlIndexAll); 
}
void handle_NotFound() {
  webServ.send(404, "text/plain", "404: Not found");
}
void handle_newCoords() {
  double sentLon;
  double sentLat;
  bool argError = true;
  String message = "";
  if ((webServ.hasArg("lat")) && (webServ.hasArg("lon"))) {
    // check for proper format? double, limits on range.
    sentLat = webServ.arg("lat").toDouble();
    sentLon = webServ.arg("lon").toDouble();
    sentLat = abs(sentLat) > 90 ? 0 : sentLat;
    sentLon = abs(sentLon) > 180 ? 0 : sentLon;
    if ((sentLon != 0 ) && (sentLat != 0)) {
      argError = false;
    } else {
      message = "Latitude or Longitude is invalid or out of range.";
    }
  } else {
    message = "Missing Latitude or Longitude.";
  }
  if (!argError) {
    // change values here.
    espData.destLat = sentLat;
    espData.destLon = sentLon;
    char htmlIndexAll[300]; // create one big string (watch the page size!)
    strcpy_P(htmlIndexAll, PAGE_UpdateOK); // pull the header into the big string
    webServ.send(200, "text/html", htmlIndexAll); 
  }
  webServ.send(200, "text/plain", message);
}
void handle_getCourse() {
  webServ.send(200, "text/plain", "N");
}

void setup() {
  Serial.begin(19200);
  Serial.println("");
  Serial.println("Starting The8266Stick.");
  TSerial.begin(19200);
  ETIn.begin(details(tnsyData),&TSerial);
  ETOut.begin(details(espData),&TSerial);
  espData.destLon = 1.000000; // Test data - close.
  espData.destLat = 1.000000;
  WiFi.hostname("TheStick");
  WiFi.begin(clSSID,clPass);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  //WiFi.softAP(apSSID, apPass);
  //WiFi.softAPConfig(apLocal, apGW, apMask);
  delay(100);
  webServ.on("/", handle_OnConnect);
  webServ.on("/to", handle_newCoords);
  webServ.on("/crs", handle_getCourse);
  webServ.onNotFound(handle_NotFound);
  webServ.begin();
}

void loop() {
  // First see if its time to process subs.
   if (webOn) {
     webServ.handleClient();
   }

   if (millis() > comCheck) {     // Time to service GPS.
    comCheck = millis() + comInterval;
    ETOut.sendData();
  }

  // End of event servicing.
  while (TSerial.available() > 0) { // Teensy Data is ready.
    if (ETIn.receiveData()) {
      Serial.println("");
      Serial.println("New Teensy Data Receieved.");
      switch (tnsyData.mode) { //0=setup, 1=findcache, 2=headlight walking, 3=headlight warn walking, 4= area light, 
        case 0:
        // Setup Mode
            Serial.println("Setup Mode");
        break;
        case 1:
        // FindCache Mode
            Serial.println("FindCache Mode");
        break;
        case 2:
        // Headlight Walking Mode
            Serial.println("Headlight Walking Mode");
        break;
        case 3:
        // Headlight Warn Mode
            Serial.println("Headlight Warn Mode");
        break;
        case 4:
        // Area Light Mode
            Serial.println("Area Light Mode");
        break;
        default:
          Serial.println("Mode Select Statement error.");
      }
      Serial.print("Lat: ");
      Serial.println(tnsyData.myLat);
      Serial.print("Lon: ");
      Serial.println(tnsyData.myLon);
      Serial.print("Course: ");
      Serial.println(tnsyData.course);
      Serial.print("Battery: ");
      Serial.println(tnsyData.battPct);
    }
  }
}