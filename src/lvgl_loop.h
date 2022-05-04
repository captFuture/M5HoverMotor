lv_task_handler(); //delay(5);

if (first == 0){
    lv_obj_set_hidden(page,true);
    lv_win_clean(page);
    lv_label_set_text_fmt(label," ");
    first = 1;
} 
//lv_label_set_text_fmt(txt1,   "speed: %d", driveSpeed );
lv_label_set_text_fmt(txt1,   "SetSpeed: %d", forwardReverseInput );
lv_label_set_text_fmt(txt2,   "configNum: %d", configNum);
lv_label_set_text_fmt(txt3,   "%d", configNum);

//lv_label_set_text_fmt(txt1,   "input: %d", forwardReverseInput );
//lv_label_set_text_fmt(label2, "Speed %d\n\n\n\n\nkm/h", configNum);

lv_gauge_set_value   (gauge,     0, driveSpeed );
//lv_gauge_set_value   (gauge,     1, sentSpeed );

lv_gauge_set_value   (gauge2,     0, batVoltage );
lv_gauge_set_value   (gauge3,     0, forwardReverseValue);
lv_gauge_set_value   (gauge3,     1, forwardReverseInput);
lv_gauge_set_value   (gauge3,     2, myDrive);


if(motorOn){
   //lv_style_set_bg_color(&st,LV_STATE_DEFAULT,lv_color_hex(0xE52521));
}else{
  // lv_style_set_bg_color(&st,LV_STATE_DEFAULT,lv_color_hex(0x049CD8));
}

if ( !auto_man ) {
  if ( (millis()-last)>tabview_time){
    last = millis();
    tab_pos = tab_pos + 1; if(tab_pos>(tb-1)){tab_pos=0;}
    lv_tabview_set_tab_act(tabview, tab_pos, false);
  }
}