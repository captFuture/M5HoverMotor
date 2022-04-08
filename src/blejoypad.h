// ######################### BLEJOYSTICK ######################
enum
{
  VB_TRIGGERS = 0,
  VB_JOYX,
  VB_JOYY,
  VB_BTNAB,
  VB_BTNCD,
  VB_NUMBYTES
};

// ===== VR Box Button Masks =====
#define VB_LOW_TRIGGER    0x01
#define VB_UPR_TRIGGER    0x02
#define VB_BUTTON_A       0x10
#define VB_BUTTON_B       0x20
#define VB_BUTTON_C       0x01
#define VB_BUTTON_D       0x02
#define FRESHFLAG         0x80

#define JOYTIMEOUT        30      // joystick no activity timeout in mS

#define JoyStickDeadZone  0

#define ServerName  "VR PARK"      // change this if your server uses a different name

//----- ESP32 Definitions ------------------------------------------------------

// ===== VRBOX Modes =====
// This code assumes you are using the Mouse Mode
// @ + A -> Music & Video mode
// @ + B -> Horizontal Game mode
// @ + C -> Vertical Game mode
// @ + D -> Mouse Mode  // use this mode
//  4 byte notification, Trigger Sws, Joystick X, Joystick Y, 0x00

// All four modes send data. However each mode uses different byte positions and
// values for each of the switches. The joystick acts like a 'D' pad when not in
// Mouse Mode (no analog value).
 
typedef void (*NotifyCallback)(BLERemoteCharacteristic*, uint8_t*, size_t, bool);

// this is the service UUID of the VR Control handheld mouse/joystick device (HID)
static BLEUUID serviceUUID("00001812-0000-1000-8000-00805f9b34fb");

// Battery Service UUID
static BLEUUID BatteryServiceUUID("0000180F-0000-1000-8000-00805f9b34fb");

// this characteristic UUID works for joystick & triggers (report)
static BLEUUID ReportCharUUID("00002A4D-0000-1000-8000-00805f9b34fb"); // report


static boolean doConnect = false;
static boolean connected = false;
static boolean doScan = false;
static BLERemoteCharacteristic* pRemoteCharacteristic;
static BLEAdvertisedDevice* myDevice;
static BLERemoteCharacteristic* pBatRemoteCharacteristic;

// pointer to a list of characteristics of the active service,
// sorted by characteristic UUID
std::map<std::string, BLERemoteCharacteristic*> *pmap;
std::map<std::string, BLERemoteCharacteristic*> :: iterator itr;

// pointer to a list of characteristics of the active service,
// sorted by characteristic handle
std::map<std::uint16_t, BLERemoteCharacteristic*> *pmapbh;
std::map<std::uint16_t, BLERemoteCharacteristic*> :: iterator itrbh;

// storage for pointers to characteristics we want to work with
// to do: change to linked list ?
BLERemoteCharacteristic *bleRcs[4];

// This is where we store the data from the buttons and joystick
volatile byte   VrBoxData[VB_NUMBYTES];
volatile bool   flag = false;         // indicates new data to process

// joyTimer is a 30 millisecond re-triggerable timer that sets the joystick 
// back to center if no activity on the joystick or trigger buttons. 
volatile uint32_t joyTimer = millis();

// task handles  
TaskHandle_t HandleJS = NULL;   // handle of the joystick task
TaskHandle_t HandleAB = NULL;   // handle of the A/B button task
TaskHandle_t HandleCD = NULL;   // handle of the C/D button task

char bfr[80];

//******************************************************************************
// HID notification callback handler.
//******************************************************************************
static void notifyCallback(
  BLERemoteCharacteristic* pBLERemoteCharacteristic,
  uint8_t* pData,
  size_t length,
  bool isNotify)
{
  Serial.print("Notify callback for characteristic ");

  Serial.print(pBLERemoteCharacteristic->getUUID().toString().c_str());
  Serial.print(" of data length ");
  Serial.println(length);
  Serial.print("data: ");

  for (int i = 0; i < length; i++)
    Serial.printf("%02X ", pData[i]);
  Serial.println();
 
  // we are getting the two trigger buttons in the first byte, joyX & joyY in 2nd & 3rd bytes
  // A four byte report is the joystick/trigger buttons.
  // A two byte report is either the A/B buttons or the C/D buttons
  // Low nibble equal to 0x05 indicates A/B buttons.
  // A/B buttons auto-repeat if held. No other buttons do this.
  if (4 == length)
  {
    // copy data to VrBoxData
    for (int i = VB_TRIGGERS; i < VB_BTNAB; i++)
      VrBoxData[i] = pData[i];

    // wake up the joystick/trigger buttons handler task
    if (HandleJS)
      vTaskResume(HandleJS);
      
    // restart the joystick timer
    joyTimer = millis() + JOYTIMEOUT;
  }
  else if (2 == length)
  {
    // show the received data
    if (0x05 == (pData[0] & 0x0F))
    {
      // A/B button report, wake the A/B button handler task
      VrBoxData[VB_BTNAB] = pData[0] & 0xF0;
      if (HandleAB)
        vTaskResume(HandleAB);
    }
    else
    {
      // C/D button report, wake the C/D button handler task
      VrBoxData[VB_BTNCD] = pData[0];
      if (HandleCD)
        vTaskResume(HandleCD);
    }
  }
} //  notifyCallback

