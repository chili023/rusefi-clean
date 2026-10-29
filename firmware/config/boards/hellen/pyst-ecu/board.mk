# PyST ECU Rev 1 - own board, STM32F407VGT6, derived from the uaEFI / uaefi-moto configuration.
# Pinout is NOT the Hellen MM100 pinout (see board_configuration.cpp and readme.md).

BOARDCPPSRC = $(BOARD_DIR)/board_configuration.cpp

DDEFS += -DFIRMWARE_ID=\"pyst-ecu\" $(VAR_DEF_ENGINE_TYPE)
DDEFS += -DEFI_SOFTWARE_KNOCK=TRUE -DSTM32_ADC_USE_ADC3=TRUE
# four MAX31855 EGT chips on SPI3
DDEFS += -DEFI_MAX_31855=TRUE

# no board ID resistors on the PyST ECU
DDEFS += -DHW_HELLEN_SKIP_BOARD_TYPE=TRUE
DDEFS += -DSTATIC_BOARD_ID=BOARD_ID_UAEFI_B

# keep the uaEFI code paths (engine presets etc.)
DDEFS += -DHW_HELLEN_UAEFI=1
DDEFS += -DHW_PYST_ECU=1
# 5 V sensor supply power good input
DDEFS += -DDIAG_5VP_PIN=Gpio::B12

# LED1 (red) on PA15, see getCommsLedPin & co in board_configuration.cpp for the others
LED_CRITICAL_ERROR_BRAIN_PIN = -DLED_CRITICAL_ERROR_BRAIN_PIN=Gpio::A15

DDEFS += -DSTM32_ICU_USE_TIM1=TRUE -DSTM32_PWM_USE_TIM1=FALSE
DDEFS += -DEFI_SENT_SUPPORT=TRUE

ONBOARD_MEMS_TYPE=LIS2DH12

include $(BOARDS_DIR)/hellen/hellen-common100.mk

# we do not have much Lua RAM, let's drop some fancy functions
DDEFS += -DWITH_LUA_CONSUMPTION=FALSE
DDEFS += -DWITH_LUA_PID=FALSE
DDEFS += -DWITH_LUA_STOP_ENGINE=FALSE

# UART2 (PD5 TX / PD6 RX) is the TunerStudio serial port, wired to the on-board ESP32-C3 dash bridge
DDEFS += $(PRIMARY_COMMUNICATION_PORT_USART2)

DDEFS += -DUSB_DESCRIPTOR_B_LENGTH=30
DDEFS += -DUSB_DESCRIPTOR_STRING_CONTENT="'r', 0, 'u', 0, 's', 0, 'E', 0, 'F', 0, 'I', 0, ' ', 0, 'P', 0, 'y', 0, 'S', 0, 'T', 0, ' ', 0, 'E', 0, 'C', 0, 'U', 0"
