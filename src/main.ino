//#include <Arduino.h>
#include <M5Stack.h>
#include "wii_i2c.h"
#include <Ramp.h>

#include <WiFi.h>

#include <ArduinoJson.h>
#include "FS.h"
#include "SPIFFS.h"

struct Config {
  char cartname[64];
  char ssid[64];
  char passwd[64];
  int speed_max;
  int speed_min;
  int steer_max;
  int steer_min;
  int accel_min;
  int boost_max;
};

struct Config2 {
  char cartname[64];
  char ssid[64];
  char passwd[64];
  int speed_max;
  int speed_min;
  int steer_max;
  int steer_min;
  int accel_min;
  int boost_max;
};

struct Config3 {
  char cartname[64];
  char ssid[64];
  char passwd[64];
  int speed_max;
  int speed_min;
  int steer_max;
  int steer_min;
  int accel_min;
  int boost_max;
};

const char *filename = "/config.json";
Config config;
Config2 config2;
Config3 config3;

const char* ssid = config.ssid;
const char* password = config.passwd;

#define TaskStackSize   5120
//#include "lvgl.h"
//#include "blejoypad.h"
#define PIN_SDA 21
#define PIN_SCL 22
#define WII_I2C_PORT 0
#define READ_TASK_CPU 0
#define READ_DELAY 30
static unsigned int controller_type;

#define HOVER_SERIAL_BAUD   115200      // [-] Baud rate for HoverSerial (used to communicate with the hoverboard)
#define SERIAL_BAUD         115200      // [-] Baud rate for built-in Serial (used for the Serial Monitor)
#define START_FRAME         0xABCD     	// [-] Start frme definition for reliable serial communication
#define TIME_SEND           100         // [ms] Sending time interval

//#define DEBUG_RX                        // [-] Debug received data. Prints all bytes to serial (comment-out to disable)
HardwareSerial &HoverSerial = Serial2;

// Global variables
uint8_t idx = 0;                        // Index for new data pointer
uint16_t bufStartFrame;                 // Buffer Start Frame
byte *p;                                // Pointer declaration for the new received data
byte incomingByte;
byte incomingBytePrev;
boolean motorOn = false;
boolean triggerstate = false;
boolean switchState = false;
boolean useNunchuk = true;

/* Telemetry */
int16_t driveSpeed = 0;
int16_t speedR = 0;
int16_t speedL = 0;
int16_t batVoltage = 0;
int16_t boardTemp = 0;

boolean triggerReleased = true;
int configNum = 0;

rampInt upRamp;
rampInt downRamp;

int leftRightCalibration = 0;
int forwardReverseCalibration = 0;
int thresholdMovement = 100;
int leftRightValue = 0;
int forwardReverseValue = 0;
int OLDleftRightValue = 0;
int OLDforwardReverseValue = 0;
int accel = config.accel_min; // Acceleration time [ms]
int safetyCool = 10;
int myDrive = 0;
int oldmyDrive = 0;

typedef struct{
   uint16_t start;
  //uint16_t dpitch; // sideboard usart
  //uint16_t pitch; // sideboard usart
   int16_t  steer;
   int16_t  speed;
  //uint16_t sensors; 
  // 0:OFF or 1:ON for switching input when dual input is used
  // controltype; 
  // 00:FOC 10:Sinusoidal 01:Commutation
  // controlmode; 
  // 00:Voltage 10:Speed 01:Torque
  // fieldweakening; 
  // 0:off 1:on
    uint16_t checksum;
} SerialCommand;
SerialCommand Command;

typedef struct{
   uint16_t start;
   int16_t  cmd1;
   int16_t  cmd2;
   int16_t  speedR_meas;
   int16_t  speedL_meas;
   int16_t  batVoltage;
   int16_t  boardTemp;
   uint16_t cmdLed;
   uint16_t checksum;
} SerialFeedback;
SerialFeedback Feedback;
SerialFeedback NewFeedback;

#include "lvgl_start.h"
#include "configload.h"
#include "display.h"
// ########################## SETUP ##########################
void setup() 
{
  M5.begin();
  Serial.begin(SERIAL_BAUD);
  HoverSerial.begin(HOVER_SERIAL_BAUD);

  while (!SPIFFS.begin()) {
    Serial.println(F("Failed to initialize SPIFFS"));
    /*bool formatted = SPIFFS.format();
    if(formatted){
    Serial.println("\n\nSuccess formatting");
    }else{
        Serial.println("\n\nError formatting");
    }*/
    delay(1000);
  }

  Serial.println(F("Loading configuration..."));
  loadConfiguration(filename, config);

  Serial.println(F("Print config file..."));
  printFile(filename);  

  if(useNunchuk){
    if (wii_i2c_init(WII_I2C_PORT, PIN_SDA, PIN_SCL) != 0) {
      Serial.printf("Error initializing nunchuk");
      return;
    }

    const unsigned char *ident = wii_i2c_read_ident();
    controller_type = wii_i2c_decode_ident(ident);

    if (wii_i2c_start_read_task(READ_TASK_CPU, READ_DELAY) != 0) {
      Serial.printf("Error creating task to read controller state");
      return;
    }
  }

  setDisplay(myDrive, true);

  #include "lvgl_setup.h"
  Serial.println("Setup Done");
}

uint32_t tabview_time = 5000;        // Auto Time
uint32_t lastMillis = 0; 
uint32_t last = 0;
uint16_t eco2, etvoc, errstat, raw;  // Read data
int first = 0;

#include "hoverboard_telemetry.h"

void accelerAte(int16_t start, int16_t target){
  upRamp.go(start);
  upRamp.go(target, accel, LINEAR, ONCEFORWARD);
}

void calibrateCenter(int16_t statex, int16_t statey){
  leftRightCalibration =  statex;
  forwardReverseCalibration = statey;
}

// ########################## LOOP ##########################

unsigned long iTimeSend = 0;

void loop(void)
{ 
  myDrive = upRamp.update();
  unsigned long timeNow = millis();
  
  #include "lvgl_loop.h"

  if(useNunchuk){
    #include "nunchuk.h"
  }

  Receive();

  if (iTimeSend > timeNow) return;
  iTimeSend = timeNow + TIME_SEND;
  if(motorOn == true){
    Send(leftRightValue, myDrive);
    setDisplay(myDrive, true);
    oldmyDrive = myDrive;
  }else{
    myDrive = 0;
    Send(0, 0);
    setDisplay(myDrive, true);
  }
  
}

