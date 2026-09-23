/**
 * @file engine_hours_storage.h
 * @brief Engine hour counter record, shared between backup registers and flash copy
 */

#pragma once

#include <cstdint>

#define ENGINE_HOURS_COUNT 5

struct engine_hours_s {
	uint32_t seconds[ENGINE_HOURS_COUNT];
	uint32_t crc;

	uint32_t getCrc() const;
	bool isValid() const;
	void updateCrc();
};
