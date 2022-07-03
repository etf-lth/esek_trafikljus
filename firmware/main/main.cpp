
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

#define STATIC      0
#define SHIFTING    1
#define SWEEP       2
#define RAINBOW     3


using namespace std;

extern "C" {
	void app_main();
}

const char *TAG = "Traffic Light";

void led_color(char *leds, int led_number, char red, char green, char blue);

/** @brief	Converts HSV color representation to RGB.
 *	@param	hue	Color from color wheel
 *	@param	saturation Color saturation
 *	@param	value Brightness
 *	@return RGB (Padding (8 bits), Red (8 bits), Green (8 bits), Blue (8 bits))
 */
uint32_t hsv_to_rgb(uint16_t hue, uint8_t saturation, uint8_t value);

/** @brief	Stop sign controller
 */
void stop_driver(void *pvParameters);

/** @brief	Go sign controller
 */
void go_driver(void *pvParameters);


void traffic_spi_init(spi_host_device_t spi_slot, spi_dma_chan_t dma_channel, spi_device_handle_t *spi_handle,
		spi_transaction_t *trans_desc, char *led_data, int data_pin, int clk_pin, int led_number);


void led_color(char *leds, int led_number, char red, char green, char blue)
{
	leds[4*led_number + 0] = 0xff;  // TODO: allocate this in the beginning, it does not change
	leds[4*led_number + 1] = red;
	leds[4*led_number + 2] = blue;
	leds[4*led_number + 3] = green;
}

uint32_t hsv_to_rgb(uint16_t hue, uint8_t saturation, uint8_t value)
{
	uint8_t remainder = (hue % 60) * 255 / 60;
	uint8_t p = (value * (255 - saturation)) >> 8;
	uint8_t q = (value * (255 - ((saturation * remainder) >> 8))) >> 8;
	uint8_t t = (value * (255 - ((saturation * (255 - remainder)) >> 8))) >> 8;
	uint32_t red;
	uint32_t green;
	uint32_t blue;

	switch((hue / 60) % 6){
		case 0:	// (0 <= deg < 60)
			red = value;
			green = t;
			blue = p;
			break;
		case 1:	// (60 <= deg < 120)
			red = q;
			green = value;
			blue = p;
			break;
		case 2:	// (120 <= deg < 180)
			red = p;
			green = value;
			blue = t;
			break;
		case 3:	// (180 <= deg < 240)
			red = p;
			green = q;
			blue = value;
			break;
		case 4:	// (240 <= deg < 300)
			red = t;
			green = p;
			blue = value;
			break;
		default:	// (300 <= deg < 360)
			red = value;
			green = p;
			blue = q;
			break;
	}
	return (red << 16 | green << 8 | blue);
}

void traffic_spi_init(spi_host_device_t spi_slot, spi_dma_chan_t dma_channel, spi_device_handle_t *spi_handle,
		spi_transaction_t *trans_desc, char *led_data, int data_pin, int clk_pin, int led_number)
{
	// SPI bus configuration
	spi_bus_config_t spi_config;
	memset(&spi_config, 0, sizeof(spi_bus_config_t));
	spi_config.sclk_io_num = clk_pin;
	spi_config.mosi_io_num = data_pin;
	spi_config.miso_io_num = -1;
	spi_config.quadwp_io_num = -1;
	spi_config.quadhd_io_num = -1;
	spi_config.max_transfer_sz = 4*(led_number+2);
	spi_config.flags = SPICOMMON_BUSFLAG_GPIO_PINS;
	ESP_ERROR_CHECK(spi_bus_initialize(spi_slot, &spi_config, dma_channel));

	// Device configuration
	spi_device_interface_config_t dev_config;
	memset(&dev_config, 0, sizeof(spi_device_interface_config_t));
	dev_config.address_bits = 0;
	dev_config.command_bits = 0;
	dev_config.dummy_bits = 0;
	dev_config.mode = 0;
	dev_config.duty_cycle_pos = 0;
	dev_config.cs_ena_posttrans = 0;
	dev_config.cs_ena_pretrans = 0;
	dev_config.clock_speed_hz = SPI_CLOCK;
	dev_config.spics_io_num = -1;
	dev_config.flags = 0;
	dev_config.queue_size = 1;
	dev_config.pre_cb = nullptr;
	dev_config.post_cb = nullptr;
	ESP_ERROR_CHECK(spi_bus_add_device(spi_slot, &dev_config, spi_handle));

	ESP_ERROR_CHECK(spi_device_acquire_bus(*spi_handle, portMAX_DELAY));
	// SPI transaction description
	memset(trans_desc, 0, sizeof(spi_transaction_t));
	trans_desc->addr = 0;
	trans_desc->cmd = 0;
	trans_desc->flags = 0;
	trans_desc->length = (2 + led_number) * 32;
	trans_desc->rxlength = 0;
	trans_desc->tx_buffer = led_data;
	trans_desc->rx_buffer = nullptr;
}

