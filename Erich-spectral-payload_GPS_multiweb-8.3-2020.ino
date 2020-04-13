/*
  spectral-payload_GPS.ino creates a WiFi access point and sends counts per second and GPS data over UDP on a ESP32 device
  Steps:
  1. Connect to the access point "spectral"
  2. sends data over UDP on a ESP32 device
  netcat -ul 3333
  netcat -ul 3333 > output.txt
  netcat -ul 3333 | tee output.txt
*/

// the number of the LED pin
const int ledPin = 13;  // 13 corresponds to internal led

// setting PWM properties
const int freq = 5;
const int ledChannel = 0;
const int resolution = 8;
int dutyCycle = 240;

bool dataRecording = false;
bool restart = false;

//SPIFFS
#include "SPIFFS.h"
/* You only need to format SPIFFS the first time you run a
   test or else use the SPIFFS plugin to create a partition
   https://github.com/me-no-dev/arduino-esp32fs-plugin */
#define FORMAT_SPIFFS_IF_FAILED true
String stringdata;
int intdata;
char stringbuffer[4];
char* filepostfix = "Spectrum.txt";
char newfilename[31];
char cpsString [10];
char gpsnofixString [23];
char gpsfixString [58];

int fixdateflag = 0;
int nofixdateflag = 0;

// GPS
#include <Adafruit_GPS.h>
#define GPSSerial Serial1
Adafruit_GPS GPS(&GPSSerial);
uint32_t timer = millis();
// Set GPSECHO to 'false' to turn off echoing the GPS data to the Serial console
// Set to 'true' if you want to debug and listen to the raw GPS sentences
#define GPSECHO false

// WIFI
#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <WiFiAP.h>
#include "index.h" 
//WEBSERVER Set these to your desired credentials.
const char* ssid = "spectral";
const char* password = "spectral";
const char* host = "esp32fs";
//const char* udpAddress1 = "192.168.4.2";
//const char* udpAddress2 = "192.168.4.3";
//const char* udpAddress3 = "192.168.4.4";
//const int udpPort = 3333;
//WiFiUDP udp;
//WiFiServer udpserver(3333);
WebServer webserver(80);

//Interrupts


const byte DRAM_ATTR interruptPin = 21;
volatile int interruptCounter = 0;
portMUX_TYPE DRAM_ATTR mux = portMUX_INITIALIZER_UNLOCKED;

void IRAM_ATTR handleInterrupt() {

  portENTER_CRITICAL_ISR(&mux);
  interruptCounter++;
  portEXIT_CRITICAL_ISR(&mux);

}

hw_timer_t * timerCPS = NULL;
portMUX_TYPE DRAM_ATTR timerCPSMux = portMUX_INITIALIZER_UNLOCKED;
volatile int cps = 0; // holds cps for one second to be called from main loop

void IRAM_ATTR onCPSTimer(){
 
  // Increment the counter and set the time of ISR
  portENTER_CRITICAL_ISR(&timerCPSMux);
  cps = interruptCounter;
  interruptCounter =0;
  portEXIT_CRITICAL_ISR(&timerCPSMux);
 
}

void setup() {


  // configure LED PWM functionalitites
  ledcSetup(ledChannel, freq, resolution);
  
  // attach the channel to the GPIO to be controlled
  ledcAttachPin(ledPin, ledChannel);

  //SERIAL

  Serial.begin(115200);
  Serial.println();
  Serial.println("Configuring access point...");

  //WIFI
  // You can remove the password parameter if you want the AP to be open.
  WiFi.softAP(ssid, password);
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);
  
  //Interrupt
  pinMode(interruptPin, INPUT);
  attachInterrupt(digitalPinToInterrupt(interruptPin), handleInterrupt, RISING);

  // Use 1st timer of 4 (counted from zero).
  // Set 80 divider for prescaler (see ESP32 Technical Reference Manual for more
  // info).
  timerCPS = timerBegin(0, 80, true);

  // Attach onTimer function to our timer.
  timerAttachInterrupt(timerCPS, &onCPSTimer, true);

  // Set alarm to call onTimer function every second (value in microseconds).
  // Repeat the alarm (third parameter)
  timerAlarmWrite(timerCPS, 1000000, true);

  // Start an alarm
  timerAlarmEnable(timerCPS);

  //GPS
  // 9600 NMEA is the default baud rate for Adafruit MTK GPS's- some use 4800
  GPS.begin(9600);
  // uncomment this line to turn on RMC (recommended minimum) and GGA (fix data) including altitude
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
  // uncomment this line to turn on only the "minimum recommended" data
  //GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCONLY);
  // For parsing data, we don't suggest using anything but either RMC only or RMC+GGA since
  // the parser doesn't care about other sentences at this time
  // Set the update rate
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ); // 1 Hz update rate
  // For the parsing code to work nicely and have time to sort thru the data, and
  // print it out we don't suggest using anything higher than 1 Hz
  // Request updates on antenna status, comment out to keep quiet
  //GPS.sendCommand(PGCMD_ANTENNA);

  delay(1000);

  // Ask for firmware version
  GPSSerial.println(PMTK_Q_RELEASE);
  
  //SPIFFS

  if(!SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED)){

          Serial.println("SPIFFS Mount Failed");
          return;
    }
    
  SPIFFS.begin();
  
  SPIFFScreateInitFile("/SpectralInit.txt");
  stringdata = SPIFFSreadFile("/SpectralInit.txt");
