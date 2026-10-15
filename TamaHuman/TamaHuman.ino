// TamaHuman - Tamagotchi P1 emulator (TamaLIB) on an Arduino Uno R4 WiFi
// with a 128x64 SSD1306 OLED, where the pet can be redrawn in higher detail.
//
// Wiring
//   OLED (I2C): VCC -> 3.3V or 5V (check your module), GND -> GND,
//               SDA -> SDA (A4), SCL -> SCL (A5)
//   Buttons:    one leg to the pin, other leg to GND (no resistors needed)
//               LEFT (A) -> D2, MIDDLE (B) -> D3, RIGHT (C) -> D4
//   Buzzer:     optional passive buzzer, D5 -> buzzer -> GND
//
// Libraries:   U8g2 (Library Manager)
// Board:       Arduino UNO R4 WiFi (Boards Manager: "Arduino UNO R4 Boards")

#define SCALE   3   // 3 = game area 96x48 with icon bars (sprites up to ~48x48)
                    // 4 = game fills the whole 128x64 screen (sprites up to ~64x64)
#define CAPTURE 1   // 1 = print each new shape over Serial (115200 baud), 0 = off

#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h>

extern "C" {
#include "src/tamalib/tamalib.h"
}
#include "tama_rom.h"
#include "renderer.h"

const int PIN_LEFT = 2, PIN_MIDDLE = 3, PIN_RIGHT = 4, PIN_BUZZER = 5;

U8G2_SSD1306_128X64_NONAME_F_HW_I2C display(U8G2_R0, U8X8_PIN_NONE);

static uint8_t lcd[tama::TH][tama::TW];
static bool icons[8];
static bool dirty = true;
static uint32_t buzzerHz = 0;

// ---------- the HAL: how TamaLIB talks to this board ----------

static void *hal_malloc(u32_t size) { return malloc(size); }
static void hal_free(void *p) { free(p); }
static void hal_halt(void) {}
static bool_t hal_is_log_enabled(log_level_t) { return 0; }
static void hal_log(log_level_t, char *, ...) {}

static timestamp_t hal_get_timestamp(void) { return micros(); }
static void hal_sleep_until(timestamp_t ts) {
  while ((int32_t)(ts - micros()) > 0) {}
}

static void hal_set_lcd_matrix(u8_t x, u8_t y, bool_t val) {
  if (lcd[y][x] != val) { lcd[y][x] = val; dirty = true; }
}
static void hal_set_lcd_icon(u8_t icon, bool_t val) {
  if (icons[icon] != (bool)val) { icons[icon] = val; dirty = true; }
}

static void px(int x, int y) { display.drawPixel(x, y); }
static void logLine(const char *s) { Serial.println(s); }

static void hal_update_screen(void) {
  if (!dirty) return;               // only redraw when the LCD actually changed
  dirty = false;
  display.clearBuffer();
  tama::renderFrame(lcd, icons, px, logLine);
  display.sendBuffer();
}

static void hal_set_frequency(u32_t freq) { buzzerHz = freq / 10; }   // dHz -> Hz
static void hal_play_frequency(bool_t en) {
  if (en && buzzerHz) tone(PIN_BUZZER, buzzerHz); else noTone(PIN_BUZZER);
}

static int hal_handler(void) { return 0; }

static hal_t hal;

// ---------- buttons ----------

static void readButtons() {
  static uint32_t last = 0;
  if (millis() - last < 10) return;
  last = millis();
  tamalib_set_button(BTN_LEFT,   digitalRead(PIN_LEFT)   == LOW ? BTN_STATE_PRESSED : BTN_STATE_RELEASED);
  tamalib_set_button(BTN_MIDDLE, digitalRead(PIN_MIDDLE) == LOW ? BTN_STATE_PRESSED : BTN_STATE_RELEASED);
  tamalib_set_button(BTN_RIGHT,  digitalRead(PIN_RIGHT)  == LOW ? BTN_STATE_PRESSED : BTN_STATE_RELEASED);
}

// ---------- setup / loop ----------

void setup() {
  Serial.begin(115200);
  pinMode(PIN_LEFT, INPUT_PULLUP);
  pinMode(PIN_MIDDLE, INPUT_PULLUP);
  pinMode(PIN_RIGHT, INPUT_PULLUP);

  display.setBusClock(400000);      // fast I2C so redraws don't hold up the game
  display.begin();

  hal.malloc = hal_malloc;
  hal.free = hal_free;
  hal.halt = hal_halt;
  hal.is_log_enabled = hal_is_log_enabled;
  hal.log = hal_log;
  hal.sleep_until = hal_sleep_until;
  hal.get_timestamp = hal_get_timestamp;
  hal.update_screen = hal_update_screen;
  hal.set_lcd_matrix = hal_set_lcd_matrix;
  hal.set_lcd_icon = hal_set_lcd_icon;
  hal.set_frequency = hal_set_frequency;
  hal.play_frequency = hal_play_frequency;
  hal.handler = hal_handler;

  tamalib_register_hal(&hal);
  tamalib_init(tama_rom, NULL, 1000000);   // timestamps are in microseconds

  Serial.println("TamaHuman started");
}

void loop() {
  static uint32_t lastFrame = 0;
  for (int i = 0; i < 64; i++) tamalib_step();
  readButtons();
  if (millis() - lastFrame >= 33) {        // ~30 fps check
    lastFrame = millis();
    hal_update_screen();
  }
}
