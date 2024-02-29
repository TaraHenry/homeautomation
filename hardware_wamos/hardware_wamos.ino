
#include <SoftwareSerial.h>
// IMPORT ALL REQUIRED LIBRARIES
#include <NewPing.h>
#include <ArduinoJson.h>
#include <math.h>
   
#ifndef STDLIB_H
#include <stdlib.h>
#endif

#ifndef STDIO_H
#include <stdio.h>
#endif

#ifndef ARDUINO_H
#include <Arduino.h>
#endif 
//**********ENTER IP ADDRESS OF SERVER******************//

#define HOST_IP     ""       // REPLACE WITH IP ADDRESS OF SERVER ( IP ADDRESS OF COMPUTER THE BACKEND IS RUNNING ON) 
#define HOST_PORT   "8080"            // REPLACE WITH SERVER PORT (BACKEND FLASK API PORT)
#define route       "api/update"      // LEAVE UNCHANGED 
#define idNumber    "620154033"       // REPLACE WITH YOUR ID NUMBER 

// WIFI CREDENTIALS
#define SSID        ""      // "REPLACE WITH YOUR WIFI's SSID"   
#define password    ""  // "REPLACE WITH YOUR WiFi's PASSWORD" 

#define stay        100
 
//**********PIN DEFINITIONS******************//

#define espRX         10
#define espTX         11
#define espTimeout_ms 300
#define ARDUINOJSON_USE_DOUBLE 1

#define TRIGGER_PIN   3
#define ECHO_PIN      5

#define sensor_height 94.5
#define max_height 77.763
#define difference 16.737
#define radius 30.75

/* Declare your functions below */
void espSend(char command[] );
void espUpdate(char mssg[]);
void espInit();
double calcReserve(double height);
double calcWaterHeight(double radar);
 
SoftwareSerial esp(espRX, espTX);
NewPing sonar(TRIGGER_PIN, ECHO_PIN,1000); 

void setup(){

  Serial.begin(115200); 
  // Configure GPIO pins here
  pinMode(ECHO_PIN, INPUT);
  pinMode(TRIGGER_PIN, OUTPUT);

  espInit();  
 
}

void loop(){ 

  double radar= sonar.ping_in();// Speed of sound in inches per microsecond formula:
  double waterHeight = calcWaterHeight(radar); // Calculate water height
  double reserve = calcReserve(waterHeight); // Calculate water reserve
  double percentage = (waterHeight / max_height) * 100; // Calculate percentage
  
  // send updates with schema ‘{"id": "student_id", "type": "ultrasonic", "radar": 0, "waterheight": 0, "reserve": 0, "percentage": 0}’
  StaticJsonDocument<290> doc;
  char message[290]  = { 0 };
  
  doc["id"]            = "620154033";
  doc["type"]          = "ultrasonic";
  doc["radar"]         = radar;
  doc["waterheight"]   = waterHeight;
  doc["reserve"]       = reserve;
  doc["percentage"]    = percentage;
  
  serializeJson(doc, message); 
  
  espUpdate(message);
    
  delay(1000);
}
 
void espSend(char command[] ){   
    esp.print(command); // send the read character to the esp    
    while(esp.available()){ Serial.println(esp.readString());}    
}

void espUpdate(char mssg[]){ 
    char espCommandString[50] = {0};
    char post[290]            = {0};

    snprintf(espCommandString, sizeof(espCommandString),"AT+CIPSTART=\"TCP\",\"%s\",%s\r\n",HOST_IP,HOST_PORT); 
    espSend(espCommandString);    //starts the connection to the server
    delay(stay);

    // GET REQUEST 
    // snprintf(post,sizeof(post),"GET /%s HTTP/1.1\r\nHost: %s\r\nContent-Type: application/x-www-form-urlencoded\r\nContent-Length: %d\r\n\r\n%s\r\n\r\n",route,HOST_IP,strlen(mssg),mssg);

    // POST REQUEST
    snprintf(post,sizeof(post),"POST /%s HTTP/1.1\r\nHost: %s\r\nContent-Type: application/json\r\nContent-Length: %d\r\n\r\n%s\r\n\r\n",route,HOST_IP,strlen(mssg),mssg);
  
    snprintf(espCommandString, sizeof(espCommandString),"AT+CIPSEND=%d\r\n", strlen(post));
    espSend(espCommandString);    //sends post length
    delay(stay);
    Serial.println(post);
    espSend(post);                //sends POST request with the parameters 
    delay(stay);
    espSend("AT+CIPCLOSE\r\n");   //closes server connection
   }


void espInit(){
    char connection[100] = {0};
    esp.begin(115200); 
    Serial.println("Initiallizing");
    esp.println("AT"); 
    delay(1000);
    esp.println("AT+CWMODE=1");
    delay(1000);
    while(esp.available()){ Serial.println(esp.readString());} 

    snprintf(connection, sizeof(connection),"AT+CWJAP=\"%s\",\"%s\"\r\n",SSID,password);
    esp.print(connection);

    delay(3000);  //gives ESP some time to get IP

    if(esp.available()){   Serial.print(esp.readString());}
    
    Serial.println("\nFinish Initializing");    
   
}

//***** Design and implement all util functions below ******
double calcReserve(double height){
 // Convert height to gallons using the volume formula for a cylinder
  double volume = M_PI * pow(radius,2) * height / 231.0; // 231 cubic inches in a gallon
  return volume;
}

double calcWaterHeight(double radar){
  // Calculate water height using the radar measurement
  //double value = radar + difference ;
  return sensor_height - radar; // Placeholder, replace with actual calculation
}

