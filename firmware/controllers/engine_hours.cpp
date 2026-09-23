/**
 * @file engine_hours.cpp
 * @brief Resettable engine hour counters which survive power cycles
 *
 * See engine_hours.h
 */

#include "pch.h"

#include "engine_hours.h"
#include "rusefi/crc.h"

#if EFI_PROD_CODE && HAL_USE_RTC && (defined(STM32F4XX) || defined(STM32F7XX) || defined(STM32H7XX))
#include "backup_ram.h"
#define ENGINE_HOURS_BACKUP_REGISTERS TRUE
#else
#define ENGINE_HOURS_BACKUP_REGISTERS FALSE
#endif

#if EFI_CONFIGURATION_STORAGE
#include "flash_main.h"
#endif

// Save flash copy once any counter moved this far away from the last saved value.
// Flash copy is only a fallback for lost backup battery, no need to wear flash more often.
#define FLASH_SAVE_THRESHOLD_S (30 * 60)

// Protect against huge time steps, for example if slow callback was blocked
#define MAX_TIME_STEP_S 5.0f

#define ENGINE_HOURS_CRC_MAGIC 0x454E4748 // 'ENGH'

uint32_t engine_hours_s::getCrc() const {
	return crc32(seconds, sizeof(seconds)) ^ ENGINE_HOURS_CRC_MAGIC;
}

bool engine_hours_s::isValid() const {
	return crc == getCrc();
}

void engine_hours_s::updateCrc() {
	crc = getCrc();
}

#if EFI_UNIT_TEST
// unit tests have no persistentState container
static engine_hours_s flashCopySimulation{};
engine_hours_s& engineHoursFlashCopy() {
	return flashCopySimulation;
}
#else
engine_hours_s& engineHoursFlashCopy() {
	return persistentState.engineHours;
}
#endif

#if ENGINE_HOURS_BACKUP_REGISTERS

static bool loadFromBackup(engine_hours_s& out) {
	for (size_t i = 0; i < ENGINE_HOURS_COUNT; i++) {
		out.seconds[i] = backupRamLoad((backup_ram_e)((int)backup_ram_e::EngineHours0 + i));
	}
	out.crc = backupRamLoad(backup_ram_e::EngineHoursCrc);
	return out.isValid();
}

static void saveToBackup(const engine_hours_s& hours) {
	for (size_t i = 0; i < ENGINE_HOURS_COUNT; i++) {
		backupRamSave((backup_ram_e)((int)backup_ram_e::EngineHours0 + i), hours.seconds[i]);
	}
	backupRamSave(backup_ram_e::EngineHoursCrc, hours.crc);
}

#else // ENGINE_HOURS_BACKUP_REGISTERS

// Simulator/unit tests: static RAM survives our simulated power cycles
static engine_hours_s backupSimulation{};

static bool loadFromBackup(engine_hours_s& out) {
	out = backupSimulation;
	return out.isValid();
}

static void saveToBackup(const engine_hours_s& hours) {
	backupSimulation = hours;
}

#if EFI_UNIT_TEST
void engineHoursLoseBackupForTest() {
	backupSimulation = {};
}
#endif

#endif // ENGINE_HOURS_BACKUP_REGISTERS

void EngineHours::init() {
	engine_hours_s fromBackup;
	const engine_hours_s& fromFlash = engineHoursFlashCopy();

	if (loadFromBackup(fromBackup)) {
		m_hours = fromBackup;
	} else if (fromFlash.isValid()) {
		efiPrintf("Engine hours: backup registers lost, restoring from flash copy");
		m_hours = fromFlash;
	} else {
		efiPrintf("Engine hours: no valid data, starting from zero");
		m_hours = {};
	}
	m_hours.updateCrc();

	if (fromFlash.isValid()) {
		m_lastFlashSave = fromFlash;
	} else {
		m_lastFlashSave = {};
	}

	storeToBackup();
	m_timer.reset();
	m_fractionalSeconds = 0;
	m_isInitialized = true;
}

void EngineHours::storeToBackup() {
	saveToBackup(m_hours);
	// keep RAM copy current so any configuration burn also stores recent values
	engineHoursFlashCopy() = m_hours;
}

