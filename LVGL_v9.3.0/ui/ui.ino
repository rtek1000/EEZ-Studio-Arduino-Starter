// Hardware (board ESP32 + ILI9341): https://github.com/rtek1000/2.4inch_ESP32-2432S024 

#include <lvgl.h>
#include <TFT_eSPI.h>
#include <ui.h>
#include "CST820.h"

/* Define touch screen pins */
#define I2C_SDA 33
#define I2C_SCL 32
#define TP_RST 25
#define TP_INT 21

// TODO: Replace with your screen resolution
/*Set to your screen resolution and rotation*/
#define screenSizeBig 320    // Bigger
#define screenSizeSmall 240  // Smaller
#define TFT_ROTATION LV_DISPLAY_ROTATION_270

#if TFT_ROTATION == LV_DISPLAY_ROTATION_0 || TFT_ROTATION == LV_DISPLAY_ROTATION_180  // Portrait
#define TFT_HOR_RES screenSizeSmall
#define TFT_VER_RES screenSizeBig
#elif TFT_ROTATION == LV_DISPLAY_ROTATION_90 || TFT_ROTATION == LV_DISPLAY_ROTATION_270  // Landscape
#define TFT_HOR_RES screenSizeBig
#define TFT_VER_RES screenSizeSmall
#endif

/*LVGL draw into this buffer, 1/10 screen size usually works well. The size is in bytes*/
#define DRAW_BUF_SIZE (TFT_HOR_RES * TFT_VER_RES / 10 * (LV_COLOR_DEPTH / 8))
uint32_t draw_buf[DRAW_BUF_SIZE / 4];

// TODO: Replace with your screen's touch controller
TFT_eSPI tft = TFT_eSPI();                      /* TFT example */
CST820 touch(I2C_SDA, I2C_SCL, TP_RST, TP_INT); /* Touch instance */

#if LV_USE_LOG != 0
void my_print(lv_log_level_t level, const char* buf) {
  LV_UNUSED(level);
  Serial.println(buf);
  Serial.flush();
}
#endif

void my_disp_flush(lv_display_t* disp, const lv_area_t* area, uint8_t* px_map) {
  uint32_t w = (area->x2 - area->x1 + 1);
  uint32_t h = (area->y2 - area->y1 + 1);

  //tft.startWrite();
  tft.pushImageDMA(area->x1, area->y1, w, h, (uint16_t*)px_map);
  //tft.endWrite();

  lv_disp_flush_ready(disp);
}

void my_touchpad_read(lv_indev_t* indev, lv_indev_data_t* data) {
  // TODO: Replace with your custom touch-screen handling logic!

  bool touched = false;
  uint8_t gesture;
  uint16_t touchX = 0, touchY = 0;
  int16_t x = 0, y = 0;

  // Serial.println("touchscreen_read");

  touched = touch.getTouch(&touchX, &touchY, &gesture);

  if (!touched) {
    data->state = LV_INDEV_STATE_RELEASED;
  } else {
    data->state = LV_INDEV_STATE_PRESSED;

    if ((TFT_ROTATION == LV_DISPLAY_ROTATION_0) || (TFT_ROTATION == LV_DISPLAY_ROTATION_180)) {  // Portrait
      x = touchX;
      y = touchY;
    } else {  // if ((TFT_ROTATION == LV_DISPLAY_ROTATION_90) || (TFT_ROTATION == LV_DISPLAY_ROTATION_270)) {  // Landscape
      x = TFT_HOR_RES - touchX;
      y = TFT_VER_RES - touchY;
    }

    if (x < 0) x = 0;
    if (y < 0) y = 0;

    /*Set the coordinates*/
    data->point.x = x;
    data->point.y = y;

    // Serial.print("Touched: ");
    // Serial.print(x);
    // Serial.print(", ");
    // Serial.println(y);
  }
}

/*use Arduinos millis() as tick source*/
static uint32_t my_tick(void) {
  return millis();
}

void setup() {
  /* Display backlight config */
  pinMode(27, OUTPUT);

  /* Display backlight disable */
  digitalWrite(27, LOW);

  /* TFT-sSPI Initialization */
  tft.begin();

  /* Rotation is done by LVGL now */
  // tft.setRotation(screenRotate);

  /* Initialize DMA */
  tft.initDMA();

  /* Display backlight enable */
  digitalWrite(27, HIGH);

  /* Screen test */
  tft.fillScreen(TFT_RED);
  delay(500);
  tft.fillScreen(TFT_GREEN);
  delay(500);
  tft.fillScreen(TFT_BLUE);
  delay(500);
  tft.fillScreen(TFT_BLACK);
  delay(500);

  /* LVGL init */
  lv_init();

  /*Set a tick source so that LVGL will know how much time elapsed. */
  lv_tick_set_cb(my_tick);

  /* register print function for debugging */
#if LV_USE_LOG != 0
  lv_log_register_print_cb(my_print);
#endif

  lv_display_t* disp;
#if LV_USE_TFT_ESPI
  /*TFT_eSPI can be enabled lv_conf.h to initialize the display in a simple way*/
  disp = lv_tft_espi_create(TFT_HOR_RES, TFT_VER_RES, draw_buf, sizeof(draw_buf));
  lv_display_set_rotation(disp, TFT_ROTATION);

#else
  /*Else create a display yourself*/
  disp = lv_display_create(TFT_HOR_RES, TFT_VER_RES);
  lv_display_set_flush_cb(disp, my_disp_flush);
  lv_display_set_buffers(disp, draw_buf, NULL, sizeof(draw_buf), LV_DISPLAY_RENDER_MODE_PARTIAL);
#endif

  /*Initialize the (dummy) input device driver*/
  lv_indev_t* indev = lv_indev_create();
  lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER); /*Touchpad should have POINTER type*/
  lv_indev_set_read_cb(indev, my_touchpad_read);

  /* Initialize the touchpad */
  touch.begin();

  // Init EEZ-Studio UI
  ui_init();
}

void loop() {
  lv_timer_handler();
  // Update EEZ-Studio UI
  ui_tick();
}
