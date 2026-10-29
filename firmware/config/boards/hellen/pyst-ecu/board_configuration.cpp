/**
 * @file boards/hellen/pyst-ecu/board_configuration.cpp
 *
 * PyST ECU Rev 1 - own motorcycle/dyno ECU design (https://github.com/chili023/code_pst-ECU), STM32F407VGT6.
 * Derived from the uaEFI / uaefi-moto configuration, but the MCU pinout was re-sorted for the PCB block plan
 * (06.-09.10.2026) so the Hellen MM100 pin names do NOT apply any more. All pins below come from
 * Hardware/generator/mcu_pins.py + ecu.py of the hardware repo, connector pins from connectors/A.yaml, B.yaml.
 *
 * Unchanged versus uaEFI: USB (PA11/PA12), SWD, CAN1 (PD0/PD1), SPI1 (PB3/4/5), SPI3 (PC10/PC11), I2C2 (PB10/PB11),
 * UART2 (PD5/PD6, TunerStudio serial port = on-board ESP32-C3 dash bridge), VR threshold PWM PD14, knock on PA2/ADC3.
 */

#include "pch.h"
#include "defaults.h"
#include "hellen_meta.h"
#include "board_overrides.h"

// --- LEDs: LED1 red PA15 (critical error, see board.mk), LED2 PB7, LED3 PB6, LED4 PD7
Gpio getCommsLedPin() {
	return Gpio::B6; // LED3
}

Gpio getRunningLedPin() {
	return Gpio::B7; // LED2
}

Gpio getWarningLedPin() {
	return Gpio::D7; // LED4
}

// injectors 5/6 and coils 5/6 exist on the PCB but end on test points (no connector pin)
static void setInjectorPins() {
	engineConfiguration->injectionPins[0] = Gpio::E10; // A2
	engineConfiguration->injectionPins[1] = Gpio::E8;  // A5
	engineConfiguration->injectionPins[2] = Gpio::C8;  // B5
	engineConfiguration->injectionPins[3] = Gpio::C6;  // B6
	engineConfiguration->injectionPins[4] = Gpio::C12; // test point
	engineConfiguration->injectionPins[5] = Gpio::B8;  // test point
}

static void setIgnitionPins() {
	engineConfiguration->ignitionPins[0] = Gpio::B15; // A15
	engineConfiguration->ignitionPins[1] = Gpio::D13; // B13
	engineConfiguration->ignitionPins[2] = Gpio::D8;  // B22
	engineConfiguration->ignitionPins[3] = Gpio::D10; // B25
	engineConfiguration->ignitionPins[4] = Gpio::D12; // test point
	engineConfiguration->ignitionPins[5] = Gpio::B13; // test point
}

static void setGppwmPins() {
	// all four with flyback diode
	engineConfiguration->gppwm[0].pin = Gpio::E7;  // A3 GPPWM1
	engineConfiguration->gppwm[1].pin = Gpio::E9;  // A4 GPPWM2
	engineConfiguration->gppwm[2].pin = Gpio::A8;  // B7 GPPWM3
	// GPPWM4 (PD15, B8) is the default fuel pump output, see below
}

static void setEgtPins() {
	// SPI3, MOSI not needed, one-way communication; PC12 is injector 5 on this board!
	engineConfiguration->is_enabled_spi_3 = true;
	engineConfiguration->spi3misoPin = Gpio::C11;
	engineConfiguration->spi3sckPin = Gpio::C10;
	engineConfiguration->spi3mosiPin = Gpio::Unassigned;
	engineConfiguration->max31855spiDevice = SPI_DEVICE_3;

	engineConfiguration->max31855_cs[0] = Gpio::C13; // EGT1, header A28/A29
	engineConfiguration->max31855_cs[1] = Gpio::E5;  // EGT2, header A30/A31
	engineConfiguration->max31855_cs[2] = Gpio::E3;  // EGT3, chip not populated, test points
	engineConfiguration->max31855_cs[3] = Gpio::E6;  // EGT4, chip not populated, test points
}

