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
        // speed 300 + 100 * 0,1,2,3,4 = 300, 400, 500, 600, 700
        // steer 200 + 100 * 0,1,2,3,4 = 200, 300, 400, 500, 600
        // mapped Value is sent to Controller and board speeds up
       
        delay(200);
      }

      leftRightInput = state.x;
      leftRightValue = state.x;
      forwardReverseInput = state.y;
      forwardReverseValue = state.y;
      triggerstate = state.z;
      switchState = state.c;

      if( leftRightInput > - 15 && leftRightInput < 15 ){
        leftRightInput = 0;
        leftRightValue = 0;
      }else{
        
      }

      if(forwardReverseInput > -15 && forwardReverseInput < 15){
        forwardReverseInput = 0;
        forwardReverseValue = 0;
      }else if(forwardReverseInput < config.speed_min-(config.boost_max*configNum)){ 
        forwardReverseValue = config.speed_min-(config.boost_max*configNum);
      }else if(forwardReverseInput > config.speed_max+(config.boost_max*configNum)){
        forwardReverseValue = config.speed_max+(config.boost_max*configNum);
      }else{
        
      }

      //leftRightValue = leftRightInput;
      leftRightValue = map(leftRightInput, -100, 100, config.steer_min, config.steer_max);

      lv_gauge_set_value   (gauge3,     2, leftRightValue);
      forwardReverseValue = map(forwardReverseInput, -100, 100, config.speed_min-(config.boost_max*configNum), config.speed_max+(config.boost_max*configNum));

      //myDrive = forwardReverseValue;
      lv_gauge_set_value   (gauge3,     0, forwardReverseValue);
      lv_gauge_set_value   (gauge3,     1, forwardReverseInput);


      if(forwardReverseInput > -5 && forwardReverseInput < 5){
        lv_led_off(led1);
        lv_led_off(led2);
        myDrive = 0;
      }else{
        lv_led_on(led1);
        lv_led_on(led2);

        if(forwardReverseInput > 5){ // forward
          lv_led_off(led2);
          lv_led_on(led1);
          if(rampDrive){
            if(OLDforwardReverseValue < forwardReverseValue){          
              forWard(OLDforwardReverseValue, forwardReverseValue, 1); // accel
            }else{
              forWard(OLDforwardReverseValue, forwardReverseValue, 0); // decel
            }
          }else{
            myDrive = forwardReverseValue;
          }

        }else if(forwardReverseInput < -5){ // backward
          lv_led_on(led2);
          lv_led_off(led1);
          if(rampDrive){
            if(OLDforwardReverseValue > forwardReverseValue){          
              backWard(OLDforwardReverseValue, forwardReverseValue, 1); // accel
            }else{
              backWard(OLDforwardReverseValue, forwardReverseValue, 0); // decel
            }
          }else{
            myDrive = forwardReverseValue;
          }

        }
      }
     
      OLDforwardReverseValue = forwardReverseValue;
    }