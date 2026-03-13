# 📘 Hướng dẫn Build & Flash Project STM32 — Từ A đến Z

> **Áp dụng cho mọi project STM32** được tạo bằng STM32CubeMX/CubeIDE, sử dụng HAL Driver và ARM GCC Toolchain.
>
> Ví dụ minh hoạ dùng board **Blue Pill (STM32F103C8T6)**, nhưng các bước hoàn toàn tương tự cho các dòng STM32 khác (F4, F7, H7, L4, G0, ...).

---

## 📋 Mục lục

1. [Phần cứng cần thiết](#1-phần-cứng-cần-thiết)
2. [Cài đặt phần mềm](#2-cài-đặt-phần-mềm)
3. [Tạo Project với STM32CubeMX](#3-tạo-project-với-stm32cubemx)
4. [Cấu trúc Project STM32 tiêu chuẩn](#4-cấu-trúc-project-stm32-tiêu-chuẩn)
5. [Viết Makefile cho bất kỳ Project nào](#5-viết-makefile-cho-bất-kỳ-project-nào)
6. [Build Project](#6-build-project)
7. [Nạp Firmware (Flash)](#7-nạp-firmware-flash)
8. [Debug](#8-debug)
9. [Xử lý lỗi thường gặp](#9-xử-lý-lỗi-thường-gặp)
10. [Tuỳ chỉnh cho dòng MCU khác](#10-tuỳ-chỉnh-cho-dòng-mcu-khác)

---

## 1. Phần cứng cần thiết

| # | Linh kiện | Mô tả |
|---|---|---|
| 1 | **Board STM32** | Bất kỳ board STM32 nào (Blue Pill, Nucleo, Discovery, custom PCB,...) |
| 2 | **Mạch nạp ST-Link V2** | Nạp và debug qua SWD. Bản gốc hoặc clone đều được |
| 3 | **Dây Dupont** | 4 dây cái-cái: 3.3V, GND, SWDIO, SWCLK |
| 4 | **Cáp USB** | Cấp nguồn cho board (nếu board có cổng USB) |

### Sơ đồ kết nối ST-Link ↔ Board STM32

```
ST-Link V2          Board STM32
───────────          ───────────
  3.3V  ──────────── 3.3V
  GND   ──────────── GND
  SWDIO ──────────── SWDIO (DIO)
  SWCLK ──────────── SWCLK (CLK)
```

> [!WARNING]
> **Không cấp nguồn 5V từ ST-Link vào chân 3.3V!** Nếu board có cổng USB riêng, nên cấp nguồn qua đó.

---

## 2. Cài đặt phần mềm

### 2.1. ARM GCC Toolchain (Bắt buộc)

Compiler biên dịch code C/C++ thành mã máy ARM.

**Ubuntu / Debian:**
```bash
sudo apt update
sudo apt install gcc-arm-none-eabi binutils-arm-none-eabi libnewlib-arm-none-eabi
```

**macOS (Homebrew):**
```bash
brew install --cask gcc-arm-embedded
```

**Windows:**
1. Tải từ [developer.arm.com](https://developer.arm.com/downloads/-/gnu-rm)
2. Chạy installer, tick **"Add path to environment variable"**

**Kiểm tra:**
```bash
arm-none-eabi-gcc --version
# Kết quả mong đợi: arm-none-eabi-gcc (...) 13.x.x hoặc mới hơn
```

### 2.2. Make (Bắt buộc nếu build bằng Makefile)

```bash
# Ubuntu/Debian
sudo apt install make

# macOS (đi kèm Xcode CLI Tools)
xcode-select --install

# Windows (dùng Chocolatey)
choco install make
```

### 2.3. STM32CubeProgrammer (Để nạp firmware)

1. Tải từ: [st.com/STM32CubeProg](https://www.st.com/en/development-tools/stm32cubeprog.html) (cần tài khoản ST miễn phí)
2. Cài đặt, ghi nhớ đường dẫn cài đặt:

| OS | Đường dẫn mặc định CLI |
|---|---|
| **Linux** | `/usr/local/STMicroelectronics/STM32Cube/STM32CubeProgrammer/bin/STM32_Programmer_CLI` |
| **macOS** | `/Applications/STMicroelectronics/.../bin/STM32_Programmer_CLI` |
| **Windows** | `C:\Program Files\STMicroelectronics\STM32Cube\STM32CubeProgrammer\bin\STM32_Programmer_CLI.exe` |

### 2.4. STM32CubeIDE (Tuỳ chọn — IDE tích hợp)

Bao gồm CubeMX + Compiler + Debugger trong 1 package:
- Tải từ: [st.com/STM32CubeIDE](https://www.st.com/en/development-tools/stm32cubeide.html)

---

## 3. Tạo Project với STM32CubeMX

### Bước 1 — Chọn MCU

1. Mở **STM32CubeMX** (hoặc STM32CubeIDE → File → New STM32 Project)
2. Tìm MCU theo part number (vd: `STM32F103C8`, `STM32F411CE`, `STM32H743ZI`,...)
3. Nhấn **Start Project**

### Bước 2 — Cấu hình Peripheral

- **Pinout & Configuration**: cấu hình GPIO, UART, SPI, I2C, Timer,...
- **Clock Configuration**: thiết lập nguồn clock (HSI/HSE) và PLL
- **Project Manager**:
  - **Toolchain/IDE**: chọn `STM32CubeIDE` hoặc `Makefile`
  - **Project Name**: đặt tên project
  - **Project Location**: chọn thư mục lưu

### Bước 3 — Generate Code

Nhấn **GENERATE CODE** → CubeMX tạo ra cấu trúc project chuẩn.

---

## 4. Cấu trúc Project STM32 tiêu chuẩn

Mọi project STM32 được CubeMX generate đều có cấu trúc tương tự:

```
<TÊN_PROJECT>/
├── Core/
│   ├── Inc/                              # Header files
│   │   ├── main.h                        # Header chính
│   │   ├── stm32<f>xx_hal_conf.h         # Cấu hình HAL (tuỳ dòng MCU)
│   │   └── stm32<f>xx_it.h              # Khai báo Interrupt handlers
│   ├── Src/                              # Source files
│   │   ├── main.c                        # ⭐ Code chính (viết logic ở đây)
│   │   ├── stm32<f>xx_hal_msp.c         # MCU Support Package init
│   │   ├── stm32<f>xx_it.c             # Interrupt handlers
│   │   ├── syscalls.c                    # System calls (newlib)
│   │   ├── sysmem.c                      # Memory management
│   │   └── system_stm32<f>xx.c          # System clock init
│   └── Startup/
│       └── startup_stm32<xxxx>.s         # Assembly startup + vector table
├── Drivers/
│   ├── CMSIS/                            # ARM CMSIS core headers
│   └── STM32<F>xx_HAL_Driver/            # ST HAL Library (tuỳ dòng MCU)
│       ├── Inc/
│       └── Src/
├── <TÊN_PROJECT>.ioc                     # File cấu hình CubeMX
├── STM32<XXXX>_FLASH.ld                  # ⭐ Linker script
├── Makefile                              # ⭐ Build script (nếu chọn Makefile)
└── Debug/                                # Thư mục output sau khi build
    ├── <TÊN_PROJECT>.elf                 # File ELF (có debug symbols)
    └── <TÊN_PROJECT>.bin                 # File BIN (nạp vào Flash)
```

> [!NOTE]
> `<f>` là ký hiệu dòng MCU viết thường (vd: `f1`, `f4`, `h7`, `g0`).
> `<F>` là viết hoa (vd: `F1`, `F4`).
> `<XXXX>` là mã MCU cụ thể (vd: `F103C8TX`, `F411CETX`).

### Vai trò từng thành phần

| Thành phần | Vai trò |
|---|---|
| `main.c` | Chứa logic chính: khởi tạo HAL → cấu hình clock → init peripheral → vòng lặp `while(1)` |
| `Makefile` | Script tự động hoá biên dịch, link và flash |
| `STM32<XXXX>_FLASH.ld` | Linker script: định nghĩa vùng nhớ FLASH và RAM của MCU cụ thể |
| `startup_stm32<xxxx>.s` | Assembly startup: khởi tạo stack pointer, vector table, gọi `main()` |
| `Drivers/` | Thư viện HAL và CMSIS do ST cung cấp |
| `<TÊN_PROJECT>.ioc` | File cấu hình CubeMX — dùng để re-generate code khi thay đổi cấu hình |

---

## 5. Viết Makefile cho bất kỳ Project nào

Nếu bạn không dùng STM32CubeIDE mà muốn build bằng command line, đây là template Makefile tổng quát.

> [!IMPORTANT]
> Khi tạo project mới, bạn **chỉ cần thay đổi các biến trong phần `THAY ĐỔI CHO PROJECT CỦA BẠN`**. Phần còn lại giữ nguyên.

```makefile
# ============================================================
# THAY ĐỔI CHO PROJECT CỦA BẠN
# ============================================================
TARGET    = <TÊN_PROJECT>                # Tên output (vd: BlinkLed, MyApp)
BUILD_DIR = Debug                        # Thư mục chứa output
MCU_FAMILY = STM32F1xx                   # Dòng MCU (STM32F1xx, STM32F4xx,...)
DEFINES   = -DSTM32F103xB -DUSE_HAL_DRIVER  # Macro defines (thay đổi theo MCU)
CPU       = -mcpu=cortex-m3             # CPU core (cortex-m0, m3, m4, m7,...)
FPU       =                             # FPU (để trống nếu Cortex-M3)
LDSCRIPT  = STM32F103C8TX_FLASH.ld      # Tên file linker script

# Đường dẫn STM32CubeProgrammer CLI
CLI = /đường/dẫn/đến/STM32_Programmer_CLI

# ============================================================
# KHÔNG CẦN THAY ĐỔI PHẦN DƯỚI (trừ khi thêm thư viện)
# ============================================================
PREFIX = arm-none-eabi-
CC = $(PREFIX)gcc
CP = $(PREFIX)objcopy
SZ = $(PREFIX)size

AS_SOURCES = $(shell find Core/Startup -name "*.s")
C_SOURCES  = $(shell find Core/Src Drivers/$(MCU_FAMILY)_HAL_Driver/Src -name "*.c")

INC_DIRS = -ICore/Inc \
           -IDrivers/$(MCU_FAMILY)_HAL_Driver/Inc \
           -IDrivers/$(MCU_FAMILY)_HAL_Driver/Inc/Legacy \
           -IDrivers/CMSIS/Device/ST/$(MCU_FAMILY)/Include \
           -IDrivers/CMSIS/Include

MCU_FLAGS = $(CPU) -mthumb $(FPU)
CFLAGS    = $(MCU_FLAGS) $(DEFINES) $(INC_DIRS) -Wall -g -Og \
            -ffunction-sections -fdata-sections
LDFLAGS   = $(MCU_FLAGS) -specs=nano.specs -T$(LDSCRIPT) -Wl,--gc-sections

# ---- Targets ----
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
```

### Bảng tham chiếu nhanh — Thay đổi gì cho MCU của bạn

| Dòng MCU | `CPU` | `FPU` | `DEFINES` | `MCU_FAMILY` |
|---|---|---|---|---|
| STM32F0xx | `-mcpu=cortex-m0` | *(trống)* | `-DSTM32F030x6` | `STM32F0xx` |
| STM32F1xx | `-mcpu=cortex-m3` | *(trống)* | `-DSTM32F103xB` | `STM32F1xx` |
| STM32F4xx | `-mcpu=cortex-m4` | `-mfpu=fpv4-sp-d16 -mfloat-abi=hard` | `-DSTM32F411xE` | `STM32F4xx` |
| STM32F7xx | `-mcpu=cortex-m7` | `-mfpu=fpv5-sp-d16 -mfloat-abi=hard` | `-DSTM32F746xx` | `STM32F7xx` |
| STM32H7xx | `-mcpu=cortex-m7` | `-mfpu=fpv5-d16 -mfloat-abi=hard` | `-DSTM32H743xx` | `STM32H7xx` |
| STM32G0xx | `-mcpu=cortex-m0plus` | *(trống)* | `-DSTM32G071xx` | `STM32G0xx` |
| STM32L4xx | `-mcpu=cortex-m4` | `-mfpu=fpv4-sp-d16 -mfloat-abi=hard` | `-DSTM32L476xx` | `STM32L4xx` |

> [!TIP]
> Macro `DEFINES` chính xác cho MCU của bạn có thể tìm trong file `.ioc` (dòng `ProjectManager.DeviceId`) hoặc file `.cproject` (dòng `definedsymbols`).

---

## 6. Build Project

### 6.1. Build bằng Makefile (Command Line)

```bash
# Di chuyển vào thư mục project
cd /đường/dẫn/đến/<TÊN_PROJECT>

# Build firmware
make all

# Build sạch từ đầu
make clean && make all

# Build + Flash ngay
make flash
```

**Kết quả build thành công** sẽ có dạng:

```
arm-none-eabi-gcc ... -o Debug/<TÊN_PROJECT>.elf
arm-none-eabi-size Debug/<TÊN_PROJECT>.elf
   text    data     bss     dec     hex filename
   XXXX      XX    XXXX   XXXX    XXXX Debug/<TÊN_PROJECT>.elf
arm-none-eabi-objcopy -O binary ... Debug/<TÊN_PROJECT>.bin
```

**Giải thích kích thước output:**

| Cột | Ý nghĩa | Nằm ở đâu |
|---|---|---|
| `text` | Code + hằng số | → FLASH |
| `data` | Biến có giá trị khởi tạo | → Copy FLASH → RAM |
| `bss` | Biến chưa khởi tạo (= 0) | → RAM |
| **Tổng FLASH** | `text + data` | Không được vượt quá dung lượng Flash của MCU |
| **Tổng RAM** | `data + bss` | Không được vượt quá dung lượng RAM của MCU |

### 6.2. Build bằng STM32CubeIDE

1. **Import Project**: File → Import → General → Existing Projects into Workspace → chọn thư mục project
2. **Build**: Click chuột phải project → **Build Project** (hoặc `Ctrl+B`)
3. Kiểm tra tab **Console** để xác nhận build thành công

### Ý nghĩa các Compiler Flag quan trọng

| Flag | Ý nghĩa |
|---|---|
| `-mcpu=cortex-mX` | Chọn CPU core |
| `-mthumb` | Sử dụng Thumb instruction set (bắt buộc cho Cortex-M) |
| `-mfpu=...` | Chọn loại FPU (chỉ Cortex-M4/M7 trở lên) |
| `-mfloat-abi=hard` | Dùng FPU phần cứng cho phép tính float |
| `-DSTM32Fxxx` | Macro xác định MCU → HAL sẽ include đúng header |
| `-DUSE_HAL_DRIVER` | Bật HAL Driver |
| `-Wall` | Bật tất cả cảnh báo |
| `-g` | Thêm debug symbols |
| `-Og` | Tối ưu cho debug (giữ nguyên flow code) |
| `-Os` | Tối ưu kích thước (dùng cho Release) |
| `-ffunction-sections` | Mỗi function = 1 section → loại bỏ function không dùng |
| `-fdata-sections` | Mỗi biến = 1 section → loại bỏ biến không dùng |
| `-specs=nano.specs` | Dùng newlib-nano (tiết kiệm Flash/RAM) |
| `-Wl,--gc-sections` | Linker loại bỏ section không tham chiếu |

---

## 7. Nạp Firmware (Flash)

### 7.1. Nạp bằng Makefile (CLI)

```bash
make flash
```

Lệnh thực tế:
```bash
STM32_Programmer_CLI -c port=SWD mode=UR -w Debug/<TÊN_PROJECT>.bin 0x08000000 -v -rst
```

| Tham số | Ý nghĩa |
|---|---|
| `-c port=SWD` | Kết nối qua giao thức SWD |
| `mode=UR` | Under Reset mode (ổn định, khuyên dùng) |
| `-w <file> 0x08000000` | Ghi file `.bin` vào địa chỉ Flash base |
| `-v` | Verify (so sánh dữ liệu sau khi ghi) |
| `-rst` | Reset MCU sau khi nạp xong |

> [!NOTE]
> Địa chỉ `0x08000000` là địa chỉ Flash base cho **hầu hết** các dòng STM32. Một số dòng đặc biệt có thể khác — kiểm tra trong linker script hoặc datasheet.

### 7.2. Nạp bằng STM32CubeProgrammer (GUI)

1. Mở **STM32CubeProgrammer**
2. Kết nối: chọn **ST-LINK** → nhấn **Connect**
3. Tab **Erasing & Programming**:
   - **File path**: chọn file `.bin` trong thư mục `Debug/`
   - **Start address**: `0x08000000`
   - ✅ **Verify programming**
   - ✅ **Run after programming**
4. Nhấn **Start Programming**

### 7.3. Nạp bằng STM32CubeIDE

1. Click **Run** → **Run Configurations...** (hoặc **Debug Configurations...**)
2. Chọn cấu hình debug đã có (file `.launch`)
3. Nhấn **Run** (hoặc **Debug**) — IDE tự build + flash + chạy

---

## 8. Debug

### 8.1. Debug bằng STM32CubeIDE

1. Cắm ST-Link → Click biểu tượng **Debug** 🪲
2. IDE sẽ flash firmware và dừng tại `main()`
3. Phím tắt:
   - **F6** — Step Over
   - **F5** — Step Into
   - **F7** — Step Return
   - **F8** — Resume
4. Xem biến, register, memory trong các panel tương ứng

### 8.2. Debug bằng GDB + OpenOCD (Command Line)

#### Cài OpenOCD

```bash
# Ubuntu/Debian
sudo apt install openocd
```

#### Terminal 1 — Chạy OpenOCD

```bash
# Thay target config phù hợp với MCU của bạn
openocd -f interface/stlink.cfg -f target/stm32f1x.cfg
```

Các file target config phổ biến:

| Dòng MCU | Target config |
|---|---|
| STM32F0 | `target/stm32f0x.cfg` |
| STM32F1 | `target/stm32f1x.cfg` |
| STM32F4 | `target/stm32f4x.cfg` |
| STM32F7 | `target/stm32f7x.cfg` |
| STM32H7 | `target/stm32h7x.cfg` |
| STM32G0 | `target/stm32g0x.cfg` |
| STM32L4 | `target/stm32l4x.cfg` |

#### Terminal 2 — Chạy GDB

```bash
arm-none-eabi-gdb Debug/<TÊN_PROJECT>.elf
```

```gdb
(gdb) target remote :3333       # Kết nối OpenOCD
(gdb) monitor reset halt        # Reset và dừng MCU
(gdb) load                      # Nạp firmware
(gdb) break main                # Breakpoint tại main
(gdb) continue                  # Chạy đến breakpoint
(gdb) next                      # Step over
(gdb) step                      # Step into
(gdb) print <tên_biến>          # Xem giá trị biến
(gdb) info registers            # Xem thanh ghi
(gdb) x/16xw 0x20000000         # Xem RAM (16 words từ địa chỉ 0x20000000)
```

---

## 9. Xử lý lỗi thường gặp

### ❌ `arm-none-eabi-gcc: command not found`

**Nguyên nhân**: Chưa cài toolchain hoặc chưa thêm vào PATH.

```bash
sudo apt install gcc-arm-none-eabi    # Ubuntu/Debian
which arm-none-eabi-gcc               # Kiểm tra
```

---

### ❌ `No ST-Link detected` / Không kết nối được ST-Link

**Kiểm tra theo thứ tự:**

1. Kiểm tra cáp USB và dây SWD
2. Kiểm tra USB có nhận:
   ```bash
   lsusb | grep -i "st-link"
   ```
3. Thêm udev rules (Linux):
   ```bash
   # Tạo /etc/udev/rules.d/49-stlink.rules:
   SUBSYSTEMS=="usb", ATTRS{idVendor}=="0483", ATTRS{idProduct}=="3748", MODE="0666"

   sudo udevadm control --reload-rules && sudo udevadm trigger
   ```

---

### ❌ `Error: Flash memory write failed`

1. Kiểm tra dây SWD (SWDIO, SWCLK, GND)
2. Thử giữ nút **RESET** trên board trong khi flash
3. Xoá toàn bộ Flash:
   ```bash
   STM32_Programmer_CLI -c port=SWD mode=UR -e all
   ```

---

### ❌ `Linker script not found`

Kiểm tra tên file `.ld` khớp chính xác với tên trong Makefile:
```bash
ls *.ld                    # Xem file linker script có tên gì
grep "FLASH.ld" Makefile   # Xem Makefile đang trỏ đến file nào
```

---

### ❌ `undefined reference to 'HAL_...'`

**Nguyên nhân**: Thiếu file source HAL tương ứng.

**Giải pháp**: Đảm bảo Makefile include đúng thư mục HAL Driver:
```makefile
C_SOURCES = $(shell find Core/Src Drivers/STM32<F>xx_HAL_Driver/Src -name "*.c")
```

---

### ❌ Firmware đã flash nhưng chương trình không chạy

Kiểm tra theo thứ tự:
1. **BOOT0 = 0** (nối GND) — MCU chạy từ Flash
2. Nguồn cấp đủ 3.3V
3. Verify firmware đã ghi đúng:
   ```bash
   STM32_Programmer_CLI -c port=SWD -r 0x08000000 0x100
   ```

---

## 10. Tuỳ chỉnh cho dòng MCU khác

### Quy trình 3 bước khi chuyển sang MCU mới

1. **Tạo project bằng CubeMX** cho MCU mới → generate code
2. **Copy template Makefile** ở [Mục 5](#5-viết-makefile-cho-bất-kỳ-project-nào) vào project
3. **Thay đổi 6 biến** theo bảng ở Mục 5:
   - `TARGET`, `MCU_FAMILY`, `DEFINES`, `CPU`, `FPU`, `LDSCRIPT`

### Ví dụ: Chuyển từ STM32F103 sang STM32F411

```makefile
# CŨ (STM32F103 - Cortex-M3)
CPU       = -mcpu=cortex-m3
FPU       =
DEFINES   = -DSTM32F103xB -DUSE_HAL_DRIVER
MCU_FAMILY = STM32F1xx
LDSCRIPT  = STM32F103C8TX_FLASH.ld

# MỚI (STM32F411 - Cortex-M4 có FPU)
CPU       = -mcpu=cortex-m4
FPU       = -mfpu=fpv4-sp-d16 -mfloat-abi=hard
DEFINES   = -DSTM32F411xE -DUSE_HAL_DRIVER
MCU_FAMILY = STM32F4xx
LDSCRIPT  = STM32F411CETX_FLASH.ld
```

---

## 📎 Tham khảo

| Tài liệu | Nội dung |
|---|---|
| [STM32CubeMX User Manual](https://www.st.com/resource/en/user_manual/um1718-stm32cubemx-for-new-design-stmicroelectronics.pdf) | Hướng dẫn dùng CubeMX |
| [STM32CubeProgrammer Guide](https://www.st.com/resource/en/user_manual/um2237-stm32cubeprogrammer-software-description-stmicroelectronics.pdf) | Hướng dẫn nạp firmware |
| [ARM GCC Toolchain](https://developer.arm.com/downloads/-/gnu-rm) | Tải toolchain |
| [OpenOCD Documentation](https://openocd.org/doc/html/index.html) | Debug với OpenOCD |
| [STM32 MCU Finder](https://www.st.com/en/microcontrollers-microprocessors/stm32-32-bit-arm-cortex-mcus.html) | Tra cứu thông số MCU |

---

> *Tài liệu được tạo bởi Antigravity — 10/03/2026*
