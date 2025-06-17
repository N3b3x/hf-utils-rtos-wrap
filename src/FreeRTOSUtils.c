/**
 * @file FreeRTOSUtils.c
 * @brief Implementation of FreeRTOS utility functions.
 *
 * Nebula Tech Corporation
 *
 * Copyright © 2023 Nebula Tech Corporation. All Rights Reserved.
 * This file is part of HardFOC and is licensed under the GNU General Public License v3.0 or later.
 */

#include "FreeRTOSUtils.h"

const char* freertos_ret_to_string(BaseType_t result) {
    switch (result) {
        case pdPASS:
            return "pdPASS (Success)";
        case pdFAIL:
            return "pdFAIL (Generic failure)";
        case errCOULD_NOT_ALLOCATE_REQUIRED_MEMORY:
            return "errCOULD_NOT_ALLOCATE_REQUIRED_MEMORY";
        case errQUEUE_BLOCKED:
            return "errQUEUE_BLOCKED";
        case errQUEUE_YIELD:
            return "errQUEUE_YIELD";
        default:
            // Handle cases where multiple constants have same value
            if (result == errQUEUE_EMPTY) {
                return "errQUEUE_EMPTY";
            } else if (result == errQUEUE_FULL) {
                return "errQUEUE_FULL";
            }
            return "Unknown FreeRTOS error";
    }
}

const char* freertos_task_state_to_string(eTaskState state) {
    switch (state) {
        case eReady:
            return "Ready";
        case eRunning:
            return "Running";
        case eBlocked:
            return "Blocked";
        case eSuspended:
            return "Suspended";
        case eDeleted:
            return "Deleted";
        case eInvalid:
            return "Invalid";
        default:
            return "Unknown state";
    }
}
