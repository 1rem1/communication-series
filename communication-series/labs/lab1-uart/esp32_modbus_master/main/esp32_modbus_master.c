#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "esp_log.h"

#define UART_TX_PIN 4
#define UART_RX_PIN 5
#define UART_NUM UART_NUM_1

static const char *TAG = "esp32_modbus_master";

static void uart_init(uart_config_t *config) {
    uart_config_t uart_config = {
        .baud_rate = config->baud_rate,
        .data_bits = config->data_bits,
        .parity = config->parity,
        .stop_bits = config->stop_bits
    };

    uart_param_config(UART_NUM_1, &uart_config);
    uart_set_pin(UART_NUM_1, UART_TX_PIN, UART_RX_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    uart_driver_install(UART_NUM_1, 1024 * 2, 0, 0, NULL, 0);
}

static uint16_t Modbus_CRC16(uint8_t *data, uint16_t length) {
    uint16_t crc = 0xFFFF;
    for (uint16_t pos = 0; pos < length; pos++) {
        crc ^= (uint16_t)data[pos]; // XOR byte into least sig. byte of crc

        for (int i = 8; i != 0; i--) { // Loop over each bit
            if ((crc & 0x0001) != 0) { // If the LSB is set
                crc >>= 1; // Shift right
                crc ^= 0xA001; // XOR with polynomial
            } else {
                crc >>= 1; // Just shift right
            }
        }
    }
    return crc;
}

static void send_modbus_write_coil_and_wait_response(uint8_t slave_id, uint16_t coil_address, uint8_t coil_value) {
    uint8_t modbus_frame[8];
    modbus_frame[0] = slave_id;
    modbus_frame[1] = 0x05; // Function code for Write Single Coil
    modbus_frame[2] = (coil_address >> 8) & 0xFF; // High byte of coil address
    modbus_frame[3] = coil_address & 0xFF; // Low byte of coil address
    modbus_frame[4] = coil_value ? 0xFF : 0x00; // Coil value (ON/OFF)
    uint16_t crc = Modbus_CRC16(modbus_frame, 5); // Calculate CRC for first 5 bytes
    modbus_frame[5] = crc & 0xFF; // CRC low byte
    modbus_frame[6] = (crc >> 8) & 0xFF; // CRC high byte   
    uart_write_bytes(UART_NUM_1, (const char *)modbus_frame, sizeof(modbus_frame));
    ESP_LOGI(TAG, "Sent Modbus Write Coil command: Slave ID=%d, Coil Address=%d, Coil Value=%d", slave_id, coil_address, coil_value);

    // Wait for response (for simplicity, we just wait a fixed time here)
    uint8_t response[8];
    int len = uart_read_bytes(UART_NUM_1, response, sizeof(response), pdMS_TO_TICKS(2000)); // Wait for 2 seconds for response
    if (len > 0) {
        ESP_LOGI(TAG, "Received response: %02X %02X %02X %02X %02X %02X %02X %02X.", response[0], response[1], response[2], response[3], response[4], response[5], response[6], response[7]);
        if (response[1] == 0x05) {
            ESP_LOGI(TAG, "Successfully received valid response for Write Coil command.");
        }
    } else {
        ESP_LOGW(TAG, "No response received within timeout");
    }
}

void app_main(void)
{
    bool status = true;

    uart_config_t config = {
        .baud_rate = 9600,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1
    };

    uart_init(&config);

    while (1) {
        send_modbus_write_coil_and_wait_response(1, 0x0001, status); // Example: Write coil at address 0x0001 to ON
        status = !status; // Toggle coil value for demonstration
        vTaskDelay(pdMS_TO_TICKS(2000)); // Delay for 2 seconds before sending the next command
    }
}
