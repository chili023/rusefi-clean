# PyST ECU Rev 1

rusEFI board configuration for the PyST ECU (own design, STM32F407VGT6, derived from the uaEFI concept; pinout is **not**
the Hellen MM100 pinout any more, see `board_configuration.cpp`). Hardware repo: https://github.com/chili023/code_pst-ECU
(`Firmware/KONZEPT_FIRMWARE.md`, `Firmware/doku/stecker.md`).

* two Molex CMC 32 headers: A (black) = base harness, B (brown) = extension
* 6 injectors, 6 logic level coil outputs (channels 5/6 on test points only), 4 GPPWM low side with flyback, 2 weak low side
* crank hall 1 (PE12), cam hall 2 (PE13), wheel speed hall 3 (PE14), rear wheel on PA4, VR inputs on test points
* 2 on-board LSU 4.9 wideband controllers (rusEFI wideband, hardware index 1 and 5), 4 x MAX31855 (EGT1/2 on header A)
* LPS22HB baro on I2C2, SD card and LIS2DH12 on SPI1, CR2032 on VBAT (engine hour counters in RTC backup registers)
* no electronic throttle on the ECU: a stacked CAN module drives the throttle (J1201)
* ESP32-C3 "PyST-Dash-Bridge" on UART2 (PD5/PD6, 460800 baud TunerStudio protocol), reset via PB2
* Features from the `uaefi-moto` branch: five persistent engine hour counters, Alpha-N barometric compensation

Firmware ID `pyst-ecu`. Build: `.github/workflows/build-pyst-ecu.yaml` (push to branch `pyst-ecu`).
