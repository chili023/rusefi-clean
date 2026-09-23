/*
 * @file persistent_configuration.h
 *
 * @date Feb 27, 2020
 * @author Andrey Belomutskiy, (c) 2012-2020
 */

#pragma once

#include "generated_lookup_engine_configuration.h"

#include "rusefi/crc.h"
#include "engine_hours_storage.h"

struct persistent_config_container_s {
	int version{};
	int size{};
	persistent_config_s persistentConfiguration{};
	uint32_t crc{};

	// not part of the TunerStudio visible configuration so that a burn from TS never overwrites counters
	// has own CRC, see engine_hours.cpp
	engine_hours_s engineHours{};

	uint32_t getCrc() {
		return crc32(&persistentConfiguration, sizeof(persistent_config_s));
	}
};
