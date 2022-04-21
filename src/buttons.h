  if (M5.BtnA.wasReleased()) {
    //motorOn = true;
    myDrive = 0;
    setDisplay(myDrive, true);
  }

  if (M5.BtnA.pressedFor(1000)) {
    setDisplay(myDrive, true);
  }

  if (M5.BtnB.wasReleased()) {
    //motorOn = false;
    myDrive = 0;
    setDisplay(myDrive, true);
  }

  if (M5.BtnB.pressedFor(1000)) {
    setDisplay(myDrive, true);
  }
  
  if (M5.BtnC.wasReleased()) {
    //motorOn = true;
    myDrive = 0;
    setDisplay(myDrive, true);
  }

  if (M5.BtnC.pressedFor(1000)) {
    setDisplay(myDrive, true);
  }
