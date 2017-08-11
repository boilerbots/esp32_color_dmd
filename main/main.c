#include "freertos/FreeRTOS.h"
#include "esp_wifi.h"
#include "esp_wifi.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_event_loop.h"
#include "nvs_flash.h"
#include "driver/gpio.h"
#include "string.h"
#include "driver/rtc_io.h"

esp_err_t event_handler(void *ctx, system_event_t *event)
{
    return ESP_OK;
}

static const uint32_t columns = 64;
static const uint32_t rows = 16;
uint8_t data[16][64];
volatile uint32_t select;  //ABCD

void clock_test()
{
  while(1)
  {
    GPIO.out = 0x10000;
    //GPIO.out_w1ts = (1 << 16);  // CLK high
    //vTaskDelay(50 / portTICK_PERIOD_MS);
    GPIO.out_w1tc = (1 << 16);
    //vTaskDelay(50 / portTICK_PERIOD_MS);
  }
}

void latch_test()
{
  while(1)
  {
    GPIO.out_w1ts = (1 << 17);  // CLK high
    vTaskDelay(50 / portTICK_PERIOD_MS);
    GPIO.out_w1tc = (1 << 17);
    vTaskDelay(50 / portTICK_PERIOD_MS);
  }
}

void row_test()
{
  while(1)
  {
    uint32_t row;
    //for (row = 0; row < rows; ++row)
    {
      //GPIO.out = (row << 12);
      GPIO.out_w1ts = 0xFFFFFFFF;
      vTaskDelay(50 / portTICK_PERIOD_MS);
      GPIO.out_w1tc = 0xFFFFFFFF;
      vTaskDelay(50 / portTICK_PERIOD_MS);
    }
  }
}

void scan()
{
  GPIO.out1_w1tc.data = (1 << 0);  //OE active low

  // Output the data, bit 7 is CLK
  uint32_t row, col;
  for (row = 0; row < rows; ++row)
  {
    select = (row << 12);
    //select = 0;
    for (col = 0; col < columns; ++col)
    {
      //GPIO.out = select | 0x9;
      GPIO.out = select | data[row][col];  // 
      //vTaskDelay(5 / portTICK_PERIOD_MS);
      GPIO.out_w1ts = (1 << 16);  // CLK high
      //vTaskDelay(5 / portTICK_PERIOD_MS);
      //GPIO.out_w1tc = 0x10001;
      //vTaskDelay(50 / portTICK_PERIOD_MS);
    }
    //GPIO.out1_w1ts.data = (1 << 0);  //OE high
    GPIO.out_w1ts = (1 << 17);  // LATCH active low
    //GPIO.out1_w1tc.data = (1 << 0);  //OE active low
    //vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

void fill_rainbow()
{
  uint32_t row, col;
  uint32_t color = 0;
  for (row = 0; row < rows; ++row)
  {
    for (col = 0; col < columns; ++col)
    {
      //data[row][col] = 0x3F;
      data[row][col] = color = (color + 0x81) & 0x3F;
    }
  }
}

/*
* I want to drive a Color DMD panel that is built using two 64x32 RGB LEDs.
* The panels are wired together in series.
* The pixel pitch is P2.5
* These are 1/16 scan type panels.
* Each row is 192 bits, we do 8 rows at a time, 16 times.
*/
void app_main(void)
{
    nvs_flash_init();
#if 0
    tcpip_adapter_init();
    ESP_ERROR_CHECK( esp_event_loop_init(event_handler, NULL) );
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
    ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );
    ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA) );
    wifi_config_t sta_config = {
        .sta = {
            .ssid = "access_point_name",
            .password = "password",
            .bssid_set = false
        }
    };
    ESP_ERROR_CHECK( esp_wifi_set_config(WIFI_IF_STA, &sta_config) );
    ESP_ERROR_CHECK( esp_wifi_start() );
    ESP_ERROR_CHECK( esp_wifi_connect() );
#endif

// 0  R0
// 1  G0
// 2  B0
// 3  R1
// 4  G2
// 5  B3
// 12 A
// 13 B
// 14 C
// 15 D
// 16 CLK
// 17 Latch
// 32 OE

  gpio_config_t ioConfig;
  ioConfig.pin_bit_mask = 0x10003F03F;
  ioConfig.mode = GPIO_MODE_OUTPUT;
  ioConfig.pull_up_en = GPIO_PULLUP_DISABLE;
  ioConfig.pull_down_en = GPIO_PULLDOWN_DISABLE;
  ioConfig.intr_type = GPIO_INTR_DISABLE;
  gpio_config(&ioConfig);


  //clock_test();
  //latch_test();
  //row_test();

  memset(data, 0, columns * rows);
  select = (1 << 17); // set Latch high

  fill_rainbow();

  while (true) {
    scan();
  }
}