//******************************************************************************
// Battery notification callback handler.
//******************************************************************************
static void BatteryNotifyCallback(
  BLERemoteCharacteristic* pBLERemoteCharacteristic,
  uint8_t* pData,
  size_t length,
  bool isNotify)
{
  Serial.println("Battery Notification Callback Event");
  Serial.print("Data length ");
  Serial.println(length);
  Serial.print("data: ");

  for (int i = 0; i < length; i++)
    Serial.printf("%02X ", pData[i]);
  Serial.println();
} //  BatteryNotifyCallback

//******************************************************************************
// Connection state change event callback handler.
//******************************************************************************
class MyClientCallback : public BLEClientCallbacks
{
  void onConnect(BLEClient* pclient)
  {
    Serial.println("onConnect event");
    //digitalWrite(GREENLED, LEDON);     // indicate connected
  }

  void onDisconnect(BLEClient* pclient)
  {
    Serial.println("onDisconnect event");
    connected = false;
    //digitalWrite(GREENLED, LEDOFF);     // indicate disconnected
  }
};

//******************************************************************************
// Connect to a service, register for notifications from Report Characteristics.
//******************************************************************************
bool setupCharacteristics(BLERemoteService* pRemoteService, NotifyCallback pNotifyCallback)
{
  // get all the characteristics of the service using the handle as the key
  pmapbh = pRemoteService->getCharacteristicsByHandle();
  
  // only interested in report characteristics that have the notify capability
  for (itrbh = pmapbh->begin(); itrbh != pmapbh->end(); itrbh++)
  {
    BLEUUID x = itrbh->second->getUUID();
    Serial.print("Characteristic UUID: ");
    Serial.println(x.toString().c_str());
    // the uuid must match the report uuid

    if (ReportCharUUID.equals(itrbh->second->getUUID()))
    {
      // found a report characteristic
      Serial.println("Found a report characteristic");

      if (itrbh->second->canNotify())
      {
        Serial.println("Can notify");
        // register for notifications from this characteristic
        itrbh->second->registerForNotify(pNotifyCallback);

        sprintf(bfr, "Callback registered for: Handle: 0x%08X, %d", itrbh->first, itrbh->first);
        Serial.println(bfr);
      }
      else
      {
        Serial.println("No notification");
      }
    }
    else
    {
        sprintf(bfr, "Found Characteristic UUID: %s\n", itrbh->second->getUUID().toString().c_str()); 
        Serial.println(bfr);
    }
  } //  for
} // setupCharacteristics
 
//******************************************************************************
// Validate the server has the correct name and services we are looking for.
// The server must have the HID service, the Battery Service is optional.
//******************************************************************************
bool connectToServer()
{
  Serial.print("Forming a connection to ");
  Serial.println(myDevice->getAddress().toString().c_str());
  
  BLEClient*  pClient  = BLEDevice::createClient();
  Serial.println(" - Created client");

  pClient->setClientCallbacks(new MyClientCallback());

  // Connect to the remote BLE Server.
  pClient->connect(myDevice);  // if you pass BLEAdvertisedDevice instead of address, it will be recognized type of peer device address (public or private)
  Serial.println(" - Connected to server");

  // BLE servers may offer several services, each with unique characteristics
  // we can identify the type of service by using the service UUID
  
  // Obtain a reference to the service we are after in the remote BLE server.
  // this will return a pointer to the remote service if it has a matching service UUID
  BLERemoteService* pRemoteService = pClient->getService(serviceUUID);
  if (pRemoteService == nullptr)
  {
    Serial.print("Failed to find HID service UUID: ");
    Serial.println(serviceUUID.toString().c_str());
    pClient->disconnect();
    return false;
  }

  Serial.println(" - Found HID service");
  setupCharacteristics(pRemoteService, notifyCallback);
  pRemoteService = pClient->getService(BatteryServiceUUID);
  if (pRemoteService == nullptr)
  {
    Serial.print("Failed to find battery service UUID: ");
    Serial.println(serviceUUID.toString().c_str());
  }
  else
  {
    Serial.println(" - Found battery service");
    setupCharacteristics(pRemoteService, BatteryNotifyCallback);
  }

  connected = true;
} //  connectToServer

