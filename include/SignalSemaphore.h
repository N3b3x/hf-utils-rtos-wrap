/**
 * @file SignalSemaphore.h
 * @brief SignalSemaphore class definition.
 *
 * Nebula Tech Corporation
 *
 * Copyright © 2023 Nebula Tech Corporation. All Rights Reserved.
 * This file is part of HardFOC and is licensed under the GNU General Public License v3.0 or later.
 *
 * The SignalSemaphore class provides a wrapper for a named semaphore that is dynamically
 * created. When the SignalSemaphore object goes out of scope, the semaphore is deleted.
 */

#ifndef SIGNALSEMAPHORE_H
#define SIGNALSEMAPHORE_H

#include <stdint.h>
#include "UTILITIES/common/TxUtility.h"
#include <cstdio>

/**
 * @brief SignalSemaphore class for managing a named semaphore.
 */
class SignalSemaphore
{
public:
    /**
     * @brief Constructor for SignalSemaphore.
     * @param baseName Base name of the semaphore.
     * @param nameExtension Optional name extension for uniqueness.
     */
    SignalSemaphore(const char* baseName, const char* nameExtension = nullptr) noexcept;

    /**
     * @brief Deleted copy constructor to avoid copying instances.
     */
    SignalSemaphore(const SignalSemaphore& copy) = delete;

    /**
     * @brief Deleted assignment operator to avoid copying instances.
     */
    SignalSemaphore& operator=(const SignalSemaphore& copy) = delete;

    /**
     * @brief Destructor for SignalSemaphore.
     */
    ~SignalSemaphore();

    /**
     * @brief Waits until the semaphore is signaled.
     * @param msecToWait Maximum time to wait in milliseconds.
     * @return True if the semaphore was successfully acquired, false otherwise.
     */
    bool WaitUntilSignalled(uint32_t msecToWait = TX_WAIT_FOREVER) noexcept;

    /**
     * @brief Signals the semaphore.
     * @return True if the semaphore was successfully signaled, false otherwise.
     */
    bool Signal() noexcept;

    /**
     * @brief Checks if the semaphore is signaled.
     * @return True if the semaphore is signaled, false otherwise.
     */
    bool IsSignalled() noexcept;

    /**
     * @brief Ensures the semaphore is initialized.
     * @return True if the semaphore is initialized, false otherwise.
     */
    bool EnsureInitialized() noexcept;

    /**
     * @brief Checks if the semaphore is initialized.
     * @return True if the semaphore is initialized, false otherwise.
     */
    bool IsInitialized() const noexcept;

    /**
     * @brief Gets the name of the semaphore.
     * @return The name of the semaphore.
     */
    const char* GetName() const noexcept;

private:

    static constexpr uint32_t MaxNameLength = 39U;  ///< Maximum length of the semaphore name (excluding null terminator).

    TX_SEMAPHORE semaphore;  ///< The semaphore object.
    char name[MaxNameLength + 1];  ///< The name of the semaphore.
    bool initialized;  ///< Indicates whether the semaphore is initialized.
};

#endif /* SIGNALSEMAPHORE_H */
