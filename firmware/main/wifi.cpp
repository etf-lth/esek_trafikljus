#include "wifi.h"

#include <iostream>

#include <esp_wifi.h>
#include <esp_err.h>

WiFi::WiFi()
{
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    std::cout << "Init WiFi" << std::endl;
}

WiFi::~WiFi()
{

}