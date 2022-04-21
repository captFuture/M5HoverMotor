    const unsigned char *data = wii_i2c_read_data_from_task();

    if(data) {
      OLDleftRightValue = leftRightValue;
      OLDforwardReverseValue = forwardReverseValue;

      wii_i2c_nunchuk_state state;
      wii_i2c_decode_nunchuk(data, &state);

      M5.Lcd.setCursor(0, 0, 2);
      M5.Lcd.setTextSize(1);
      M5.Lcd.print("x: "); M5.Lcd.println(state.x);
      M5.Lcd.print("y: "); M5.Lcd.println(state.y);
      M5.Lcd.print("switches: "); M5.Lcd.print((state.c) ? "C pressed" : "C not pressed "); M5.Lcd.println((state.z) ? "Z pressed" : "Z not pressed");
      M5.Lcd.print("Motor: "); M5.Lcd.println((motorOn) ? "true" : "false");
      M5.Lcd.print("Config: "); M5.Lcd.println(configNum);
      M5.Lcd.print("rpm: "); M5.Lcd.println(driveSpeed);

      float speedFactor = 2.0 * 3.14 * 0.08255 * 60 /1000;
      M5.Lcd.print("fac: "); M5.Lcd.println(speedFactor);
      M5.Lcd.print("km/h: "); M5.Lcd.println(driveSpeed * speedFactor);

      if(state.c){
        motorOn = !motorOn;
        calibrateCenter(state.x, state.y);
        delay(200);
      }
      if(state.z){
        configNum = configNum+1;
        if(configNum > 4){
          configNum = 0;
        }
        delay(200);
      }

      leftRightValue = state.x;
      forwardReverseValue = state.y;
      triggerstate = state.z;
      switchState = state.c;

      if( leftRightValue > -15 && leftRightValue < 15 ){
        leftRightValue = 0;
      //}else if(leftRightValue < config.steer_max && leftRightValue > config.steer_min){
        //leftRightValue = 0;
      }else{
        leftRightValue = leftRightValue - leftRightCalibration;
      }

      if(forwardReverseValue > -15 && forwardReverseValue < 15){
        forwardReverseValue = 0;
      }else if(forwardReverseValue < config.speed_max+(config.boost_max*configNum) && forwardReverseValue > config.speed_min+(config.boost_max*configNum)){
        forwardReverseValue = 0;
      }else{
        forwardReverseValue = forwardReverseValue - forwardReverseCalibration;
      }

      leftRightValue = map(leftRightValue, -100, 100, config.steer_min-(config.boost_max*configNum), config.steer_max+(config.boost_max*configNum));
      forwardReverseValue = map(forwardReverseValue, -100, 100, config.speed_min+(config.boost_max*configNum), config.speed_max-(config.boost_max*configNum));

      M5.Lcd.print("x: "); M5.Lcd.println(leftRightValue);
      M5.Lcd.print("y: "); M5.Lcd.println(forwardReverseValue);
      

      if(OLDforwardReverseValue > forwardReverseValue){       // brake
          accelerAte(OLDforwardReverseValue, forwardReverseValue);
        }else if(OLDforwardReverseValue < forwardReverseValue){ 
          accelerAte(OLDforwardReverseValue, forwardReverseValue);
        }

    }