//  Serial.println (stringdata);
  intdata = stringdata.toInt();
//  Serial.println (intdata);
  intdata++;
//  Serial.println (intdata);
  itoa(intdata,stringbuffer, 10); 
  SPIFFSwriteFile("/SpectralInit.txt",stringbuffer);
  stringdata = SPIFFSreadFile("/SpectralInit.txt");
//  Serial.println (stringdata);
//  Serial.println(stringbuffer); 
//  Serial.println(filepostfix); 

  sprintf(newfilename,"/%s%s",stringbuffer,filepostfix); 
//  Serial.println(newfilename); 
  SPIFFSwriteFile(newfilename,""); //create new data file

 //WIFI INIT
 
  MDNS.begin(host);
  Serial.print("Open http://");
  Serial.print(host);
  Serial.println(".local/edit to see the file browser");

  //SERVER INIT
  webserver.on("/readCPS", handleCPS);
  webserver.on("/readGPSfix", handleGPSfix);
  webserver.on("/readBattery", handleBattery);
  webserver.on("/readFSspace", handleFSspace);
  webserver.on("/", handle_OnConnect);
  webserver.on("/dataRecordingOn", handle_dataRecordingOn);
  webserver.on("/dataRecordingOff", handle_dataRecordingOff);
  webserver.on("/restartOn", handle_restartOn);
  webserver.on("/restartOff", handle_restartOff);
  webserver.onNotFound(handle_NotFound);
  //list directory
  webserver.on("/list", HTTP_GET, handleFileList);
  //load editor
  webserver.on("/edit", HTTP_GET, []() {
    if (!handleFileRead("/edit.htm")) {
      webserver.send(404, "text/plain", "FileNotFound");
    }
  });
  //create file
//  webserver.on("/edit", HTTP_PUT, handleFileCreate);
  //delete file
    webserver.on("/edit", HTTP_DELETE, handleFileDelete);
  //first callback is called after the request has ended with all parsed arguments
  //second callback handles file uploads at that location
//  webserver.on("/edit", HTTP_POST, []() {
//    webserver.send(200, "text/plain", "");
//  }, handleFileUpload);
  //called when the url is not defined here
  //use it to load content from FILESYSTEM
  webserver.onNotFound([]() {
    if (!handleFileRead(webserver.uri())) {
      webserver.send(404, "text/plain", "FileNotFound");
    }
  });
  
  webserver.begin();
  Serial.println("HTTP server started");
  
  //udpserver.begin();
  //Serial.println("UDP Server started");

  timer = millis();

}

void loop() {

  ledcWrite(ledChannel, dutyCycle);

  if (timer > millis()) timer = millis();
  
  while (millis() - timer < 1000) {
     GPS.read(); 
     webserver.handleClient();
     
     delay(1);
  }

  timer = millis(); // reset the timer

  if (GPS.newNMEAreceived()) {    
    if (!GPS.parse(GPS.lastNMEA())) // this also sets the newNMEAreceived() flag to false
      return; // we can fail to parse a sentence in which case we should just wait for another
  }

    //dumpGPStoSerial(); //debug GPS on serial monitor
     
    Serial.print("CPS: ");
    Serial.println(cps, DEC);

    Serial.print("Fix: "); 
    Serial.println((int)GPS.fix);
    Serial.println(GPS.latitude);
    Serial.println(convlat());
    Serial.println(GPS.longitude);
    Serial.println(convlon());

    //sendUDP();
    
    if (dataRecording) {
      
      dutyCycle = 1;

      writeDataToSPIFFS (); 
    }

    else {
        dutyCycle = 240;
      }
    
    if (restart) {
      ESP.restart();
    }
    
}