void EngineHours::updateFlashCopy(bool forceSave) {
	bool needSave = forceSave;

	for (size_t i = 0; i < ENGINE_HOURS_COUNT; i++) {
		uint32_t current = m_hours.seconds[i];
		uint32_t saved = m_lastFlashSave.seconds[i];
		// counter went down means it was reset, persist immediately
		if (current < saved || current - saved >= FLASH_SAVE_THRESHOLD_S) {
			needSave = true;
		}
	}

	if (!needSave) {
		return;
	}

	m_lastFlashSave = m_hours;
	engineHoursFlashCopy() = m_hours;
	flashSaveCounter++;
#if EFI_CONFIGURATION_STORAGE
	// actual write is postponed by storage manager until engine is stopped
	setNeedToWriteConfiguration();
#endif
}

void EngineHours::onSlowCallback() {
	if (!m_isInitialized) {
		init();
	}

	float dt = m_timer.getElapsedSecondsAndReset(getTimeNowNt());
	bool isStopped = engine->rpmCalculator.isStopped();

	if (!isStopped) {
		m_fractionalSeconds += std::min(dt, MAX_TIME_STEP_S);

		if (m_fractionalSeconds >= 1) {
			uint32_t wholeSeconds = (uint32_t)m_fractionalSeconds;
			m_fractionalSeconds -= wholeSeconds;

			for (size_t i = 0; i < ENGINE_HOURS_COUNT; i++) {
				m_hours.seconds[i] += wholeSeconds;
			}
			m_hours.updateCrc();
			storeToBackup();
		}
	} else {
		updateFlashCopy(false);
	}

#if EFI_TUNER_STUDIO
	engine->outputChannels.hours_front_tire = getHours((size_t)engine_hours_counter_e::FrontTire);
	engine->outputChannels.hours_back_tire = getHours((size_t)engine_hours_counter_e::RearTire);
	engine->outputChannels.hours_cylinder = getHours((size_t)engine_hours_counter_e::Cylinder);
	engine->outputChannels.hours_piston = getHours((size_t)engine_hours_counter_e::Piston);
	engine->outputChannels.hours_engine = getHours((size_t)engine_hours_counter_e::Engine);
#endif // EFI_TUNER_STUDIO
}

void EngineHours::reset(size_t index) {
	if (index >= ENGINE_HOURS_COUNT) {
		efiPrintf("Engine hours: invalid counter %d", (int)index);
		return;
	}
	if (!engine->rpmCalculator.isStopped()) {
		efiPrintf("Engine hours: reset refused while engine is running");
		return;
	}
	if (!m_isInitialized) {
		init();
	}

	efiPrintf("Engine hours: reset counter %d (was %lu s)", (int)index, (unsigned long)m_hours.seconds[index]);
	m_hours.seconds[index] = 0;
	m_hours.updateCrc();
	storeToBackup();
	updateFlashCopy(true);
}

void EngineHours::requestFlashSave() {
	if (!m_isInitialized) {
		init();
	}
	updateFlashCopy(true);
}

uint32_t EngineHours::getSeconds(size_t index) const {
	return index < ENGINE_HOURS_COUNT ? m_hours.seconds[index] : 0;
}

float EngineHours::getHours(size_t index) const {
	return getSeconds(index) / 3600.0f;
}

void handleEngineHoursCommand(uint16_t index) {
	if (index == ENGINE_HOURS_CMD_SAVE) {
		engine->module<EngineHours>()->requestFlashSave();
	} else {
		engine->module<EngineHours>()->reset(index);
	}
}

static void printEngineHours() {
	static const char *names[ENGINE_HOURS_COUNT] = { "front tire", "rear tire", "cylinder", "piston", "engine" };
	for (size_t i = 0; i < ENGINE_HOURS_COUNT; i++) {
		uint32_t s = engine->module<EngineHours>()->getSeconds(i);
		efiPrintf("Engine hours %d %s: %lu s = %.2f h", (int)i, names[i], (unsigned long)s, s / 3600.0f);
	}
}

static void resetEngineHours(int index) {
	engine->module<EngineHours>()->reset(index);
}

void initEngineHours() {
	addConsoleAction("enginehours", printEngineHours);
	addConsoleActionI("enginehoursreset", resetEngineHours);
}
