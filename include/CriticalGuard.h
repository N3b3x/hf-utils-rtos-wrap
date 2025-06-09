#pragma once
/**
 * @file CriticalGuard.h
 * @brief RAII helper for entering and exiting a critical section.
 */

#include "OsAbstraction.h"

/**
 * @class CriticalGuard
 * @brief Automatically enters a critical section on construction and
 *        exits it on destruction.
 */
class CriticalGuard {
public:
    CriticalGuard() noexcept  { os_critical_enter(); }
    ~CriticalGuard()          { os_critical_exit();  }
    CriticalGuard(const CriticalGuard&) = delete;
    CriticalGuard& operator=(const CriticalGuard&) = delete;
};

