
#pragma once

#include <driver/spi_master.h>
#include "io.h"

void traffic_spi_init(spi_host_device_t spi_slot, spi_dma_chan_t dma_channel, spi_device_handle_t *spi_handle,
		spi_transaction_t *trans_desc, char *led_data, int data_pin, int clk_pin, int led_number);

extern spi_device_handle_t spi_handle_stop;
extern spi_transaction_t trans_desc_stop;
extern char stop_data[4 * (STOP_LED_NUMBER + 2)];
