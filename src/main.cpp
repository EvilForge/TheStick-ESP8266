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
TinyGPSPlus gps;
double courseTo = 0;
double course = 0;
double distance = 0;

// Web Server Setup
#include "secrets.h" // Passwords, SSIDs.
IPAddress apLocal(192,168,4,1); // Values for AP mode
IPAddress apGW(192,168,4,1);
IPAddress apMask(255,255,255,0); 
IPAddress clLocal(10,10,31,70); // Values for wifi Client mode
IPAddress clGW(10,10,31,1);
IPAddress clMask(255,255,255,128);
ESP8266WebServer webServ(80);

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
  char value[12]="";
  char htmlIndexAll[1600]; // create one big string (watch the page size!)
  strcpy_P(htmlIndexAll, PAGE_DefaultTop); // pull the static 1st part into the big string
  dtostrf(espData.destLat,4,6,value);
  strcat(htmlIndexAll, value); // pull the value into the big string
  strcat(htmlIndexAll, "'><br><label for='lon'>Longitude</label><input type='text' id='lon' name='lon' value='"); // Close the first field, start next.
  dtostrf(espData.destLon,4,6,value);
  strcat(htmlIndexAll, value); // pull the value into the big string
  strcat(htmlIndexAll, "'><br><input type='submit' value='Update'><input type='button' value='Copy' onclick='copyLoc(this.form)'></fieldset></form></div><h2>Status</h2><div><fieldset><label>Mode</label><label>");
  strcat(htmlIndexAll, getMode(tnsyData.mode));
  strcat(htmlIndexAll, "</label><br><label>Battery</label><label>");
  dtostrf(tnsyData.battPct,3,2,value);
  strcat(htmlIndexAll, value); // pull the value into the big string
  strcat(htmlIndexAll, " V</label><br><label>My Lat</label><input id='mylat' readonly value='");
  dtostrf(tnsyData.myLat,4,6,value);
  strcat(htmlIndexAll, value); // pull the value into the big string
  strcat(htmlIndexAll, "'><br><label>My Lon</label><input id='mylon' readonly value='");
  dtostrf(tnsyData.myLon,4,6,value);
  strcat(htmlIndexAll, value); // pull the value into the big string
  courseTo = gps.courseTo(tnsyData.myLat,tnsyData.myLon,espData.destLat,espData.destLon);
  distance = gps.distanceBetween(tnsyData.myLat,tnsyData.myLon,espData.destLat,espData.destLon) / 1000.0;
  strcat(htmlIndexAll, "'><br><label>Distance</label><label>");
  dtostrf(distance,6,2,value);
  strcat(htmlIndexAll, value); // pull the value into the big string
  strcat(htmlIndexAll," m</label><br><label>Cardinal To</label><label>");
  strcat(htmlIndexAll, gps.cardinal(courseTo));
  strcat(htmlIndexAll,"</label><br><label>Course</label><label>");
  strcat(htmlIndexAll, gps.cardinal(course));
  strcat(htmlIndexAll, "</label></fieldset></div></body></html>");
  webServ.send(200, "text/html", htmlIndexAll); 
  Serial.println("WEB-SERVED: Config page.");
}
void handle_NotFound() {
  webServ.send(404, "text/plain", "404: Not found.");
  Serial.println("WEB-SERVED: 404.");
}
void handle_newCoords() {
  double sentLon;
  double sentLat;
  bool argError = true;
  char const * message = "";
  if ((webServ.hasArg("lat")) && (webServ.hasArg("lon"))) {
    // check for proper format? double, limits on range.
    sentLat = webServ.arg("lat").toDouble();
    sentLon = webServ.arg("lon").toDouble();
    sentLat = abs(sentLat) > 90 ? 0 : sentLat;
    sentLon = abs(sentLon) > 180 ? 0 : sentLon;
    if ((sentLon != 0 ) && (sentLat != 0)) {
      argError = false;
    } else {
      message = "Latitude or Longitude is invalid or out of range.</p></body></html>";
    }
  } else {
    message = "Missing Latitude or Longitude.</p></body></html>";
  }
  if (argError) {
    char htmlIndexAll[300]; // create one big string (watch the page size!)
    strcpy_P(htmlIndexAll, PAGE_Update400); // pull the header into the big string
    strcat(htmlIndexAll, message);
    webServ.send(400, "text/html", htmlIndexAll); 
    Serial.println("WEB-SERVED: Coordinate update FAIL.");
  } else {
    // change values here.
    espData.destLat = sentLat;
    espData.destLon = sentLon;
    char htmlIndexAll[300]; // create one big string (watch the page size!)
    strcpy_P(htmlIndexAll, PAGE_UpdateOK); // pull the header into the big string
    webServ.send(200, "text/html", htmlIndexAll); 
    Serial.println("WEB-SERVED: Coordinates updated.");
    ETOut.sendData();
  }
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
  WiFi.begin(clSSID,clPass); // Comment this line thru serial.println to disable Wifi Client.
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
//  WiFi.softAP(apSSID, apPass); // Comment these softAP lines to disable Access point.
//  WiFi.softAPConfig(apLocal, apGW, apMask);
  delay(100);
  webServ.on("/", handle_OnConnect);
  webServ.on("/to", handle_newCoords);
  webServ.onNotFound(handle_NotFound);
  webServ.begin();
}

void loop() {
  // First see if its time to process subs.
   webServ.handleClient();
  // End of event servicing.

  while (TSerial.available() > 0) { // Teensy Data is ready.
    if (ETIn.receiveData()) {
      Serial.println("");
      Serial.println("DATA-RECEIVED: Teensy");
      Serial.print("Mode: ");
      Serial.println(getMode(tnsyData.mode));
      Serial.print("Position: ");
      Serial.print(tnsyData.myLat,6);
      Serial.print(", ");
      Serial.println(tnsyData.myLon,6);
      Serial.print("Course: ");
      Serial.println(tnsyData.course);
      Serial.print("Battery: ");
      Serial.println(tnsyData.battPct);
    }
  }
}