static void setSdAndAccelerometer() {
	// SD card and LIS2DH12 share SPI1 (PB3 SCK, PB4 MISO, PB5 MOSI)
	enableHellenSpi1();
	engineConfiguration->sdCardCsPin = Gpio::E2;
	engineConfiguration->sdCardSpiDevice = SPI_DEVICE_1;
	engineConfiguration->isSdCardEnabled = true;
	engineConfiguration->accelerometerCsPin = Gpio::B9;
}

static void setupDefaultSensorInputs() {
	engineConfiguration->tps1_1AdcChannel = EFI_ADC_9;  // A14 TPS1 (PB1)
	engineConfiguration->tps1_2AdcChannel = EFI_ADC_11; // B15 TPS2 (PC1)
	engineConfiguration->clt.adcChannel = EFI_ADC_1;    // A17 CLT (PA1), 4.7k bias
	engineConfiguration->iat.adcChannel = EFI_ADC_14;   // A32 IAT (PC4), 4.7k bias
	engineConfiguration->map.sensor.hwChannel = EFI_ADC_3; // A19 external MAP (PA3)
	setPPSInputs(EFI_ADC_6, EFI_ADC_5);                 // B11 PPS1 (PA6), PPS2 (PA5) test point only

	// battery voltage 33k/6.8k on PC2, ignition key 33k/6.8k on PC0
	engineConfiguration->vbattAdcChannel = EFI_ADC_12;
	engineConfiguration->ignKeyAdcChannel = EFI_ADC_10;
	engineConfiguration->ignKeyAdcDivider = (33 + 6.8) / 6.8;

	// crank hall 1 on A13, cam hall 2 on B14; VR inputs PE0 (discrete) / PE1 (MAX9924) on test points as alternative
	engineConfiguration->triggerInputPins[0] = Gpio::E12; // A13 HALL1
	engineConfiguration->camInputs[0] = Gpio::E13;        // B14 HALL2

	// wheel speed: hall 3 (B10) as vehicle speed, wheel sensor 2 (B9, 4.7k pull-up) as aux speed 1 for slip ratio
	engineConfiguration->vehicleSpeedSensorInputPin = Gpio::E14;
	engineConfiguration->auxSpeedSensorInputPin[0] = Gpio::A4;
}

static void pyst_boardConfigOverrides() {
	// PWR_EN on PD2 switches the 5 V sensor supply, the ESP32 regulator and the output drivers
	setHellenEnPin(Gpio::D2);

	setHellenAnalogDividers(); // 2.0 analog divider, 5.835 vbatt divider, adcVcc 3.29
	engineConfiguration->vbattAdcChannel = EFI_ADC_12;

	setSdAndAccelerometer();

	engineConfiguration->vrThreshold[0].pin = Gpio::D14; // discrete VR input threshold PWM

	engineConfiguration->canTxPin = Gpio::D1;
	engineConfiguration->canRxPin = Gpio::D0;

	setDefaultHellenAtPullUps();
}

bool validateBoardConfig() {
	// no CAN2 transceiver on this board
	return true;
}

/**
 * @brief   Board-specific configuration defaults.
 *
 * See also setDefaultEngineConfiguration
 */
