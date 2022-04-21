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

            //driveSpeed = 2*3,14*8,255*Feedback.speedR_meas*60/1000;
            //driveSpeed = 3,11 * Feedback.speedL_meas;
            driveSpeed = (float)Feedback.speedL_meas;
        } else {
          Serial.println("Non-valid data skipped");
        }
        idx = 0;
    }

    incomingBytePrev = incomingByte;
}