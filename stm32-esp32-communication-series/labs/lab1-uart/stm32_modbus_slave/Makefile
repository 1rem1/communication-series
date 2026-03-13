TARGET = stm32-modbus-slave
BUILD_DIR = Debug
CLI = /home/manus/STMicroelectronics/STM32Cube/STM32CubeProgrammer/bin/STM32_Programmer_CLI

PREFIX = arm-none-eabi-
CC = $(PREFIX)gcc
CP = $(PREFIX)objcopy
SZ = $(PREFIX)size

DEFINES = -DSTM32F103xB -DUSE_HAL_DRIVER

# IMPORTANT: Include Startup file
AS_SOURCES = Core/Startup/startup_stm32f103c8tx.s
C_SOURCES = $(shell find Core Drivers Middlewares -name "*.c")

INC_DIRS = $(shell find Core Drivers Middlewares -type d | sed 's/^/-I/')

CPU = -mcpu=cortex-m3
MCU = $(CPU) -mthumb
CFLAGS = $(MCU) $(DEFINES) $(INC_DIRS) -Wall -g -Og -ffunction-sections -fdata-sections
# Đảm bảo tên file .ld đúng với file trong thư mục của bạn
LDFLAGS = $(MCU) -specs=nano.specs -TSTM32F103C8TX_FLASH.ld -Wl,--gc-sections

all: $(BUILD_DIR)/$(TARGET).bin

$(BUILD_DIR)/$(TARGET).elf: $(C_SOURCES) $(AS_SOURCES)
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $(LDFLAGS) $(AS_SOURCES) $(C_SOURCES) -o $@
	$(SZ) $@

$(BUILD_DIR)/$(TARGET).bin: $(BUILD_DIR)/$(TARGET).elf
	$(CP) -O binary $< $@

flash: all
	$(CLI) -c port=SWD mode=UR -w $(BUILD_DIR)/$(TARGET).bin 0x08000000 -v -rst

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all flash clean