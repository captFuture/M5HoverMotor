#include <Arduino.h>
#include <M5Stack.h>
#include "wii_i2c.h"
#include <Ramp.h>
#include "BLEDevice.h"
#define TaskStackSize   5120
//#include "lvgl.h"
//#include "blejoypad.h"

#define PIN_SDA 21
#define PIN_SCL 22
#define WII_I2C_PORT 0
#define READ_TASK_CPU 0
#define READ_DELAY 30
static unsigned int controller_type;

// ########################## DEFINES ##########################
#define HOVER_SERIAL_BAUD   115200      // [-] Baud rate for HoverSerial (used to communicate with the hoverboard)
#define SERIAL_BAUD         115200      // [-] Baud rate for built-in Serial (used for the Serial Monitor)
#define START_FRAME         0xABCD     	// [-] Start frme definition for reliable serial communication
#define TIME_SEND           100         // [ms] Sending time interval
#define SPEED_MAX           700         // [-] Maximum speed 
#define SPEED_MIN           -700        // [-] Minimum speed 
#define STEER_MAX           400         // [-] Maximum steer
#define STEER_MIN           -400        // [-] Minimum steer
#define ACCEL_MIN           1000
#define BOOST               100         // 

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
boolean boost = false;
boolean triggerReleased = true;

rampInt upRamp;
rampInt downRamp;

int leftRightCalibration = 0;
int forwardReverseCalibration = 0;

int thresholdMovement = 100;

int leftRightValue = 0;
int forwardReverseValue = 0;

int OLDleftRightValue = 0;
int OLDforwardReverseValue = 0;

int accel = ACCEL_MIN; // Acceleration time [ms]
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

