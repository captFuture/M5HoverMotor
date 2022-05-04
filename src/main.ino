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
  int decel_min;
  int boost_max;
};

const char *filename = "/config.json";
Config config;

const char* ssid = config.ssid;
const char* password = config.passwd;

#define TaskStackSize   5120
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
unsigned long timeNow = 0;

/* Telemetry */
int16_t driveSpeed = 0;
//int16_t sentSpeed = 0;
int16_t speedR = 0;
int16_t speedL = 0;
int16_t batVoltage = 0;
int16_t boardTemp = 0;

boolean triggerReleased = true;
int16_t configNum = 0;

int16_t leftRightCalibration = 0;
int16_t forwardReverseCalibration = 0;
int16_t thresholdMovement = 100;
int16_t leftRightValue = 0;
int16_t forwardReverseValue = 0;
int16_t forwardReverseInput = 0;
int16_t OLDleftRightValue = 0;
int16_t OLDforwardReverseValue = 0;
int16_t accel = config.accel_min; // Acceleration time [ms]
int16_t decel = config.accel_min; // Acceleration time [ms]
int16_t safetyCool = 10;


int16_t myDrive = 0;
int16_t oldmyDrive = 0;

typedef struct{
   int16_t start;
   int16_t  steer;
   int16_t  speed;
   int16_t checksum;
} SerialCommand;
SerialCommand Command;

typedef struct{
   int16_t start;
   int16_t  cmd1;
   int16_t  cmd2;
   int16_t  speedR_meas;
   int16_t  speedL_meas;
   int16_t  batVoltage;
   int16_t  boardTemp;
   int16_t cmdLed;
   int16_t checksum;
} SerialFeedback;
SerialFeedback Feedback;
SerialFeedback NewFeedback;

#include "lvgl_start.h"
#include "configload.h"

// ########################## SETUP ##########################
void setup() 
{
  M5.begin();
  Serial.begin(SERIAL_BAUD);
  HoverSerial.begin(HOVER_SERIAL_BAUD);

  while (!SPIFFS.begin()) {
    Serial.println(F("Failed to initialize SPIFFS"));
    bool formatted = SPIFFS.format();
    if(formatted){
    Serial.println("\n\nSuccess formatting");
    }else{
        Serial.println("\n\nError formatting");
    }
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

  #include "lvgl_setup.h"
  Serial.println("Setup Done");
}

uint32_t tabview_time = 5000;        // Auto Time
uint32_t lastMillis = 0; 
uint32_t last = 0;
int first = 0;

#include "hoverboard_telemetry.h"

    void forWard(int16_t start, int16_t target){
        myDrive = start;
        while(myDrive < target){
          myDrive = myDrive + 1; 
        }
        //myDrive = target;
      oldmyDrive = myDrive;
    }

    void backWard(int16_t start, int16_t target){
        myDrive = start;
        while(myDrive > target){
          myDrive = myDrive - 1; 
        }
        //myDrive = target;
      oldmyDrive = myDrive;
    }

    void calibrateCenter(int16_t statex, int16_t statey){
      leftRightCalibration =  statex;
      forwardReverseCalibration = statey;
    }

// ########################## LOOP ##########################

unsigned long iTimeSend = 0;

void loop(void)
{ 
  #include "lvgl_loop.h"
  if(useNunchuk){
    #include "nunchuk.h"
  }

  ReceiveTelemetry();

  if (millis() > TIME_SEND + iTimeSend ){
    iTimeSend = millis();

    if(motorOn == true){
      SendCommand(leftRightValue, myDrive);
    }else{
      myDrive = 0;
      SendCommand(0, 0);
    }


    lv_chart_set_next(chart, ser1, forwardReverseInput);
    lv_chart_set_next(chart, ser2, forwardReverseValue);
    lv_chart_set_next(chart, ser3, myDrive);
  }

}

