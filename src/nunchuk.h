    const unsigned char *data = wii_i2c_read_data_from_task();

    if(data) {
      OLDleftRightValue = leftRightValue;
      wii_i2c_nunchuk_state state;
      wii_i2c_decode_nunchuk(data, &state);
      
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

        // USART command 1000 is 1000 RPM
        // 300 + 100 * 0,1,2,3,4 = 300, 400, 500, 600, 700
        // mapped Value is sent to Controller and board speeds up
       
        delay(200);
      }

      leftRightValue = state.x;
      forwardReverseInput = state.y;
      forwardReverseValue = state.y;
      triggerstate = state.z;
      switchState = state.c;

      if( leftRightValue > -15 && leftRightValue < 15 ){
        leftRightValue = 0;
      }else{
        leftRightValue = leftRightValue - leftRightCalibration;
      }

      if(forwardReverseInput > -15 && forwardReverseInput < 15){
        forwardReverseValue = 0;
      }else if(forwardReverseInput < config.speed_max+(config.boost_max*configNum) && forwardReverseInput > config.speed_min+(config.boost_max*configNum)){
        forwardReverseValue = 0;
      }else{
        forwardReverseValue = forwardReverseValue - forwardReverseCalibration;
      }

      /*if(forwardReverseInput < -50){
        forwardReverseInput = -50; // limit backward
      }*/

      leftRightValue = map(leftRightValue, -100, 100, config.steer_min, config.steer_max);
      forwardReverseValue = map(forwardReverseInput, -100, 100, config.speed_min+(config.boost_max*configNum), config.speed_max-(config.boost_max*configNum));


      if(OLDforwardReverseValue > forwardReverseValue){       // brake
        //decelerAte(OLDforwardReverseValue, forwardReverseValue);
        myDrive = forwardReverseValue;
      }else if(OLDforwardReverseValue < forwardReverseValue){ 
        //accelerAte(OLDforwardReverseValue, forwardReverseValue);
        myDrive = forwardReverseValue;
      }

    //float SentSpeedFactor = 2.0 * 3.14 * 0.08255 * 60 /1000;
    //float tempSentSpeed = (float)forwardReverseValue * SentSpeedFactor;
    //sentSpeed = abs((int)tempSentSpeed);

    }