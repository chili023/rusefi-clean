/**
 * @file engine_hours.h
 * @brief Resettable engine hour counters which survive power cycles
 *
 * Counters run while the engine is turning. Primary storage are the battery backed
 * RTC backup registers (written every second, no flash wear, no CPU freeze), secondary
 * storage is a copy inside persistentState which is saved to internal flash while the
 * engine is stopped. The flash copy is only used if the backup registers are lost
 * (for example backup battery empty or replaced).
 */

#pragma once

#include "engine_module.h"
#include "engine_hours_storage.h"
#include <rusefi/timer.h>

enum class engine_hours_counter_e : uint8_t {
	FrontTire = 0,
	RearTire = 1,
	Cylinder = 2,
	Piston = 3,
	Engine = 4,
};

// TS command index to request immediate save of the counters to flash
#define ENGINE_HOURS_CMD_SAVE 0xFF

class EngineHours : public EngineModule {
public:
	void onSlowCallback() override;

	// Zero one counter. Only allowed while engine is stopped.
	void reset(size_t index);
	// Schedule a write of the flash copy (engine stopped only)
	void requestFlashSave();

	uint32_t getSeconds(size_t index) const;
	float getHours(size_t index) const;

	// number of times a flash save was requested, for diagnostics and tests
	uint32_t flashSaveCounter = 0;

#if EFI_UNIT_TEST
	// simulate a power cycle
	void resetForTest() {
		m_isInitialized = false;
	}
#endif

private:
	void init();
	void storeToBackup();
	void updateFlashCopy(bool forceSave);

	bool m_isInitialized = false;
	engine_hours_s m_hours{};
	// what was last sent to flash
	engine_hours_s m_lastFlashSave{};

	Timer m_timer;
	float m_fractionalSeconds = 0;
};

void initEngineHours();
// TS_ENGINE_HOURS command: index 0..4 resets that counter, ENGINE_HOURS_CMD_SAVE forces flash save
void handleEngineHoursCommand(uint16_t index);
#if EFI_UNIT_TEST
void engineHoursLoseBackupForTest();
#endif
