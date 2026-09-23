/**
 * @file boards/hellen/uaefi-moto/board_configuration.cpp
 *
 * uaEFI derivative with two Superseal 1.0 34-pin headers, four EGT channels and two on-board widebands.
 * Pinout: connectors/A.yaml, connectors/B.yaml, connectors/EGT.yaml
 */

#include "pch.h"
#include "defaults.h"
#include "hellen_meta.h"
#include "hellen_leds_100.cpp"
#include "board_overrides.h"

static void setInjectorPins() {
	engineConfiguration->injectionPins[0] = Gpio::MM100_INJ1; // A7
	engineConfiguration->injectionPins[1] = Gpio::MM100_INJ2; // A8
	engineConfiguration->injectionPins[2] = Gpio::MM100_INJ3; // A9
	engineConfiguration->injectionPins[3] = Gpio::MM100_INJ4; // A10
	engineConfiguration->injectionPins[4] = Gpio::MM100_INJ5; // A11
	engineConfiguration->injectionPins[5] = Gpio::MM100_INJ6; // A12
}

static void setIgnitionPins() {
	engineConfiguration->ignitionPins[0] = Gpio::MM100_IGN1; // A13
	engineConfiguration->ignitionPins[1] = Gpio::MM100_IGN2; // A14
	engineConfiguration->ignitionPins[2] = Gpio::MM100_IGN3; // A15
	engineConfiguration->ignitionPins[3] = Gpio::MM100_IGN4; // A16
	engineConfiguration->ignitionPins[4] = Gpio::MM100_IGN5; // A17
	engineConfiguration->ignitionPins[5] = Gpio::MM100_IGN6; // A18
}

static void setGppwmPins() {
	// all four with flyback diode
	engineConfiguration->gppwm[0].pin = Gpio::MM100_INJ7; // A19 GPPWM1
	engineConfiguration->gppwm[1].pin = Gpio::MM100_INJ8; // A20 GPPWM2
	engineConfiguration->gppwm[2].pin = Gpio::MM100_OUT_PWM1; // A21 GPPWM3
	engineConfiguration->gppwm[3].pin = Gpio::MM100_OUT_PWM2; // A22 GPPWM4
}

static void setEgtPins() {
	// SPI3, MOSI not needed, we have one-way communication here
	engineConfiguration->is_enabled_spi_3 = true;
	engineConfiguration->spi3misoPin = Gpio::C11;
	engineConfiguration->spi3sckPin = Gpio::C10;
	engineConfiguration->max31855spiDevice = SPI_DEVICE_3;

	engineConfiguration->max31855_cs[0] = Gpio::A15; // EGT1, original uaEFI EGT footprint
	// no ETB H-bridges on this board, their control pins are used as chip selects
	engineConfiguration->max31855_cs[1] = Gpio::MM100_OUT_PWM3; // EGT2
	engineConfiguration->max31855_cs[2] = Gpio::MM100_OUT_PWM4; // EGT3
	engineConfiguration->max31855_cs[3] = Gpio::MM100_OUT_PWM5; // EGT4
}

static void setupDefaultSensorInputs() {
	engineConfiguration->tps1_1AdcChannel = MM100_IN_TPS_ANALOG; // B6
	engineConfiguration->tps1_2AdcChannel = MM100_IN_AUX1_ANALOG; // B7
	engineConfiguration->clt.adcChannel = MM100_IN_CLT_ANALOG; // B8
	engineConfiguration->iat.adcChannel = MM100_IN_IAT_ANALOG; // B9
	engineConfiguration->map.sensor.hwChannel = MM100_IN_MAP1_ANALOG; // B10

	// Hall sensors on B16/B17, VR inputs on B21..B24 are available as alternative
	engineConfiguration->triggerInputPins[0] = Gpio::MM100_IN_D1; // B16 HALL1
	engineConfiguration->camInputs[0] = Gpio::MM100_IN_D2; // B17 HALL2

	engineConfiguration->vehicleSpeedSensorInputPin = Gpio::MM100_IN_D3; // B18 HALL3/VSS
}

static void uaefi_moto_boardConfigOverrides() {
	setHellenMegaEnPin();
	setHellenVbatt();

	hellenMegaSdWithAccelerometer();

	engineConfiguration->vrThreshold[0].pin = Gpio::MM100_OUT_PWM6;

	setHellenCan();

	setDefaultHellenAtPullUps();
}

bool validateBoardConfig() {
	if (engineConfiguration->can2RxPin != Gpio::B12) {
		setHellenCan2();
	}
	return true;
}

/**
 * @brief   Board-specific configuration defaults.
 *
 * See also setDefaultEngineConfiguration
 */
static void uaefi_moto_boardDefaultConfiguration() {
	setInjectorPins();
	setIgnitionPins();
	setGppwmPins();
	setEgtPins();

	// on-board MAP sensor is used as baro
	setHellenMMbaro();

	engineConfiguration->displayLogicLevelsInEngineSniffer = true;
	engineConfiguration->isSdCardEnabled = true;

	engineConfiguration->enableSoftwareKnock = true;

	engineConfiguration->canTxPin = Gpio::MM100_CAN_TX;
	engineConfiguration->canRxPin = Gpio::MM100_CAN_RX;
	setHellenCan2();

	// ECU is powered directly from ignition, no main relay
	// A23 (IGN7) and A24 (IGN8) are general purpose weak low side outputs
	engineConfiguration->mainRelayPin = Gpio::Unassigned;
	engineConfiguration->fanPin = Gpio::Unassigned;
	engineConfiguration->fuelPumpPin = Gpio::Unassigned;

	// no ETB H-bridges on this board
	engineConfiguration->etbFunctions[0] = DC_None;
	engineConfiguration->etbFunctions[1] = DC_None;

	setupDefaultSensorInputs();
	engineConfiguration->enableVerboseCanTx = true;

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

	// two on-board wideband controllers
	hellenWbo();
}

static Gpio OUTPUTS[] = {
	Gpio::MM100_INJ1, // A7 injector output 1
	Gpio::MM100_INJ2, // A8 injector output 2
	Gpio::MM100_INJ3, // A9 injector output 3
	Gpio::MM100_INJ4, // A10 injector output 4
	Gpio::MM100_INJ5, // A11 injector output 5
	Gpio::MM100_INJ6, // A12 injector output 6
	Gpio::MM100_INJ7, // A19 GPPWM1 low side (has flyback)
	Gpio::MM100_INJ8, // A20 GPPWM2 low side (has flyback)
	Gpio::MM100_OUT_PWM1, // A21 GPPWM3 low side (has flyback)
	Gpio::MM100_OUT_PWM2, // A22 GPPWM4 low side (has flyback)
	Gpio::MM100_IGN7, // A23 weak low side 1 (relay, no flyback)
	Gpio::MM100_IGN8, // A24 weak low side 2 (relay, no flyback)
	// logic level coil outputs
	Gpio::MM100_IGN1, // A13 Coil 1
	Gpio::MM100_IGN2, // A14 Coil 2
	Gpio::MM100_IGN3, // A15 Coil 3
	Gpio::MM100_IGN4, // A16 Coil 4
	Gpio::MM100_IGN5, // A17 Coil 5
	Gpio::MM100_IGN6, // A18 Coil 6
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

void setup_custom_board_overrides() {
	custom_board_DefaultConfiguration = uaefi_moto_boardDefaultConfiguration;
	custom_board_ConfigOverrides = uaefi_moto_boardConfigOverrides;
}
