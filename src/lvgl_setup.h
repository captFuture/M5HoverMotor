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
  lv_obj_set_style_local_pad_top(        // Minimize tab height
                   tabview,LV_TABVIEW_PART_TAB_BG, LV_STATE_DEFAULT,1);
  lv_obj_set_style_local_pad_bottom(
                   tabview,LV_TABVIEW_PART_TAB_BG, LV_STATE_DEFAULT,1);
  lv_obj_set_style_local_pad_top(
                   tabview,LV_TABVIEW_PART_TAB_BTN,LV_STATE_DEFAULT,1);
  lv_obj_set_style_local_pad_bottom(
                   tabview,LV_TABVIEW_PART_TAB_BTN,LV_STATE_DEFAULT,1); 
  // Add 4 tabs (the tabs are page (lv_page) and can be scrolled ------
  lv_obj_t *tab1 = lv_tabview_add_tab(tabview, "TELE"); // LABEL
  lv_obj_t *tab2 = lv_tabview_add_tab(tabview, "SPEED");     // GAUGE
  lv_obj_t *tab3 = lv_tabview_add_tab(tabview, "GAME");     // CHART
  lv_obj_t *tab4 = lv_tabview_add_tab(tabview, "EXTRA");     // CHART
  lv_tabview_set_tab_act(tabview, tab_pos, false); // display position
  static lv_style_t st; lv_style_init(&st);
  lv_style_set_bg_color(&st,LV_STATE_DEFAULT,lv_color_hex(0x6185F8));
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
  int p2 =  40; // tab1 

  
  // txt2 -------------------------------------------------------------
  static lv_style_t st2; lv_style_init(&st2);
  lv_style_set_text_font(&st2,LV_STATE_DEFAULT,&lv_font_montserrat_28); 
  txt2 = lv_label_create( tab1, NULL);
  lv_obj_add_style(       txt2, LV_LABEL_PART_MAIN, &st2);
  lv_obj_set_auto_realign(txt2, true);
  lv_obj_align(           txt2, NULL, LV_ALIGN_CENTER, 0, p1);
  lv_label_set_align(     txt2, LV_LABEL_ALIGN_CENTER);
  lv_label_set_text(      txt2, "wait");
  //-------------------------------------------------------------------

  // txt5 -------------------------------------------------------------
  txt5 = lv_label_create( tab1, NULL);
  lv_obj_add_style(       txt5, LV_LABEL_PART_MAIN, &st2);
  lv_obj_set_auto_realign(txt5, true);
  lv_obj_align(           txt5, NULL, LV_ALIGN_CENTER, 0, p2);
  lv_label_set_align(     txt5, LV_LABEL_ALIGN_CENTER);
  lv_label_set_text(      txt5, "wait");
  //-------------------------------------------------------------------
    
  // driveSpeed GAUGE tab2 ==================================================
  // Gauge : https://docs.lvgl.io/latest/en/html/widgets/gauge.html
  // Create a gauge on tab2 ===========================================
  static lv_style_t st3; lv_style_init(&st3);
  lv_style_set_text_font(&st3,LV_STATE_DEFAULT,&lv_font_montserrat_16); 
  static lv_color_t needle_colors[1];
  needle_colors[0] = lv_color_hex(0x049CD8);
  lv_page_set_scrollbar_mode(tab2, LV_SCROLLBAR_MODE_OFF); //scroll off
  gauge = lv_gauge_create(   tab2, NULL); // Display gauge on tab2
  lv_gauge_set_needle_count(gauge, 1, needle_colors);
  lv_obj_set_size(          gauge, 240, 240);
  lv_gauge_set_range(       gauge, 0, 15); // y range  
  lv_obj_align(             gauge, NULL, LV_ALIGN_CENTER, 0, -5);
  lv_gauge_set_value(       gauge, 0, 0);      // value
  lv_obj_set_style_local_bg_color(gauge, LV_CHART_PART_BG, LV_STATE_DEFAULT, lv_color_hex(0xFBD000)); 
  lv_obj_t * label2 = lv_label_create(tab2, NULL);
  lv_obj_add_style(        label2, LV_LABEL_PART_MAIN, &st3);  // 16p
  lv_obj_set_auto_realign( label2, true);
  lv_obj_align(            label2, NULL, LV_ALIGN_CENTER, 0, 0);
  lv_label_set_text(       label2, "Speed\n\n\n\n\nkm/h");
  lv_gauge_set_critical_value(gauge, 10);
  //------------------------------------------------------------------- 
  
  // driveSpeed CHART tab3 ==================================================
  // Chart : https://docs.lvgl.io/latest/en/html/widgets/chart.html
  // Create a chart1 on tab3 ========================================== 
  static lv_style_t st4; lv_style_init(&st4);
  lv_style_set_text_font(&st4,LV_STATE_DEFAULT,&lv_font_montserrat_16); 
  lv_page_set_scrollbar_mode(tab3, LV_SCROLLBAR_MODE_OFF); //scroll off
  chart1 = lv_chart_create(  tab3, NULL); // Display Chart on tab3
  lv_obj_set_size(   chart1, screenWidth, screenHeight-20); // size
  lv_obj_align(      chart1, NULL, LV_ALIGN_CENTER, -7, -7);
  lv_chart_set_type( chart1, LV_CHART_TYPE_LINE);//Show lines and point
  lv_chart_set_range(chart1, 0, 20);  // y range