// ########################## SETUP ##########################
void setup() 
{
  M5.begin();
  Serial.begin(SERIAL_BAUD);
  Serial.println("Hoverboard Serial v1.0");

  HoverSerial.begin(HOVER_SERIAL_BAUD);

  if(useNunchuk){
    if (wii_i2c_init(WII_I2C_PORT, PIN_SDA, PIN_SCL) != 0) {
      Serial.printf("Error initializing nunchuk :(");
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
}

// ########################## SEND ##########################
void Send(int16_t uSteer, int16_t uSpeed)
{
  // Create command
  Command.start    = (uint16_t)START_FRAME;
  //Command.pitch    = 0;
  //Command.dpitch   = 0;
  Command.steer    = (int16_t)uSteer;
  Command.speed    = (int16_t)uSpeed;
  //Command.sensors  = ;
  Command.checksum = (uint16_t)(Command.start ^ Command.steer ^ Command.speed);

  // Write to Serial
  HoverSerial.write((uint8_t *) &Command, sizeof(Command)); 
}

// ########################## RECEIVE ##########################
void Receive()
{
    if (HoverSerial.available()) {
        incomingByte 	  = HoverSerial.read();                                
        bufStartFrame	= ((uint16_t)(incomingByte) << 8) | incomingBytePrev;      
    }
    else {
        return;
    }

    #ifdef DEBUG_RX
        Serial.print(incomingByte);
        return;
    #endif

    if (bufStartFrame == START_FRAME) {	                    
        p       = (byte *)&NewFeedback;
        *p++    = incomingBytePrev;
        *p++    = incomingByte;
        idx     = 2;	
    } else if (idx >= 2 && idx < sizeof(SerialFeedback)) {  
        *p++    = incomingByte; 
        idx++;
    }	
    
    if (idx == sizeof(SerialFeedback)) {
        uint16_t checksum;
        checksum = (uint16_t)(NewFeedback.start ^ NewFeedback.cmd1 ^ NewFeedback.cmd2 ^ NewFeedback.speedR_meas ^ NewFeedback.speedL_meas
                            ^ NewFeedback.batVoltage ^ NewFeedback.boardTemp ^ NewFeedback.cmdLed);

        if (NewFeedback.start == START_FRAME && checksum == NewFeedback.checksum) {
            memcpy(&Feedback, &NewFeedback, sizeof(SerialFeedback));

            Serial.print("1: ");   Serial.print(Feedback.cmd1);
            Serial.print(" 2: ");  Serial.print(Feedback.cmd2);
            Serial.print(" 3: ");  Serial.print(Feedback.speedR_meas);
            Serial.print(" 4: ");  Serial.print(Feedback.speedL_meas);
            Serial.print(" 5: ");  Serial.print(Feedback.batVoltage);
            Serial.print(" 6: ");  Serial.print(Feedback.boardTemp);
            Serial.print(" 7: ");  Serial.println(Feedback.cmdLed);
        } else {
          Serial.println("Non-valid data skipped");
        }
        idx = 0;
    }

    incomingBytePrev = incomingByte;
}

void accelerAte(int16_t start, int16_t target){
  upRamp.go(start);
  upRamp.go(target, accel, LINEAR, ONCEFORWARD);
}

void brAke(int16_t start, int16_t target){
  upRamp.go(start);
  upRamp.go(target, accel, LINEAR, ONCEBACKWARD);
}

void calibrateCenter(int16_t statex, int16_t statey){
  leftRightCalibration =  statex;
  forwardReverseCalibration = statey;
}

// ########################## LOOP ##########################

unsigned long iTimeSend = 0;

void loop(void)
{ 
  M5.update();
  myDrive = upRamp.update();

  unsigned long timeNow = millis();

  if(useNunchuk){
    const unsigned char *data = wii_i2c_read_data_from_task();

    if (data) {
      OLDleftRightValue = leftRightValue;
      OLDforwardReverseValue = forwardReverseValue;

      wii_i2c_nunchuk_state state;
      wii_i2c_decode_nunchuk(data, &state);

      M5.Lcd.setCursor(10, 0, 2);
      M5.Lcd.setTextSize(1);
      M5.Lcd.print("x: "); M5.Lcd.println(state.x);
      M5.Lcd.print("y: "); M5.Lcd.println(state.y);
      M5.Lcd.print("switch: "); M5.Lcd.println((state.c) ? "pressed" : "not pressed");
      M5.Lcd.print("trigger: "); M5.Lcd.println((state.z) ? "pressed" : "not pressed");

      if(state.c){
        motorOn = true;
        calibrateCenter(state.x, state.y);
      }
      if(state.z){
        motorOn = false;
      }

      leftRightValue = state.x;
      forwardReverseValue = state.y;
      triggerstate = state.z;
      switchState = state.c;

      if( leftRightValue > -20 && leftRightValue < 20 ){
        leftRightValue = 0;
      }else{
        leftRightValue = leftRightValue - leftRightCalibration;
      }

      if(forwardReverseValue > -10 && forwardReverseValue < 10){
        forwardReverseValue = 0;
      }else{
        forwardReverseValue = forwardReverseValue - forwardReverseCalibration;
      }

      leftRightValue = map(leftRightValue, -100, 100, STEER_MIN, STEER_MAX);
      forwardReverseValue = map(forwardReverseValue, -100, 100, SPEED_MIN, SPEED_MAX);

      M5.Lcd.print("x: "); M5.Lcd.println(leftRightValue);
      M5.Lcd.print("y: "); M5.Lcd.println(forwardReverseValue);

      if(OLDforwardReverseValue > forwardReverseValue){       // brake
          //brAke(forwardReverseValue, forwardReverseValue);
          accelerAte(OLDforwardReverseValue, forwardReverseValue);
        }else if(OLDforwardReverseValue < forwardReverseValue){ // accelerate
          //if(forwardReverseValue > OLDforwardReverseValue +100){;
          accelerAte(OLDforwardReverseValue, forwardReverseValue);
          //}
        }

    }
  }

  Receive();
 
  if (M5.BtnA.wasReleased()) {
    if(triggerReleased){
      motorOn = true;
      myDrive = 0;
      upRamp.go(0);
      upRamp.go(SPEED_MIN, accel, LINEAR, ONCEFORWARD);
    }else{
      triggerReleased = true;
    }
    setDisplay(myDrive, true);
  }

  if (M5.BtnA.pressedFor(1000)) {
    boost = !boost;
    triggerReleased = false;
    setDisplay(myDrive, true);
  }

  if (M5.BtnB.wasReleased()) {
    if(triggerReleased){
      motorOn = !motorOn;
      accel = accel -100;
      if(accel < 500){accel = 500;}
      myDrive = 0;
      upRamp.go(0);
    }else{
      triggerReleased = true;
    }
    setDisplay(myDrive, true);
  }

  if (M5.BtnB.pressedFor(1000)) {
    accel = ACCEL_MIN;
    triggerReleased = false;
    setDisplay(myDrive, true);
  }
  
  if (M5.BtnC.wasReleased()) {
    if(triggerReleased){
      motorOn = true;
      myDrive = 0;
      upRamp.go(0);
      if(boost){
        upRamp.go(SPEED_MAX + BOOST, accel, LINEAR, ONCEFORWARD); 
      }else{
        upRamp.go(SPEED_MAX, accel, LINEAR, ONCEFORWARD); 
      }
    }else{
      triggerReleased = true;
    }
    setDisplay(myDrive, true);
  }

  if (M5.BtnC.pressedFor(1000)) {
    accel = 500;
    triggerReleased = false;
    setDisplay(myDrive, true);
  }


  if (iTimeSend > timeNow) return;
  iTimeSend = timeNow + TIME_SEND;
  if(motorOn == true){
    Send(leftRightValue, myDrive);
    setDisplay(myDrive, true);
    oldmyDrive = myDrive;
  }else{
    Send(0, 0);
    setDisplay(myDrive, true);
  }
  
}

void setDisplay(int myDrive, boolean clear){
    if(clear){M5.Lcd.fillScreen(TFT_BLACK);}
    M5.Lcd.setCursor(10, 100, 2);
    M5.Lcd.setTextSize(1);
    M5.Lcd.print("speed: "); M5.Lcd.println(myDrive);
    M5.Lcd.print("MAX: "); M5.Lcd.println(SPEED_MAX);
    M5.Lcd.print("MIN: "); M5.Lcd.println(SPEED_MIN);
    M5.Lcd.print("accel: "); M5.Lcd.println(accel);
    //M5.Lcd.print("boost: "); M5.Lcd.println(boost);
}