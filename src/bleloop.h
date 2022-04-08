  // If the flag "doConnect" is true then we have scanned for and found the desired
  // BLE Server with which we wish to connect.  Now we connect to it.  Once we are 
  // connected we set the connected flag to be true.
  if (doConnect == true) 
  {
    if (connectToServer())
    {
      Serial.println("We are now connected to the BLE Server.");
    } 
    else
    {
      Serial.println("We have failed to connect to the server; there is nothin more we will do.");
    }
    doConnect = false;
  }

  if (connected)
  {
    // joystick no activity detector
    if (joyTimer && (joyTimer < millis()))
    {
//      Serial.println("Timeout");
      // no joystick notification for 30mS, center the joystik
      VrBoxData[VB_JOYX] = VrBoxData[VB_JOYY] = 0;

      // wake up the joystick task
      vTaskResume(HandleJS);
      
      joyTimer = 0;
    }
  }
  else if (doScan)
  {
    Serial.println("Start scanning after disconnect");
    // this is just example to start scan after disconnect, most likely there is
    // a better way to do it in Arduino
    //digitalWrite(BLUELED, LEDON);     // indicate scanning
    //digitalWrite(GREENLED, LEDOFF);   // indicate not connected
    BLEDevice::getScan()->start(0);
  }