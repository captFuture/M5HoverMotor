  M5.Lcd.begin();
  M5.Lcd.setRotation(1);
  M5.Lcd.fillScreen(TFT_BLUE);
  //WiFi.Begin(); WiFi.setSleep(false);  // A Button error prevention
  lv_init();
  Initialize_the_display(); // Initialize the display, Display flushin
  //-------------------------------------------------------------------
  
  // Initialize the Touch pad =========================================
  lv_indev_drv_t indev_drv;
  lv_indev_drv_init(&indev_drv);         //input device driver
  indev_drv.type = LV_INDEV_TYPE_POINTER;//Touch pad is a pointer-like
  indev_drv.read_cb = my_touchpad_read;  //Set your driver function
  lv_indev_drv_register(&indev_drv);     //Finally register the driver
  //-------------------------------------------------------------------
 
  // Tabview : https://docs.lvgl.io/latest/en/html/widgets/tabview.html
  // Create a Tabview object ==========================================
  tabview = lv_tabview_create(lv_scr_act(), NULL);
  lv_obj_set_style_local_pad_top(tabview,LV_TABVIEW_PART_TAB_BG, LV_STATE_DEFAULT,1);
  lv_obj_set_style_local_pad_bottom(tabview,LV_TABVIEW_PART_TAB_BG, LV_STATE_DEFAULT,1);
  lv_obj_set_style_local_pad_top(tabview,LV_TABVIEW_PART_TAB_BTN,LV_STATE_DEFAULT,1);
  lv_obj_set_style_local_pad_bottom(tabview,LV_TABVIEW_PART_TAB_BTN,LV_STATE_DEFAULT,1); 
  // Add 4 tabs (the tabs are page (lv_page) and can be scrolled ------
  lv_obj_t *tab1 = lv_tabview_add_tab(tabview, "TELE");
  lv_obj_t *tab2 = lv_tabview_add_tab(tabview, "SPEED");
  lv_obj_t *tab3 = lv_tabview_add_tab(tabview, "INPUT");
  lv_obj_t *tab4 = lv_tabview_add_tab(tabview, "BATTERY");
  lv_obj_t *tab5 = lv_tabview_add_tab(tabview, "Chart");
  lv_tabview_set_tab_act(tabview, tab_pos, false); // display position
  static lv_style_t st; lv_style_init(&st);
  lv_style_set_bg_color(&st,LV_STATE_DEFAULT,lv_color_hex(0x049CD8));
    lv_obj_add_style(tabview, LV_OBJ_PART_MAIN, &st); // Backgroud color
  // ------------------------------------------------------------------
  
  // Add content to the tabs ------------------------------------------
  // Label : https://docs.lvgl.io/latest/en/html/widgets/label.html
  // change : lv_arduino/lv_conf.h/ #define LV_FONT_MONTSERRAT_28 1
  lv_style_set_text_font(&st,LV_STATE_DEFAULT,&lv_font_montserrat_28); 
  lv_page_set_scrollbar_mode(tab1, LV_SCROLLBAR_MODE_OFF); //scroll off

  label = lv_label_create(   tab1, NULL);
  lv_obj_add_style(    label, LV_LABEL_PART_MAIN, &st);        
  lv_label_set_recolor(label, true); // Enable re-coloring by commands
  lv_label_set_align(  label, LV_LABEL_ALIGN_CENTER); // Center aligned
  lv_obj_set_width(    label, screenWidth);
  // ------------------------------------------------------------------
  
  // Create a page ----------------------------------------------------
  page = lv_page_create(lv_scr_act(), NULL);
  lv_obj_set_size(page, 310, 160);
  lv_obj_align(page, NULL, LV_ALIGN_CENTER, 0, 0);
  lv_obj_t * labelP = lv_label_create(page, NULL);
  lv_obj_set_width(         labelP, lv_page_get_width_fit(page)); 
  lv_obj_add_style(         labelP, LV_LABEL_PART_MAIN, &st);
  lv_label_set_recolor(     labelP, true);
  lv_obj_set_auto_realign(  labelP, true);
  lv_label_set_text(labelP,"TarantlCart\nSTARTING#\nPlease wait");
  //-------------------------------------------------------------------

  int p1 = -60; // tab1 
  int p2 =  -35; // tab1 

  
  // tab 1 txt 1 -------------------------------------------------------------
  static lv_style_t st2; lv_style_init(&st2);
  lv_style_set_text_font(&st2,LV_STATE_DEFAULT,&lv_font_montserrat_28); 
  txt1 = lv_label_create( tab1, NULL);
  lv_obj_add_style(       txt1, LV_LABEL_PART_MAIN, &st2);
  lv_obj_set_auto_realign(txt1, true);
  lv_obj_align(           txt1, NULL, LV_ALIGN_CENTER, 0, p1);
  lv_label_set_align(     txt1, LV_LABEL_ALIGN_CENTER);
  lv_label_set_text(      txt1, "wait");
  //-------------------------------------------------------------------

  // tab 1 txt 2 -------------------------------------------------------------
  txt2 = lv_label_create( tab1, NULL);
  lv_obj_add_style(       txt2, LV_LABEL_PART_MAIN, &st2);
  lv_obj_set_auto_realign(txt2, true);
  lv_obj_align(           txt2, NULL, LV_ALIGN_CENTER, 0, p2);
  lv_label_set_align(     txt2, LV_LABEL_ALIGN_CENTER);
  lv_label_set_text(      txt2, "wait");
  //-------------------------------------------------------------------


  // driveSpeed GAUGE tab2 ==================================================
  // Gauge : https://docs.lvgl.io/latest/en/html/widgets/gauge.html
  // Create a gauge on tab2 ===========================================
  static lv_style_t st3; lv_style_init(&st3);
  lv_style_set_text_font(&st3,LV_STATE_DEFAULT,&lv_font_montserrat_16); 
  static lv_color_t needle_colors[1];
 
  needle_colors[0] = lv_color_hex(0x049CD8);
  needle_colors[1] = lv_color_hex(0xE52521);

  lv_page_set_scrollbar_mode(tab2, LV_SCROLLBAR_MODE_OFF); //scroll off
  gauge = lv_gauge_create(   tab2, NULL); // Display gauge on tab2

  lv_gauge_set_needle_count(gauge, 2, needle_colors);
  lv_obj_set_size(          gauge, 240, 240);
  lv_gauge_set_range(       gauge, 0, 25); // y range  
  lv_obj_align(             gauge, NULL, LV_ALIGN_CENTER, 0, -5);
  lv_gauge_set_value(       gauge, 0, 0);      // value
  lv_gauge_set_value(       gauge, 1, 0);      // value
  lv_obj_set_style_local_bg_color(gauge, LV_CHART_PART_BG, LV_STATE_DEFAULT, lv_color_hex(0xFBD000)); 

  lv_obj_t * label2 = lv_label_create(tab2, NULL);
  lv_obj_add_style(        label2, LV_LABEL_PART_MAIN, &st3);  // 16p
  lv_obj_set_auto_realign( label2, true);
  lv_obj_align(            label2, NULL, LV_ALIGN_CENTER, 0, 0);
  lv_label_set_text(       label2, "Speed\n\n\n\n\nkm/h");
  lv_gauge_set_critical_value(gauge, 15);

  //------------------------------------------------------------------- 
  
   // tab 2 txt 3 -------------------------------------------------------------
  txt3 = lv_label_create( tab2, NULL);
  lv_obj_add_style(       txt3, LV_LABEL_PART_MAIN, &st2);
  lv_obj_set_auto_realign(txt3, true);
  lv_obj_align(           txt3, NULL, LV_ALIGN_IN_BOTTOM_LEFT, 10, -40);
  lv_label_set_align(     txt3, LV_LABEL_ALIGN_CENTER);
  lv_label_set_text(      txt3, "0");
  //-------------------------------------------------------------------

  // tab3 ==================================================
  // Create tab3 ========================================== 
  static lv_style_t st4; lv_style_init(&st4);
  lv_style_set_text_font(&st4,LV_STATE_DEFAULT,&lv_font_montserrat_16); 
  lv_page_set_scrollbar_mode(tab3, LV_SCROLLBAR_MODE_OFF); //scroll off

   // GAUGE tab3 ==================================================
  // Gauge : https://docs.lvgl.io/latest/en/html/widgets/gauge.html
  // Create a gauge on tab3 ===========================================

  needle_colors[0] = lv_color_hex(0x049CD8); // blau
  needle_colors[1] = lv_color_hex(0xE52521); // rot
  needle_colors[2] = lv_color_hex(0x43B047); // gruen

  lv_page_set_scrollbar_mode(tab3, LV_SCROLLBAR_MODE_OFF); //scroll off
  gauge3 = lv_gauge_create(tab3, NULL); // Display gauge on tab3

  lv_gauge_set_needle_count(gauge3, 3, needle_colors);
  lv_obj_set_size(          gauge3, 240, 240);
  lv_gauge_set_range(       gauge3, -800, 800); // y range  
  lv_obj_align(             gauge3, NULL, LV_ALIGN_CENTER, 0, -5);
  lv_gauge_set_value(       gauge3, 0, 0);      // value
  lv_gauge_set_value(       gauge3, 1, 0);      // value
  lv_obj_set_style_local_bg_color(gauge3, LV_CHART_PART_BG, LV_STATE_DEFAULT, lv_color_hex(0xFBD000)); //gelb
  lv_gauge_set_critical_value(gauge3, 0);

  //-------------------------------------------------------------------
  
  // Batterx GAUGE tab4 ==================================================
  // Gauge : https://docs.lvgl.io/latest/en/html/widgets/gauge.html
  // Create a gauge on tab4 ===========================================
  static lv_style_t st5; lv_style_init(&st5);
  lv_style_set_text_font(&st5,LV_STATE_DEFAULT,&lv_font_montserrat_16); 
  lv_page_set_scrollbar_mode(tab4, LV_SCROLLBAR_MODE_OFF); //scroll off
  static lv_color_t needle_colors2[1];
  needle_colors2[0] = lv_color_hex(0xFBD000);
  gauge2 = lv_gauge_create(   tab4, NULL); // Display gauge on tab4
  lv_gauge_set_needle_count(gauge2, 1, needle_colors2);
  lv_obj_set_size(          gauge2, 240, 240);
  lv_gauge_set_range(       gauge2, 3000, 4200); // y range  
  lv_obj_align(             gauge2, NULL, LV_ALIGN_CENTER, 0, -5);
  lv_gauge_set_value(       gauge2, 0, 0);      // value
  lv_obj_set_style_local_bg_color(gauge2, LV_CHART_PART_BG, LV_STATE_DEFAULT, lv_color_hex(0x43B047)); 

  lv_obj_t * label4 = lv_label_create(tab4, NULL);
  lv_obj_add_style(        label4, LV_LABEL_PART_MAIN, &st5);  // 16p
  lv_obj_set_auto_realign( label4, true);
  lv_obj_align(            label4, NULL, LV_ALIGN_CENTER, 0, 0);
  lv_label_set_text(       label4, "Battery\n\n\n\n\nmAh");
    lv_gauge_set_critical_value(gauge2, 3600);
  //------------------------------------------------------------------- 

  // speed CHART tab5 ==================================================
  // Chart : https://docs.lvgl.io/latest/en/html/widgets/chart.html
  // Create a chart on tab5 ===========================================

  chart = lv_chart_create(tab5, NULL);
  lv_obj_set_size(   chart, screenWidth -20, screenHeight -20); // size
  lv_obj_align(      chart, NULL, LV_ALIGN_CENTER, 0, 0);
  lv_chart_set_type( chart, LV_CHART_TYPE_LINE);
  lv_chart_set_range(chart, -800, 800);  // y axis

  ser1 = lv_chart_add_series(chart,LV_COLOR_BLUE);
  ser2 = lv_chart_add_series(chart, LV_COLOR_RED);
  ser3 = lv_chart_add_series(chart, LV_COLOR_GREEN);

  led1  = lv_led_create(tab5, NULL);
  lv_obj_align(led1, NULL, LV_ALIGN_CENTER, -50,-30);
  lv_led_off(led1);

  led2  = lv_led_create(tab5, NULL);
  lv_obj_align(led2, NULL, LV_ALIGN_CENTER, -100,-30);
  lv_led_off(led2);