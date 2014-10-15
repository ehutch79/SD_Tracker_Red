
#include <Wire.h>
#include <SoftwareSerial.h>

#include <Adafruit_GPS.h>
#include <Adafruit_NeoPixel.h>
#include <LSM303.h>

//////////// CONGFIGURABLES ////////////////////////////////
String devName = String("red");
#define compassOffset -274
unsigned int baseSendDelay = 3000; // minimum delay between transmits of location.
unsigned int baseDisplayDelay = 3000;

//////////// COMS ////////////////////////////////
SoftwareSerial xbee(3, 2);
unsigned long lastSend = 0;
String inData = "";
String noFixString = "";
unsigned int sendDelay = baseSendDelay;

//////////// GPS ////////////////////////////////
SoftwareSerial gpsSerial(8, 7);
Adafruit_GPS GPS(&gpsSerial);
unsigned long lastGPS = 0;
unsigned int gpsFreq = 1000;

boolean blackFix = true;
float blackLat =   42.354046;
float blackLon = -71.128036;

boolean enemyFix = true;
float enemyLat =  42.700111;
float enemyLon = -70.995709;

boolean myFix = false;
float myLat = 42.701029;
float myLon = -70.991872;

//////////// COMPASS ////////////////////////////////
LSM303 compass;
unsigned int compassDelay = 200; // how often the compass reads
unsigned long lastCompassRead = 0;
float compassReading;

//////////// LIGHTS ////////////////////////////////
Adafruit_NeoPixel strip = Adafruit_NeoPixel(16, 4, NEO_GRB + NEO_KHZ800);
unsigned int ledsR[16] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
unsigned int ledsG[16] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
unsigned int ledsB[16] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
unsigned int ledMult = 100;
unsigned int displayDelay = baseDisplayDelay;
unsigned long lastDisplay = 0;

void setup() {

  Serial.begin(115200);

  noFixString = getNofixString(devName);
  xbee.begin(9600);
  delay(10);
  xbee.println(noFixString);

  GPS.begin(9600);
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);
  GPS.sendCommand(PGCMD_NOANTENNA);

  Wire.begin();
  compass.init();
  compass.enableDefault();
  compass.m_min.x = -617;
  compass.m_min.y = -836;
  compass.m_min.z = -852;
  compass.m_max.x = +583;
  compass.m_max.y = +221;
  compass.m_max.z = +223;

  delay(100);

  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
  delay(500);

}

void loop() {
  unsigned long timeStamp = millis();
  char received;
  
  if (!gpsSerial.isListening() && (timeStamp - lastGPS) > gpsFreq) {
    gpsSerial.listen();
    inData = "";
    delay(5);
  }

  if (gpsSerial.isListening()) {
    received = GPS.read();

    if (GPS.newNMEAreceived()) {
      char *stringptr = GPS.lastNMEA();

      if (GPS.parse(stringptr)) { // this also sets the newNMEAreceived() flag to false
        lastGPS = timeStamp;
        xbee.listen();
        Serial.print("Satellites: "); Serial.println((int)GPS.satellites);
        if (!GPS.fix) {
          myFix = true;
          myLat = decimalDegrees(GPS.latitude, GPS.lat);
          myLon = decimalDegrees(GPS.longitude, GPS.lon);
        } else {
          myFix = true;
          myLat = decimalDegrees(GPS.latitude, GPS.lat);
          myLon = decimalDegrees(GPS.longitude, GPS.lon);
        }
      }
    }
  }  // endif gpsSerial.isListening()

  if (xbee.isListening()) {
    
    while (xbee.available() > 0) {
      received = xbee.read();
      if (received != '\n' && received != '\r') { //ignore control characters
        inData += received;
      }
      if (received == '\n') { // Process message when new line character is recieved
        processSerial(inData);
        
        inData = ""; // Clear recieved buffer
      }
    }

  } //endif xbee.isListening();

  
  if ((timeStamp - lastDisplay) > displayDelay) {
    lastDisplay = timeStamp;

    if (blackFix) {
      int ledBlack = compassDirection(calcBearing(myLat, myLon, blackLat, blackLon));
      ledsG[ledBlack] = 250 * ledMult;      
    }

    if (enemyFix) {
      int ledEnemy = compassDirection(calcBearing(myLat, myLon, enemyLat, enemyLon));
      ledsR[ledEnemy] = 250 * ledMult;
    }
  }

  if ((timeStamp - lastSend) > sendDelay) {
    lastSend = timeStamp;
    if (myFix) {
      String locstring = getLocString(devName, myLat, myLon);
      Serial.println(locstring);
      xbee.println(locstring);
    } else {
      Serial.println(noFixString);
      xbee.println(noFixString);
    }
  }
  
  if ((timeStamp - lastCompassRead) > compassDelay) {
    compassReading = getHeading();
    lastCompassRead = timeStamp;
  
    int ledNorth = compassDirection(compassReading);
    if (myFix) {
      int j = ledNorth;
      for (int i = 0; i < 16; i++) {
        strip.setPixelColor(j, strip.Color(ledsR[i] / ledMult, ledsG[i] / ledMult, ledsB[i] / ledMult));
        if (j < 15) {
          j++;
        } else {
          j = 0;
        }
      }
      
    } else {
      for (int i = 0; i < 16; i++) {
        strip.setPixelColor(i, strip.Color(50, 0, 0));
      }
      strip.setPixelColor(ledNorth, strip.Color(0, 0, 100));
    }
    strip.show();

  }
  
  for(int i=0;i<16;i++) {
     if (ledsR[i] > 0) { ledsR[i]--; }
      if (ledsG[i] > 0) { ledsG[i]--; }
      if (ledsB[i] > 0) { ledsB[i]--; }
  }

  if (lastSend > timeStamp) {
    lastSend = timeStamp;
  }
  if (lastDisplay > timeStamp) {
    lastDisplay = timeStamp;
  }
  if (lastCompassRead > timeStamp) {
    lastCompassRead = timeStamp;
  }

}
