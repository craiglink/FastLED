from pathlib import Path

HERE = Path(__file__).resolve().parent
PROJECT_ROOT = HERE.parent
PLATFORMIO_INI = PROJECT_ROOT / "platformio.ini"

ESP32C6 = """
[platformio]
src_dir = dev ; target is ./dev/dev.ino

[env:dev]
; This is the espressif32 platform which is the 4.1 toolchain as of 2024-Aug-23rd
; platform = espressif32
; The following platform enables the espressif32 platform to use the 5.1 toolchain, simulating
; the new Arduino 2.3.1+ toolchain.

# Developement branch of the open source espressif32 platform
platform = https://github.com/pioarduino/platform-espressif32/releases/download/53.03.10/platform-espressif32.zip

framework = arduino
board = esp32-c6-devkitc-1

upload_protocol = esptool

monitor_filters = 
	default
	esp32_exception_decoder  ; Decode exceptions so that they are human readable.

; Symlink in the FastLED library so that changes to the library are reflected in the project
; build immediatly.
lib_deps =
  FastLED=symlink://./

build_type = debug

build_flags =
	-DDEBUG
    -g
    -Og
    -DCORE_DEBUG_LEVEL=5
    -DLOG_LOCAL_LEVEL=ESP_LOG_VERBOSE
    ;-DFASTLED_RMT5=1
    -DFASTLED_ESP32_SPI_BULK_TRANSFER=1
    -DENABLE_ESP32_I2S_YVES_DRIVER=1
    
check_tool = clangtidy
"""

ESP32S3 = """
[platformio]
src_dir = dev ; target is ./dev/dev.ino

[env:dev]
; This is the espressif32 platform which is the 4.1 toolchain as of 2024-Aug-23rd
; platform = espressif32
; The following platform enables the espressif32 platform to use the 5.1 toolchain, simulating
; the new Arduino 2.3.1+ toolchain.

# Developement branch of the open source espressif32 platform
platform = https://github.com/pioarduino/platform-espressif32/releases/download/53.03.10/platform-espressif32.zip

framework = arduino
board = esp32-s3-devkitc-1

upload_protocol = esptool

monitor_filters = 
	default
	esp32_exception_decoder  ; Decode exceptions so that they are human readable.

; Symlink in the FastLED library so that changes to the library are reflected in the project
; build immediatly.
lib_deps =
  FastLED=symlink://./

build_type = debug

build_flags =
	-DDEBUG
    -g
    -Og
    -DCORE_DEBUG_LEVEL=5
    -DLOG_LOCAL_LEVEL=ESP_LOG_VERBOSE
    ;-DFASTLED_RMT5=1
    -DFASTLED_ESP32_SPI_BULK_TRANSFER=1
    -DENABLE_ESP32_I2S_YVES_DRIVER=1
    
check_tool = clangtidy
"""