/* Simple HTTP Server Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <iostream>

#include <driver/spi_master.h>
#include <esp_eth.h>
#include <esp_event.h>
#include <esp_http_server.h>
#include <esp_log.h>
#include <esp_netif.h>
#include <esp_system.h>
#include <esp_wifi.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <nvs_flash.h>
#include <sys/param.h>
#include "protocol_examples_common.h"

#include "io.h"
#include "routes.h"
#include "webserver.h"
#include "spi.h"

#define STATIC      0
#define SHIFTING    1
#define SWEEP       2
#define RAINBOW     3


using namespace std;

extern "C" {
	void app_main();
}

const char *TAG = "Traffic Light";


void go_driver(void *pvParameters);


/*
void go_driver(void *pvParameters)
{
	ESP_LOGI(TAG, "Setting up SPI for GO sign.");
	spi_device_handle_t spi_handle;
	spi_transaction_t trans_desc;
	char data[4*(GO_LED_NUMBER+2)];

	traffic_spi_init(VSPI_HOST, SPI_DMA_CH2, &spi_handle, &trans_desc, data,
			GO_LED_DATA_PIN, GO_LED_CLK_PIN, GO_LED_NUMBER);
	ESP_LOGI(TAG, "Done configuring SPI for GO sign.");

	// Start frame
	data[0] = 0x00;
	data[1] = 0x00;
	data[2] = 0x00;
	data[3] = 0x00;

	// End frame
	data[(GO_LED_NUMBER+1)*4 + 0] = 0xff;
	data[(GO_LED_NUMBER+1)*4 + 1] = 0xff;
	data[(GO_LED_NUMBER+1)*4 + 2] = 0xff;
	data[(GO_LED_NUMBER+1)*4 + 3] = 0xff;

	int state = RAINBOW;

	uint16_t color_shift = 0xf333;
	uint16_t colors;
	uint32_t color_time;
	uint32_t colors_hsv;

	while(1) {

		switch (state) {
			case STATIC:

				for (int led = 1; led <= GO_LED_NUMBER; led++){
					led_color(data, led, 0x00, 0x7f, 0x00);	// rgb
				}
				break;
			
			case SHIFTING:

				color_time = 0xff & esp_log_timestamp();
				for (int led = 1; led <= GO_LED_NUMBER; led++){
					colors = color_time - go_sign_y[led];
					led_color(data, led, (char) (colors) & 0x7f,
						(char) (colors) & 0x7f, (char) (255 - colors) & 0x7f);	// rgb
				}
				break;

			case SWEEP:

				if (color_shift > 0x00f0) {
					color_shift >>= 1;
				} else {
					color_shift = 0xf333;
				}
				for (int led = 1; led <= GO_LED_NUMBER; led++){
					colors = color_shift << (go_sign_y[led-1]/30);
					led_color(data, led, (char) (colors >> 8) & 0x0f,
						(char) (colors >> 4) & 0x0f, (char) colors & 0x0f);	// rgb
				}
				break;

			case RAINBOW:

				color_time = 0xff & esp_log_timestamp();
				for (int led = 1; led <= GO_LED_NUMBER; led++){
					uint8_t x = color_time - go_sign_y[led-1];
					colors_hsv = Color::hsv_to_rgb((uint32_t)x * 359 / 256, 255, 255);
					led_color(data, led, (char) (colors_hsv >> 16) & 0x7f,
						(char) (colors_hsv >> 8) & 0x7f, (char) colors_hsv & 0x7f); // rgb
				}
				break;
		}

		ESP_ERROR_CHECK(spi_device_queue_trans(spi_handle, &trans_desc, portMAX_DELAY));
		vTaskDelay(157 / portTICK_RATE_MS);
	}
}
*/

void app_main(void)
{
	// TODO: Move out in to init_wifi() ?
	/* Init WiFi */
	ESP_ERROR_CHECK(nvs_flash_init());
	ESP_ERROR_CHECK(esp_netif_init());
	ESP_ERROR_CHECK(esp_event_loop_create_default());

	/**
	 * Register two events and bind the callbacks to the WiFi object.
	 * We want to know when we have an IP, and when we disconnet from the WiFi.
	 */
	ESP_ERROR_CHECK(
		esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, WebServer::connectHandler, nullptr, nullptr)
	);

	ESP_ERROR_CHECK(
		esp_event_handler_instance_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, WebServer::disconnectHandler,
			nullptr, nullptr)
	);

	/* This helper function configures Wi-Fi or Ethernet, as selected in menuconfig.
	 * Read "Establishing Wi-Fi or Ethernet Connection" section in
	 * examples/protocols/README.md for more information about this function.
	 */
	ESP_ERROR_CHECK(example_connect());

	// add handlers for the URLs
	g_webServer.addHandler(&g_root)
		   .addHandler(&g_echo)
		   .addHandler(&g_about);


	// initialize SPI for stop and go sign
	traffic_spi_init(HSPI_HOST, SPI_DMA_CH1, &spi_handle_stop, &trans_desc_stop, stop_data,
			STOP_LED_DATA_PIN, STOP_LED_CLK_PIN, STOP_LED_NUMBER);
	ESP_LOGI(TAG, "Done configuring SPI for STOP sign.");
	
	traffic_spi_init(VSPI_HOST, SPI_DMA_CH2, &spi_handle_go, &trans_desc_go, go_data,
			GO_LED_DATA_PIN, GO_LED_CLK_PIN, GO_LED_NUMBER);
	ESP_LOGI(TAG, "Done configuring SPI for GO sign.");

//	xTaskCreatePinnedToCore(
//		stop_driver,		/* Task's function. */
//		"Stop sign driver",	/* Name of the task. */
//		10000,			/* Stack size of the task */
//		NULL,			/* Parameter of the task */
//		2,			/* Priority of the task */
//		NULL,			/* Task handle to keep track of created task */
//		1			/* Pin task to core 1 */
//	);


//	xTaskCreatePinnedToCore(
//		go_driver,		/* Task's function. */
//		"Go sign driver",	/* Name of the task. */
//		10000,			/* Stack size of the task */
//		NULL,			/* Parameter of the task */
//		3,			/* Priority of the task */
//		NULL,			/* Task handle to keep track of created task */
//		1			/* Pin task to core 1 */
//	);
}
