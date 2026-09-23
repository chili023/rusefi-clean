# uaEFI moto

uaEFI derivative for motorcycle use:

* two TE Superseal 1.0 34-pin headers (A: 6437288-1 keying 1, power/outputs; B: 6437288-2 keying 2, sensors)
* four MAX31855 EGT channels with miniature type K thermocouple sockets on the side
* two on-board LSU 4.9 wideband controllers, strapped to hardware index 1 (SEL1 low, SEL2 floating) and
  5 (SEL1 floating, SEL2 high). Wideband firmware maps hardware index 1 to Lambda 1 and 5 to Lambda 2,
  no manual CAN index setup needed on a fresh module.
* no electronic throttle H-bridges (their MCU pins are used as EGT chip selects)
* CR2032 backup battery on VBAT: keeps engine hour counters in RTC backup registers

Base hardware: https://github.com/rusefi/uaefi