//lv_obj_set_style_local_bg_opa(chart1,   // Transparent
//                  LV_CHART_PART_BG, LV_STATE_DEFAULT, LV_OPA_TRANSP);
  lv_obj_set_style_local_bg_color(chart1, LV_CHART_PART_BG, 
                             LV_STATE_DEFAULT, lv_color_hex(0xD2FFD8));                    
  lv_obj_set_style_local_pad_left(chart1, LV_CHART_PART_BG, LV_STATE_DEFAULT,4 * (LV_DPI / 7.5));
  lv_chart_set_y_tick_texts(chart1,"5\n10\n15\n20",3, LV_CHART_AXIS_DRAW_LAST_TICK | LV_CHART_AXIS_INVERSE_LABELS_ORDER);  
  ser1 = lv_chart_add_series(chart1,LV_COLOR_RED);//Add two data series
  lv_chart_set_next( chart1, ser1, 10); // Set the next points 
  lv_obj_t * label3 = lv_label_create(tab3, NULL);
  lv_obj_add_style( label3, LV_LABEL_PART_MAIN, &st4);        // 16p
  lv_obj_align(     label3, NULL, LV_ALIGN_CENTER, -143, -85);
  lv_label_set_text(label3, "km/h");
  //-------------------------------------------------------------------
  
  // TVOC CHART tab4 ==================================================
  // Chart : https://docs.lvgl.io/latest/en/html/widgets/chart.html
  // Create a chart2 on tab4 ==========================================
  static lv_style_t st5; lv_style_init(&st5);
  lv_style_set_text_font(&st5,LV_STATE_DEFAULT,&lv_font_montserrat_16); 
  lv_page_set_scrollbar_mode(tab4, LV_SCROLLBAR_MODE_OFF); //scroll off
  chart2 = lv_chart_create(  tab4, NULL); // Display Chart on tab4
  lv_obj_set_size(   chart2, screenWidth, screenHeight-20); // size
  lv_obj_align(      chart2, NULL, LV_ALIGN_CENTER, -7, -7);
  lv_chart_set_type( chart2, LV_CHART_TYPE_LINE);//Show lines and point
  lv_chart_set_range(chart2, 0, 200);     // y range
//lv_obj_set_style_local_bg_opa(chart2,   // Transparent
//                  LV_CHART_PART_BG, LV_STATE_DEFAULT, LV_OPA_TRANSP);
  lv_obj_set_style_local_bg_color(chart2, LV_CHART_PART_BG, // color
                             LV_STATE_DEFAULT, lv_color_hex(0xEAE7FF));                     
  lv_obj_set_style_local_pad_left(chart2,
    LV_CHART_PART_BG, LV_STATE_DEFAULT,4 * (LV_DPI / 7.5));
  lv_chart_set_y_tick_texts(chart2,"0\n50\n100\n150\n200",4, 
    LV_CHART_AXIS_DRAW_LAST_TICK | LV_CHART_AXIS_INVERSE_LABELS_ORDER);  
  ser2 = lv_chart_add_series(chart2,LV_COLOR_RED);//Add two data series
  lv_chart_set_next( chart2, ser2, 10); // Set the next points
  lv_obj_t * label4 = lv_label_create(tab4, NULL);
  lv_obj_add_style( label4, LV_LABEL_PART_MAIN, &st5);         // 16p
  lv_obj_align(     label4, NULL, LV_ALIGN_CENTER, -143, -85);
  lv_label_set_text(label4, "ppb\n\n\nTVOC");
  //-------------------------------------------------------------------