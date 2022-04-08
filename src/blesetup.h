  BaseType_t xReturned;

  // create tasks to handle the joystick and buttons
  xReturned = xTaskCreate(taskJoyStick,             // task to handle activity on the joystick.
                          "Joystick",               // String with name of task.
                          TaskStackSize,            // Stack size in 32 bit words.
                          NULL,                     // Parameter passed as input of the task
                          1,                        // Priority of the task.
                          &HandleJS);               // Task handle.
  if (pdPASS == xReturned)
  {
    Serial.println("Joystick Task Created");
  }
 
  xReturned = xTaskCreate(taskButtonAB,             // task to handle activity on the A & B buttons.
                          "ButtonsAB",              // String with name of task.
                          TaskStackSize,            // Stack size in 32 bit words.
                          NULL,                     // Parameter passed as input of the task
                          1,                        // Priority of the task.
                          &HandleAB);               // Task handle.
  if (pdPASS == xReturned)
  {
    Serial.println("AB Button Task Created");
    
  }
 
  xReturned = xTaskCreate(taskButtonCD,             // task to handle activity on the C & D buttons.
                          "ButtonsCD",              // String with name of task.
                          TaskStackSize,            // Stack size in 32 bit words.
                          NULL,                     // Parameter passed as input of the task
                          1,                        // Priority of the task.
                          &HandleCD);               // Task handle.
  if (pdPASS == xReturned)
  {
    Serial.println("CD Button Task Created");
    
  }
  Serial.println("Starting ESP32 BLE Client...");
  BLEDevice::init("");

  // Retrieve a GATT Scanner and set the callback we want to use to be informed 
  // when we have detected a new device.  Specify that we want active scanning
  // and start the scan to run for 5 seconds.
  //digitalWrite(BLUELED, LEDON);     // indicate scanning
  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setInterval(1349);
  pBLEScan->setWindow(449);
  pBLEScan->setActiveScan(true);
  pBLEScan->start(5, false);      // scan for 5 seconds

  if (!connected)
  {
    doScan = true;
    Serial.println("Offline, start a scan");
  }