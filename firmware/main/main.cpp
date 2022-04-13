/* Simple HTTP Server Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <iostream>

#include <esp_wifi.h>
#include <esp_event.h>
#include <esp_log.h>
#include <esp_system.h>
#include <nvs_flash.h>
#include <sys/param.h>
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_eth.h"
#include "protocol_examples_common.h"

#include <esp_http_server.h>

#include "webserver.h"
#include "routes.h"

using namespace std;

extern "C" {
void app_main();
}

const char *TAG = "Traffic Light";


void app_main(void)
{
	ESP_ERROR_CHECK(nvs_flash_init());
	ESP_ERROR_CHECK(esp_netif_init());

	// set up a default event loop
	ESP_ERROR_CHECK(esp_event_loop_create_default());

	/**
	 * Register two events and bind the callbacks to the WiFi object.
	 * We want to know when we have an IP, and when we disconnet from the WiFi.
	 * We can not bind a non-static member function since the caller needs a reference to the object (this).
	 * Instead, we call a static member function and pass the object reference (this) as an argument.
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
	g_webServer.addHandler(&g_root).addHandler(&g_echo);
}
