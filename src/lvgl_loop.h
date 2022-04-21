lv_task_handler(); //delay(5);

etvoc = 000;

if (first == 0){
    lv_obj_set_hidden(page,true);
    lv_win_clean(page);
    lv_label_set_text_fmt(label," ");
    first = 1;
} 
lv_label_set_text_fmt(txt2,   "%d", driveSpeed );
lv_label_set_text_fmt(txt5,   "%d", etvoc);
lv_gauge_set_value   (gauge,     0, driveSpeed );
//lv_gauge_set_value   (gauge2,     0, batVoltage );
lv_chart_set_next    (chart1, ser1, driveSpeed ); 
lv_chart_set_next    (chart2, ser2, driveSpeed); 

if ( !auto_man ) {
  if ( (millis()-last)>tabview_time){
    last = millis();
    tab_pos = tab_pos + 1; if(tab_pos>(tb-1)){tab_pos=0;}
    lv_tabview_set_tab_act(tabview, tab_pos, false);
  }
}