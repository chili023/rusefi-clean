/*
 * @file persistent_configuration.h
 *
 * @date Feb 27, 2020
 * @author Andrey Belomutskiy, (c) 2012-2020
 */

#pragma once

#include "generated_lookup_engine_configuration.h"

#include "rusefi/crc.h"

struct persistent_config_container_s {
        int version{};
        int size{};
        persistent_config_s persistentConfiguration{};

        float frontTireHours{};
        float rearTireHours{};
        float cylinderHours{};
        float pistonHours{};
        float engineHours{};

        bool resetFrontTireHours{};
        bool resetRearTireHours{};
        bool resetCylinderHours{};
        bool resetPistonHours{};
        bool resetEngineHours{};

        uint32_t crc{};

        uint32_t getCrc() {
                // calculate CRC over all persistent data except metadata fields
                return crc32(&persistentConfiguration,
                        (uint32_t)((uint8_t*)&crc - (uint8_t*)&persistentConfiguration));
        }
};
