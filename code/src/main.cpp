#include "motor.h"
#include <ESP8266WebServerSecure.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <string.h>

#include "deBounce.h"
#include "Button.h"

#include "config.h"

#define OPEN_PIN D6
#define CLOSED_PIN D5
#define PUSH_PIN D8
#define LED_PIN D3
#define SENSOR_PIN D7

TMC2208Stepper driver(NULL, R_SENSE);

Button switchClosed (CLOSED_PIN, 50, INPUT);
Button switchOpen (OPEN_PIN, 50, INPUT);
Button pushButton(PUSH_PIN, 50, INPUT_PULLUP);
Button doorSensor(SENSOR_PIN, 50, INPUT);

#define HTTP_REST_PORT 8080
ESP8266WebServerSecure httpsRestServer(8083);


#define DIR_CLOCKWISE LOW
#define DIR_COUNTERCLOCKWISE HIGH
#define MAX_MOTOR_STEPS 2000

bool turnMotorUntilLimit(int direction, Button button)
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

  //Add a couple just to engage fully the limit switch
  for(int i=0; i<50; i++)
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

String unlock(){
  String response = "{";
  response+=" \"action\": \"unlock\"";

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

String lock(){
  String response = "{";
  response+=" \"action\": \"lock\"";

  bool result; 
  
  /*if(doorSensor.read()==LOW)
  {
    Serial.println("Cannot lock if door is open");
    response+=", \"status\": \"failure\"";
    response+=", \"reason\": \"Door\"";
    response+="}";
    return response;
  }*/
  if(switchClosed.read()==HIGH)
  {
    response+=", \"status\": \"success\"";
    response+="}";
    return response;
  }

  result = turnMotorUntilLimit(DIR_CLOCKWISE, switchClosed);
  response+=", \"status\": ";
  if(result)
    response+="\"success\"";
  else{
    response+="\"failure\"";
    response+=", \"reason\": \"Motor\"";
  }
  response+="}";
  //httpRestServer.send(200, F("text/json"), response);
  return response;
}

bool authenticate()
{
  if(!httpsRestServer.hasArg("token"))
    return false;
  
  if(httpsRestServer.arg("token").compareTo(authenticationToken)!=0)
    return false;
  
  return true;
}

void restServerRouting() {
    httpsRestServer.on("/", HTTP_GET, []() {
        httpsRestServer.send(404, F("text/html"),
            F("Nope."));
    });
    httpsRestServer.on(F("/open"), HTTP_GET, [](){
      String result; 
      if (!authenticate()) 
        result = F("{\"status\": \"failure\", \"reason\":\"SysUnavailable\"}");
      else
        result = unlock();
      httpsRestServer.send(200, F("text/json"), result);
    });

    httpsRestServer.on(F("/close"), HTTP_GET, [](){
      String result;
      if (!authenticate()) 
        result = F("{\"status\": \"failure\", \"reason\":\"SysUnavailable\"}");
      else
        result = lock();
      httpsRestServer.send(200, F("text/json"), result);
    });

    httpsRestServer.on(F("/setState"), HTTP_GET, [](){
      String result;
      String on("1");
      if(httpsRestServer.arg("value").compareTo(on)==0)
      {
        lock();
        result = "{\"currentState\": 1}";
      } else{
        unlock();
        result = "{\"currentState\": 0}";
      }
      httpsRestServer.send(200, F("text/json"), result);
    });
    httpsRestServer.on(F("/status"), HTTP_GET, [](){
      String result;
      result = "{\"currentState\": ";
      result+= doorSensor.read()==HIGH && switchClosed.read()==HIGH;
      result+= "}";
      httpsRestServer.send(200, F("text/json"), result);
    });
}

unsigned int lastLedState = LOW;
unsigned long lastLedStateChange = millis();
void setLedState()
{
  unsigned int newState;
  if(doorSensor.read()==HIGH && switchClosed.read()==HIGH)
  {
    newState = LOW;
    //digitalWrite(LED_PIN, HIGH);
  }
  else {
    //digitalWrite(LED_PIN, LOW);
    newState = HIGH;
  }
  if(newState!=lastLedState && millis()-lastLedStateChange > 10){
    lastLedState = newState;
    lastLedStateChange = millis();
    digitalWrite(LED_PIN, newState);
  }
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
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());


  configTime(5 * 3600, 0, "pool.ntp.org", "time.nist.gov");



  pinMode(LED_PIN, OUTPUT);
  setLedState();

  // Activate mDNS this is used to be able to connect to the server
  // with local DNS hostmane esp8266.local
  if (MDNS.begin("SmartLock")) {
    Serial.println("MDNS responder started");
  }

  setupMotor(driver);


  restServerRouting();

   // Start server
  httpsRestServer.getServer().setRSACert(new BearSSL::X509List(serverCert), new BearSSL::PrivateKey(serverKey));
  httpsRestServer.begin();
  Serial.println("HTTP server started");
}


#define DELAYED_TIMER 10000
unsigned long delayedLock = 0;

unsigned int pushPinState = -1;


void loop() {
  httpsRestServer.handleClient();
  MDNS.update();

  //Serial.print("Door Sensor: ");
  //Serial.print(doorSensor.read());
  //Serial.print(" | Push button: ");
  //Serial.println(pushButton.read());
  //Serial.print("Switch closed: ");
  //Serial.println(switchClosed.read());


  setLedState();

  //Door closed and button pushed
  if(doorSensor.read()==HIGH && pushButton.read()==HIGH)
  {
    lock();
  }
  //We are not on a delayed locking sequence, door is open and button has been long pressed
  else if(!delayedLock && doorSensor.read()==LOW && pushButton.longPress(2000))
  {
    //Blink the led
    for(int i=0; i<20; i++)
    {
      digitalWrite(LED_PIN, HIGH);
      delay(100);
      digitalWrite(LED_PIN, LOW);
      delay(100);
    }
    setLedState();
    //Initiate delayed lock sequence
    delayedLock = millis();
  }
  else if(delayedLock && doorSensor.read()==HIGH && millis() - delayedLock >= 5000)
  {
    lock();
    delayedLock = 0;
  }
  
}
