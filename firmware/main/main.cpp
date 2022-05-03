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

#include "webserver.h"
#include "routes.h"

// TODO: Move out
#define SPI_CLOCK 				1000000

#define GO_LED_CLK_PIN			18
#define GO_LED_DATA_PIN			19
#define GO_LED_NUMBER			64

#define STOP_LED_CLK_PIN		22
#define STOP_LED_DATA_PIN		23
#define STOP_LED_NUMBER			60

using namespace std;

extern "C" {
void app_main();
}

const char *TAG = "Traffic Light";

void led_color(char *leds, uint8_t led_number, char r, char g, char b);
void stop_driver(void *pvParameters);
void go_driver(void *pvParameters);
void traffic_spi_init(spi_device_handle_t *spi_handle, spi_transaction_t *trans_desc,
		char *led_data, int data_pin, int clk_pin, int led_number);


void led_color(char *leds, uint8_t led_number, char r, char g, char b)
{   
    leds[4*led_number + 0] = 0xff;  // TODO: allocate this in the beginning, it does not change
    leds[4*led_number + 1] = r;
    leds[4*led_number + 2] = b;
    leds[4*led_number + 3] = g;
}


void traffic_spi_init(spi_device_handle_t *spi_handle, spi_transaction_t *trans_desc,
		char *led_data, int data_pin, int clk_pin, int led_number)
{
	// SPI bus configuration
    spi_bus_config_t spi_config;
	memset(&spi_config, 0, sizeof(spi_bus_config_t));
    spi_config.sclk_io_num = clk_pin;
    spi_config.mosi_io_num = data_pin,
    spi_config.miso_io_num = -1,
    spi_config.quadwp_io_num = -1;
    spi_config.quadhd_io_num = -1;
    spi_config.max_transfer_sz = 4*(led_number+2);
    spi_config.flags = SPICOMMON_BUSFLAG_MASTER;
	ESP_ERROR_CHECK(spi_bus_initialize(HSPI_HOST, &spi_config, 1));

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
    dev_config.pre_cb = NULL;
    dev_config.post_cb = NULL;
	ESP_ERROR_CHECK(spi_bus_add_device(HSPI_HOST, &dev_config, spi_handle));

	ESP_ERROR_CHECK(spi_device_acquire_bus(*spi_handle, portMAX_DELAY));

	// SPI transaction description
	memset(trans_desc, 0, sizeof(spi_transaction_t));
    trans_desc->addr = 0;
    trans_desc->cmd = 0;
    trans_desc->flags = 0;
    trans_desc->length = 32*(led_number+2);
    trans_desc->rxlength = 0;
    trans_desc->tx_buffer = led_data;
    trans_desc->rx_buffer = 0;
}

void stop_driver(void *pvParameters)
{
	ESP_LOGI(TAG, "Setting up SPI for STOP sign.");
	spi_device_handle_t spi_handle;
	spi_transaction_t trans_desc;
	char data[4*(STOP_LED_NUMBER+2)];

	traffic_spi_init(&spi_handle, &trans_desc, data,
			STOP_LED_DATA_PIN, STOP_LED_CLK_PIN, STOP_LED_NUMBER);
	/*
    // SPI bus configuration
    spi_bus_config_t spi_config;
	memset(&spi_config, 0, sizeof(spi_bus_config_t));
    spi_config.sclk_io_num = STOP_LED_CLK_PIN;
    spi_config.mosi_io_num = STOP_LED_DATA_PIN,
    spi_config.miso_io_num = -1,
    spi_config.quadwp_io_num = -1;
    spi_config.quadhd_io_num = -1;
    spi_config.max_transfer_sz = 4*(STOP_LED_NUMBER+2);
    spi_config.flags = SPICOMMON_BUSFLAG_MASTER;

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
    dev_config.pre_cb = NULL;
    dev_config.post_cb = NULL;

	// SPI transaction description
	spi_transaction_t trans_desc;
	memset(&trans_desc, 0, sizeof(spi_transaction_t));
    trans_desc.addr = 0;
    trans_desc.cmd = 0;
    trans_desc.flags = 0;
    trans_desc.length = 32*(STOP_LED_NUMBER+2);
    trans_desc.rxlength = 0;
    trans_desc.tx_buffer = data;
    trans_desc.rx_buffer = 0;

	ESP_ERROR_CHECK(spi_bus_initialize(HSPI_HOST, &spi_config, 1));
    ESP_ERROR_CHECK(spi_bus_add_device(HSPI_HOST, &dev_config, &spi_handle));
	ESP_ERROR_CHECK(spi_device_acquire_bus(spi_handle, portMAX_DELAY));
	*/
	ESP_LOGI(TAG, "Done confgiguring SPI device.");

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
	
}


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
	//ESP_ERROR_CHECK(example_connect());

	// add handlers for the URLs
	g_webServer.addHandler(&g_root)
	           .addHandler(&g_echo)
	           .addHandler(&g_about);

	xTaskCreatePinnedToCore(
        stop_driver,	/* Task's function. */
        "Stop sign driver",	/* Name of the task. */
        10000,			/* Stack size of the task */
        NULL,			/* Parameter of the task */
        2,				/* Priority of the task */
        NULL,			/* Task handle to keep track of created task */
        1);				/* Pin task to core 1 */


	xTaskCreatePinnedToCore(
        go_driver,		/* Task's function. */
        "Go sign driver",	/* Name of the task. */
        10000,			/* Stack size of the task */
        NULL,			/* Parameter of the task */
        2,				/* Priority of the task */
        NULL,			/* Task handle to keep track of created task */
        1);				/* Pin task to core 1 */
}
