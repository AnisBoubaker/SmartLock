/**
 * Author Teemu Mäntykallio
 * Initializes the library and runs the stepper
 * motor in alternating directions.
 */

#include "motor.h"
#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>

#include "deBounce.h"

#define OPEN_PIN D6
#define CLOSED_PIN D5
#define PUSH_PIN D7

TMC2208Stepper driver(NULL, R_SENSE);

DebounceButton switchClosed (CLOSED_PIN, 50, INPUT_PULLUP);
DebounceButton switchOpen (OPEN_PIN, 50, INPUT_PULLUP);
DebounceButton pushButton(PUSH_PIN, 500, INPUT_PULLUP);

#define HTTP_REST_PORT 8080
ESP8266WebServer httpRestServer(HTTP_REST_PORT);

const char* ssid = "Boubagou";
const char* password = "bichanis";

#define DIR_CLOCKWISE LOW
#define DIR_COUNTERCLOCKWISE HIGH
#define MAX_MOTOR_STEPS 2000

bool turnMotorUntilLimit(int direction, DebounceButton button)
{
  //Enable stepper
  digitalWrite(EN_PIN, LOW);
  
  //Set direction
  digitalWrite(DIR_PIN, direction);

  int counter = 0;

  //Turn until limit switch is activated
  while(button.read()==LOW && counter<MAX_MOTOR_STEPS)
  {
    digitalWrite(STEP_PIN, HIGH);
    delayMicroseconds(600);
    digitalWrite(STEP_PIN, LOW);
    delayMicroseconds(600);
    yield();
    counter++;
  }

  //Disable stepper
  digitalWrite(EN_PIN, HIGH);

  return counter<MAX_MOTOR_STEPS;
}

String openDoor(){
  String response = "{";
  response+=" \"action\": \"openDoor\"";

  bool result; 
  
  result = turnMotorUntilLimit(DIR_COUNTERCLOCKWISE, switchOpen);
  response+=", \"status\": ";
  if(result)
    response+="\"success\"";
  else
    response+="\"failure\"";

  response+="}";
  //httpRestServer.send(200, F("text/json"), response);
  return response;
}

String closeDoor(){
  String response = "{";
  response+=" \"action\": \"closeDoor\"";

  bool result; 
  
  result = turnMotorUntilLimit(DIR_CLOCKWISE, switchClosed);
  response+=", \"status\": ";
  if(result)
    response+="\"success\"";
  else
    response+="\"failure\"";

  response+="}";
  //httpRestServer.send(200, F("text/json"), response);
  return response;
}
 
void restServerRouting() {
    httpRestServer.on("/", HTTP_GET, []() {
        httpRestServer.send(200, F("text/html"),
            F("Welcome to the REST Web Server"));
    });
    //httpRestServer.on(F("/open"), HTTP_GET, openDoor);
    //httpRestServer.on(F("/close"), HTTP_GET, closeDoor);

    httpRestServer.on(F("/open"), HTTP_GET, [](){
      String result; 
      result = openDoor();
      httpRestServer.send(200, F("text/json"), result);
    });

    httpRestServer.on(F("/close"), HTTP_GET, [](){
      String result; 
      result = closeDoor();
      httpRestServer.send(200, F("text/json"), result);
    });
}


void setup() {
  //SPI.begin();                    // SPI drivers
  SERIAL_PORT.begin(115200);      // HW UART drivers
  Serial.begin(115200);


  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected tow ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // Activate mDNS this is used to be able to connect to the server
  // with local DNS hostmane esp8266.local
  if (MDNS.begin("SmartLock")) {
    Serial.println("MDNS responder started");
  }


  setupMotor(driver);


  restServerRouting();
  httpRestServer.begin();

   // Start server
  httpRestServer.begin();
  Serial.println("HTTP server started");

}


#define DELAYED_TIMER 10000
unsigned long delayedLock = 0;


void loop() {
  httpRestServer.handleClient();
  MDNS.update();

  if(pushButton.read()==HIGH)
  {
    delayedLock = millis();
  }

  if(delayedLock && millis()-delayedLock >= DELAYED_TIMER)
  {
    closeDoor();
    delayedLock = 0;
  }

}
