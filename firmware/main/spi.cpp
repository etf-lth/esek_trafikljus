
#include "spi.h"
#include "io.h"
#include <cstring>


// declare the spi handles and data for go and stop signs
spi_device_handle_t spi_handle_stop;
spi_transaction_t trans_desc_stop;
char stop_data[4 * (STOP_LED_NUMBER + 2)];

spi_device_handle_t spi_handle_go;
spi_transaction_t trans_desc_go;
char go_data[4 * (GO_LED_NUMBER + 2)];


void traffic_spi_init(spi_host_device_t spi_slot, spi_dma_chan_t dma_channel, spi_device_handle_t *spi_handle,
		spi_transaction_t *trans_desc, char *led_data, int data_pin, int clk_pin, int led_number)
{
	// SPI bus configuration
	spi_bus_config_t spi_config{};
	spi_config.sclk_io_num = clk_pin;
	spi_config.mosi_io_num = data_pin,
	spi_config.miso_io_num = -1,
	spi_config.quadwp_io_num = -1;
	spi_config.quadhd_io_num = -1;
	spi_config.max_transfer_sz = 4*(led_number+2);
	spi_config.flags = SPICOMMON_BUSFLAG_GPIO_PINS;
	ESP_ERROR_CHECK(spi_bus_initialize(spi_slot, &spi_config, dma_channel));

	// Device configuration
	spi_device_interface_config_t dev_config{};
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
	trans_desc->length = 32*(led_number+2);
	trans_desc->rxlength = 0;
	trans_desc->tx_buffer = led_data;
	trans_desc->rx_buffer = 0;
}
