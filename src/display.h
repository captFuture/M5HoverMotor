void setDisplay(int myDrive, boolean clear){
    if(clear){M5.Lcd.fillScreen(TFT_BLACK);}
    M5.Lcd.setCursor(0, 140, 2);
    M5.Lcd.setTextSize(1);
    M5.Lcd.print("speed: "); M5.Lcd.println(myDrive);
    M5.Lcd.print("MAX: "); M5.Lcd.println(config.speed_max);
    M5.Lcd.print("MIN: "); M5.Lcd.println(config.speed_min);
    M5.Lcd.print("ST-MAX: "); M5.Lcd.println(config.steer_max);
    M5.Lcd.print("ST-MIN: "); M5.Lcd.println(config.steer_min);
    M5.Lcd.print("accel: "); M5.Lcd.println(accel);
    
    //M5.Lcd.print("boost: "); M5.Lcd.println(boost);
}