//******************************************************************************
// Scan for BLE servers and find the first one that advertises the service we are looking for.
//******************************************************************************
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks
{
  // Called for each advertising BLE server.
  void onResult(BLEAdvertisedDevice advertisedDevice)
  {
    Serial.print("BLE Advertised Device found: ");
    Serial.println(advertisedDevice.toString().c_str());

    // we have found a server, see if it has the name we are looking for
    if (advertisedDevice.haveName())
    {
      if (0 == strcmp(ServerName, advertisedDevice.getName().c_str()))
      {
        Serial.println("Found VRBOX Server");

        // we found a server with the correct name, see if it has the service we are
        // interested in (HID)
  
        if (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(serviceUUID))
        {
          Serial.println("Server has HID service");
          
          BLEDevice::getScan()->stop();
          //digitalWrite(BLUELED, LEDOFF);     // indicate not scanning
    
          myDevice = new BLEAdvertisedDevice(advertisedDevice);
          doConnect = true;
          doScan = true;
        } // Found our server
        else
        {
          Serial.println("Server does not have an HID service, not our server");
        }
      }
    }
    else
    {
      Serial.println("Server name does not match, not our server");
    }
  } // onResult
}; // MyAdvertisedDeviceCallbacks


// All of these tasks are designed to run forever. The tasks are resumed when
// a notification message is received with new data.
//******************************************************************************
// Joystick handler Task.
// Moving the joystick off center causes this task to be resumed about every
// 15ms. Press or release of either trigger button will also resume this task.
// If this task does not complete in less than 15mS you will lose joystick
// movement data !!!
// Holding the lower button will prevent the server from detecting that the 
// upper button has been pressed. 
// Holding the upper trigger and pressing the lower trigger results in the server
// sending a notification that the lower trigger is pressed (the upper trigger
// will be zero!). Releasing the lower trigger will cause the server to send a
// notification that the upper trigger is pressed and the lower trigger is
// released.
//******************************************************************************
void taskJoyStick(void *parameter)
{
  int8_t  x;
  int8_t  y;
  uint8_t triggers;
  
  //===== if the task requires any one time initialization, put it here =====

  // forever loop
  while(true)
  {
    // give up the CPU, wait for new data
    vTaskSuspend(NULL);

    // we just woke up, new data is available, convert joystick data to
    // signed 8 bit integers
    x = (int8_t)VrBoxData[VB_JOYX];
    y = (int8_t)VrBoxData[VB_JOYY];
    triggers = VrBoxData[VB_TRIGGERS];
    
    Serial.printf("Joystick X: %d, Y: %d Triggers: %02X\n", x, y, triggers);

    if (y < -JoyStickDeadZone)
    {
      // move forward
      Serial.println("Forward");
      
      //===== add your code here =====
      
    }
    else if (y > JoyStickDeadZone)
    {
      // move backward
      Serial.println("Backward");

      //===== add your code here =====
      
    }
        
    if (x > JoyStickDeadZone)
    {
      // turn right
      Serial.println("Turn Right");

      //===== add your code here =====
      
    }
    else if (x < -JoyStickDeadZone)
    {
      // turn left
      Serial.println("Turn Left");

      //===== add your code here =====
      
    }

    if (triggers & VB_LOW_TRIGGER)
    {
      // the lower trigger button is pressed
      Serial.println("Low Trigger Pressed");

      //===== add your code here =====
    }

    if (triggers & VB_UPR_TRIGGER)
    {
      // the upper trigger button is pressed
      Serial.println("Upper Trigger Pressed");

      //===== add your code here =====
      
    }
  } //  for
} //  taskJoyStick

//******************************************************************************
// A & B Buttons handler Task.
// Holding the A or B button down will cause this task to be invoked about every
// 15ms. If this task does not complete within 15mS you will lose button events.
// The AB buttons work similar to the trigger buttons in that the A button will
// prevent the B button from being detected and will override the B button when
// pressed while the B button is held down.
//******************************************************************************
void taskButtonAB(void *parameter)
{
  uint8_t buttons;

  //===== if the task requires any one time initialization, put it here =====
  
  while(true)
  {
    // give up the CPU, wait for new data
    vTaskSuspend(NULL);
    
    // we just woke up, new data is available
    buttons = VrBoxData[VB_BTNAB];
    Serial.printf("A/B Buttons: %02X\n", buttons);
       
    if (buttons & VB_BUTTON_A)
    {
      // button A pressed or is being held down
      Serial.println("Button A");
      
      //===== add your code here =====
      
    }

    if (buttons & VB_BUTTON_B)
    {
      // button B pressed or is being held down
      Serial.println("Button B");

      //===== add your code here =====
      
    }
  } //  for
} //  taskButtonAB

//******************************************************************************
// C & D Buttons handler Task. 
// Press or release of either the C or D button will resume this task. Holding
// one button down blocks the Server from detecting the other button being 
// pressed.
//******************************************************************************
void taskButtonCD(void *parameter)
{
  uint8_t buttons;
  
  //===== if the task requires any one time initialization, put it here =====
  
  while(true)
  {
    // give up the CPU
    vTaskSuspend(NULL);

    // we just woke up, new data is available
    buttons = VrBoxData[VB_BTNCD];
    Serial.printf("C/D Buttons: %02X\n", buttons);
    
    if (buttons & VB_BUTTON_C)
    {
      // button C pressed
      Serial.println("Button C");
      
      //===== add your code here =====
      
    }

    if (buttons & VB_BUTTON_D)
    {
      // button D pressed
      Serial.println("Button D");

      //===== add your code here =====
      
    }
  } //  for
} //  taskButtonCD