static void pyst_boardDefaultConfiguration() {
	setInjectorPins();
	setIgnitionPins();
	setGppwmPins();
	setEgtPins();

	// LPS22HB on I2C2 (PB10/PB11) is the barometric sensor
	setHellenMMbaro();

	engineConfiguration->displayLogicLevelsInEngineSniffer = true;
	engineConfiguration->isSdCardEnabled = true;

	engineConfiguration->enableSoftwareKnock = true;

	engineConfiguration->canTxPin = Gpio::D1;
	engineConfiguration->canRxPin = Gpio::D0;
	// rusEFI CAN broadcast for the dash bridge (listen-only transceiver) and RaceChrono
	engineConfiguration->enableVerboseCanTx = true;

	// weak low side outputs: main relay on B24 (PD11), fan on B28 (PB14); fuel pump on GPPWM4 (B8, PD15)
	engineConfiguration->mainRelayPin = Gpio::D11;
	engineConfiguration->fanPin = Gpio::B14;
	engineConfiguration->fuelPumpPin = Gpio::D15;

	// no ETB H-bridges on the ECU: the throttle lives on a stacked CAN module (J1201)
	engineConfiguration->etbFunctions[0] = DC_None;
	engineConfiguration->etbFunctions[1] = DC_None;

	setupDefaultSensorInputs();

	// UART2 -> ESP32-C3 dash bridge, 460800 baud (0.16 % error on the 42 MHz APB1)
	engineConfiguration->tunerStudioSerialSpeed = 460800;

	setCrankOperationMode();

	setAlgorithm(LM_ALPHA_N);
	engineConfiguration->alphaNUseIat = true;
	engineConfiguration->alphaNUseBaro = true;

	engineConfiguration->injectorCompensationMode = ICM_FixedRailPressure;

#ifndef EFI_BOOTLOADER
	setCommonNTCSensor(&engineConfiguration->clt, HELLEN_DEFAULT_AT_PULLUP);
	setCommonNTCSensor(&engineConfiguration->iat, HELLEN_DEFAULT_AT_PULLUP);
#endif // EFI_BOOTLOADER

	setTPS1Calibration(100, 650);

	// two on-board wideband controllers (hardware index 1 and 5)
	hellenWbo();
}

static Gpio OUTPUTS[] = {
	Gpio::E10, // A2 injector output 1
	Gpio::E8,  // A5 injector output 2
	Gpio::C8,  // B5 injector output 3
	Gpio::C6,  // B6 injector output 4
	Gpio::C12, // injector output 5 (test point)
	Gpio::B8,  // injector output 6 (test point)
	Gpio::E7,  // A3 GPPWM1 low side (has flyback)
	Gpio::E9,  // A4 GPPWM2 low side (has flyback)
	Gpio::A8,  // B7 GPPWM3 low side (has flyback)
	Gpio::D15, // B8 GPPWM4 low side (has flyback)
	Gpio::D11, // B24 weak low side 1 (relay, no flyback)
	Gpio::B14, // B28 weak low side 2 (relay, no flyback)
	// logic level coil outputs
	Gpio::B15, // A15 Coil 1
	Gpio::D13, // B13 Coil 2
	Gpio::D8,  // B22 Coil 3
	Gpio::D10, // B25 Coil 4
	Gpio::D12, // Coil 5 (test point)
	Gpio::B13, // Coil 6 (test point)
};

int getBoardMetaOutputsCount() {
	return efi::size(OUTPUTS);
}

int getBoardMetaLowSideOutputsCount() {
	return getBoardMetaOutputsCount() - 6;
}

Gpio* getBoardMetaOutputs() {
	return OUTPUTS;
}

int getBoardMetaDcOutputsCount() {
	return 0;
}

#if EFI_PROD_CODE
// PB2 (BOOT1 pull-down 10k) drives Q1301: high = ESP32-C3 held in reset
static OutputPin espResetPin;

static void pystEspReset() {
	if (!espResetPin.isInitialized()) {
		espResetPin.initPin("ESP_RST", Gpio::B2);
	}
	efiPrintf("PyST: resetting ESP32 dash bridge");
	espResetPin.setValue(1);
	chThdSleepMilliseconds(100);
	espResetPin.setValue(0);
}
#endif // EFI_PROD_CODE

// TS_BOARD_ACTION, see board_commands.ini (weak default in bench_test.cpp)
void boardTsAction(uint16_t index);
void boardTsAction(uint16_t index) {
	switch (index) {
	case 1:
#if EFI_PROD_CODE
		pystEspReset();
#endif // EFI_PROD_CODE
		break;
	default:
		efiPrintf("PyST: unknown board action %d", index);
		break;
	}
}

void setup_custom_board_overrides() {
	custom_board_DefaultConfiguration = pyst_boardDefaultConfiguration;
	custom_board_ConfigOverrides = pyst_boardConfigOverrides;
}
