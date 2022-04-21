
#include <lvgl.h>
int screenWidth  = 320;
int screenHeight = 240;
#define LVGL_TICK_PERIOD 30
#include <Ticker.h>
Ticker tick;

static lv_disp_buf_t disp_buf;
static lv_color_t buf[LV_HOR_RES_MAX * 10];
static lv_obj_t * page;
static lv_obj_t * txt2;
static lv_obj_t * txt5;
static lv_obj_t * txt6;
static lv_obj_t * label;
static lv_obj_t * gauge;
static lv_obj_t * gauge2;
static lv_obj_t * tabview;
uint32_t period = 10; //
int tb = 4;           // Number of tabs
int tab_pos = 0;
int time_count = 0;

bool auto_man = true;

//=====================================================================
// Interrupt driven periodic handler ----------------------------------
static void lv_tick_handler(void){lv_tick_inc(LVGL_TICK_PERIOD);}
//=====================================================================
int mx = screenWidth/(tb*4), my = 10, md = screenWidth/tb;
// Read M5Stack A B,C buttons as touchpad : Tabview select ------------
static bool my_touchpad_read(lv_indev_drv_t * indev_driver,
                             lv_indev_data_t * data){
  // A Button : Leftward ----------------------------------------------
  if (M5.BtnA.wasPressed()){ 
    mx = mx - md; if (mx < 0){ mx = screenWidth - (md / 4);}
    data->point.x = mx; data->point.y = my;
    data->state =LV_INDEV_STATE_PR; M5.update(); return true;
  }
  // C Button : Rightward ---------------------------------------------
  if (M5.BtnC.wasPressed()){ 
    mx = mx + md; if (mx > screenWidth){ mx = md / 4;}
    data->point.x = mx; data->point.y = my;
    data->state =LV_INDEV_STATE_PR; M5.update(); return true;
  }
  // B Button : Auto / Manual Display ---------------------------------
  if (M5.BtnB.wasPressed()){
    configNum = configNum+1;
      if(configNum > 6){
        configNum = 0;
      }
  }

  if (M5.BtnB.pressedFor(2000)){
    auto_man = !auto_man;          M5.update(); return false;
  }  
  // No button operation ----------------------------------------------
  data->state =LV_INDEV_STATE_REL; M5.update(); return false;
}
//=====================================================================


// Display flushing ===================================================
void my_disp_flush(
  lv_disp_drv_t *disp,const lv_area_t *area,lv_color_t *color_p){
  uint32_t w = (area->x2 - area->x1 + 1);
  uint32_t h = (area->y2 - area->y1 + 1);
  M5.Lcd.startWrite();
  M5.Lcd.setAddrWindow(area->x1, area->y1, w, h);
  M5.Lcd.pushColors(&color_p->full, w*h, true);
  M5.Lcd.endWrite();
  lv_disp_flush_ready(disp);
}
//=====================================================================

// Initialize the display =============================================
void Initialize_the_display(){
  lv_disp_buf_init(&disp_buf, buf, NULL, LV_HOR_RES_MAX * 10);
  lv_disp_drv_t disp_drv;
  lv_disp_drv_init(&disp_drv);
  disp_drv.hor_res  = screenWidth;
  disp_drv.ver_res  = screenHeight;
  disp_drv.flush_cb = my_disp_flush;
  disp_drv.buffer   = &disp_buf;
  lv_disp_drv_register(&disp_drv);
}
//=====================================================================