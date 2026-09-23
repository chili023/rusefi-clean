/**
 * @file	backup_ram.cpp
 *
 * @date Dec 19, 2017
 */

#include "backup_ram.h"

uint32_t backupRamLoad(backup_ram_e idx) {
#if HAL_USE_RTC
	switch (idx) {
	case backup_ram_e::StepperPosition:
		return RTCD1.rtc->BKP0R & 0xffff;
	case backup_ram_e::IgnCounter:
		return (RTCD1.rtc->BKP0R >> 16) & 0xff;
	case backup_ram_e::EngineHours0:
	case backup_ram_e::EngineHours1:
	case backup_ram_e::EngineHours2:
	case backup_ram_e::EngineHours3:
	case backup_ram_e::EngineHours4:
	case backup_ram_e::EngineHoursCrc:
		// BKPxR registers are consecutive, engine hours use BKP1R..BKP6R
		return (&RTCD1.rtc->BKP0R)[1 + (int)idx - (int)backup_ram_e::EngineHours0];
	default:
		criticalError("Invalid backup ram idx %d", (int)idx);
		return 0;
	}
#else
	return 0;
#endif /* HAL_USE_RTC */
}

void backupRamSave(backup_ram_e idx, uint32_t value) {
#if HAL_USE_RTC
	switch (idx) {
	case backup_ram_e::StepperPosition:
		RTCD1.rtc->BKP0R = (RTCD1.rtc->BKP0R & ~0x0000ffff) | (value & 0xffff);
		break;
	case backup_ram_e::IgnCounter:
		RTCD1.rtc->BKP0R = (RTCD1.rtc->BKP0R & ~0x00ff0000) | ((value & 0xff) << 16);
		break;
	case backup_ram_e::EngineHours0:
	case backup_ram_e::EngineHours1:
	case backup_ram_e::EngineHours2:
	case backup_ram_e::EngineHours3:
	case backup_ram_e::EngineHours4:
	case backup_ram_e::EngineHoursCrc:
		(&RTCD1.rtc->BKP0R)[1 + (int)idx - (int)backup_ram_e::EngineHours0] = value;
		break;
	default:
		criticalError("Invalid backup ram idx %d, value %lx", (int)idx, value);
		break;
	}
#endif /* HAL_USE_RTC */
}

#if !defined(AT32F4XX)

void backupRamFlush(void) {
	// nothing to do here, in STM32 all data is saved instantaneously
}

// STM32 only has 4k bytes of backup SRAM
static_assert(sizeof(BackupSramData) <= 4096);

static BKUP_RAM_NOINIT BackupSramData backupSramData;

BackupSramData* getBackupSram() {
	return &backupSramData;
}

#endif