void stop_driver(void *pvParameters)
{
	ESP_LOGI(TAG, "Setting up SPI for STOP sign.");
	spi_device_handle_t spi_handle;
	spi_transaction_t trans_desc;
	char data[4*(STOP_LED_NUMBER+2)];

	traffic_spi_init(HSPI_HOST, SPI_DMA_CH1, &spi_handle, &trans_desc, data,
			STOP_LED_DATA_PIN, STOP_LED_CLK_PIN, STOP_LED_NUMBER);
	ESP_LOGI(TAG, "Done configuring SPI for STOP sign.");

	// Start frame
	data[0] = 0x00;
	data[1] = 0x00;
	data[2] = 0x00;
	data[3] = 0x00;

	// End frame
	data[(STOP_LED_NUMBER+1)*4 + 0] = 0xff;
	data[(STOP_LED_NUMBER+1)*4 + 1] = 0xff;
	data[(STOP_LED_NUMBER+1)*4 + 2] = 0xff;
	data[(STOP_LED_NUMBER+1)*4 + 3] = 0xff;

	while(true) { 

		for (int led = 1; led <= STOP_LED_NUMBER; led++){
			led_color(data, led, 0x7f, 0x00, 0x00);	// rgb
		}
		ESP_ERROR_CHECK(spi_device_queue_trans(spi_handle, &trans_desc, portMAX_DELAY));
		vTaskDelay(10000 / portTICK_RATE_MS);
	}
}

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

	while(true) {

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
					uint32_t colors_hsv = hsv_to_rgb((uint32_t)x * 359 / 256, 255, 255);
					led_color(data, led, (char) (colors_hsv >> 16) & 0x7f,
						(char) (colors_hsv >> 8) & 0x7f, (char) colors_hsv & 0x7f); // rgb
				}
				break;
		}

		ESP_ERROR_CHECK(spi_device_queue_trans(spi_handle, &trans_desc, portMAX_DELAY));
		vTaskDelay(157 / portTICK_RATE_MS);
	}
}


void app_main()
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
	
//	xTaskCreatePinnedToCore(
//		stop_driver,	/* Task's function. */
//		"Stop sign driver",	/* Name of the task. */
//		10000,			/* Stack size of the task */
//		nullptr,			/* Parameter of the task */
//		2,				/* Priority of the task */
//		nullptr,			/* Task handle to keep track of created task */
//		1);				/* Pin task to core 1 */
//
//
//	xTaskCreatePinnedToCore(
//		go_driver,		/* Task's function. */
//		"Go sign driver",	/* Name of the task. */
//		10000,			/* Stack size of the task */
//		nullptr,			/* Parameter of the task */
//		3,				/* Priority of the task */
//		nullptr,			/* Task handle to keep track of created task */
//		1);				/* Pin task to core 1 */

	
}
