# uaEFI moto

uaEFI derivative for motorcycle use:

* two TE Superseal 1.0 34-pin headers: A (6437288-1, keying 1) = base harness (everything "1": INJ/IGN/GPPWM 1-2,
  TPS1, CLT, IAT, crank hall, wideband 1, USB, CAN, relay 1, knock), B (6437288-2, keying 2) = extension (INJ/IGN/GPPWM 3-4,
  wideband 2, VR, cam hall, wheel sensors, analog inputs, relay 2)
* 4 injectors and 4 coils (channels 5/6 of the uaEFI are not on the connectors)
* two wheel speed inputs: B20 front (VSS), B28 rear (aux speed 1, R39 populated as pull-up, R36 not populated)
* four MAX31855 EGT channels with miniature type K thermocouple sockets on the side
* two on-board LSU 4.9 wideband controllers, strapped to hardware index 1 (SEL1 low, SEL2 floating) and
  5 (SEL1 floating, SEL2 high). Wideband firmware maps hardware index 1 to Lambda 1 and 5 to Lambda 2,
  no manual CAN index setup needed on a fresh module.
* no electronic throttle H-bridges (their MCU pins are used as EGT chip selects)
* CR2032 backup battery on VBAT: keeps engine hour counters in RTC backup registers

Base hardware: https://github.com/rusefi/uaefi
