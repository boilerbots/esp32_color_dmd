#include "freertos/FreeRTOS.h"
#include "esp_wifi.h"
#include "esp_wifi.h"
#include "esp_system.h"
#include "esp_attr.h"
#include "esp_event.h"
#include "esp_event_loop.h"
#include "nvs_flash.h"
#include "driver/gpio.h"
#include "string.h"

//extern volatile int port_xSchedulerRunning[2];
void scan(void *args) IRAM_ATTR __attribute__((noreturn));


esp_err_t event_handler(void *ctx, system_event_t *event)
{
    return ESP_OK;
}

static const uint32_t columns = 64;
static const uint32_t rows = 16;
volatile uint8_t data[16][64];

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
    //uint32_t row;
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

void scan(void *args)
{
  GPIO.out1_w1tc.data = (1 << 0);  //OE active low

  // Output the data, bit 7 is CLK
  uint32_t row, col;
  volatile uint32_t select;  //ABCD
  while(1)
  {
    for (row = 0; row < rows; ++row)
    {
      select = (row << 12);
      for (col = 0; col < columns; ++col)
      {
        GPIO.out = select | data[row][col];  // 
        //GPIO.out = select | 0x3F;  // 
        GPIO.out_w1ts = (1 << 16);  // CLK high
      }
      GPIO.out_w1ts = (1 << 17);  // LATCH active high
    }
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

void fill_solid(uint32_t color)
{
  uint32_t row, col;
  for (row = 0; row < rows; ++row)
  {
    for (col = 0; col < columns; ++col)
    {
      data[row][col] = color;
    }
  }
}

void video_pattern()
{
  while(1)
  {
    fill_rainbow(0);
    vTaskDelay(2000 / portTICK_PERIOD_MS);
    fill_solid(0x09); //RED
    vTaskDelay(2000 / portTICK_PERIOD_MS);
    fill_solid(0x12); //BLUE
    vTaskDelay(2000 / portTICK_PERIOD_MS);
    fill_solid(0x24); //GREEN
    vTaskDelay(2000 / portTICK_PERIOD_MS);
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

  //memset(data, 0, columns * rows);

  xTaskCreatePinnedToCore(&scan, "scan", configMINIMAL_STACK_SIZE, NULL, (2 | portPRIVILEGE_BIT), NULL, 1);

  while (true) {
    video_pattern();
  }
}

