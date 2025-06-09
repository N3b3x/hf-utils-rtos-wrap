/**
  * Nebula Tech Corporation
  *
  * Copyright © 2023 Nebula Tech Corporation.   All Rights Reserved.
 * This file is part of HardFOC and is licensed under the GNU General Public License v3.0 or later.
  *

#ifndef UTILITIES_COMMON_THREADXEVENTFLAGS_H_
#define UTILITIES_COMMON_THREADXEVENTFLAGS_H_

#include "UTILITIES/common/RtosCompat.h"

#include "HAL/component_handlers/ConsolePort.h"

#include "UTILITIES/common/ThingsToString.h"
#include "UTILITIES/common/Utility.h"
#include "UTILITIES/common/TxUtility.h"

template <size_t groupSizeBytes>
class ThreadXEventFlags {
public:
    /**
     * @brief Construct a new ThreadXEventFlags object.
     *
     * The constructor does not initialize the ThreadX event flags or the mutex.
     * The event flags and the mutex are initialized the first time an event is set or retrieved.
     */
    ThreadXEventFlags(const char *groupName) :
        initialized(false),
        groupCreated(false),
        mutexCreated(false),
        name(groupName)
    {
        /// No code at this time.
    }

    /**
     * @brief Destroy the ThreadXEventFlags object.
     *
     * If the event flags have been initialized, they are deleted.
     * If the mutex has been initialized, it is deleted.
     */
    ~ThreadXEventFlags() {
        if (groupCreated) {
            DeleteTxEventFlags(group);
        }
        if (mutexCreated) {
            DeleteTxMutex(mtx);
        }
    }

    bool EnsureInitialized() noexcept
    {
        if (!initialized)
        {
            initialized = Initialize();
        }
        return initialized;
    }

    /**
     * @brief Set event flags.
     *
     * If the event flags have not been initialized, they are initialized before the event is set.
     *
     * @param flagsToSet The flags to set.
     */
    bool Set(ULONG flagsToSet) noexcept {
        if (EnsureInitialized()) {
            MutexGuard guard((TX_MUTEX*)&mtx);
            return SetTxEventFlags(group, flagsToSet);
        }
        ConsolePort::WriteConditional(verbose, "ThreadXEventFlags::Set() - [%s] Event flags not initialized.", name);
        return false;
    }

    /**
     * @brief Get event flags.
     *
     * If the event flags have not been initialized, they are initialized before the event is retrieved.
     *
     * @return The retrieved flags.
     */
    bool Get(ULONG& flagsToGet, UINT getOption, ULONG wait_option = TX_NO_WAIT) noexcept {
        ULONG actualFlags;
        if (EnsureInitialized()) {
            MutexGuard guard((TX_MUTEX*)&mtx);
            return GetTxEventFlags(group, flagsToGet, getOption, actualFlags, wait_option);
        }
        ConsolePort::WriteConditional(verbose, "ThreadXEventFlags::Get() - [%s] Event flags not initialized.", name);
        return false;
    }

private:
    bool Initialize() noexcept
    {
        if(!mutexCreated) {
            mutexCreated = CreateTxMutex(mtx, mutexName, TX_INHERIT);
        }

        if(!groupCreated) {
            groupCreated = CreateTxEventFlags(group, name);
        }
        return mutexCreated && groupCreated;
    }

    bool initialized; ///< Whether the event flags have been initialized

    TX_EVENT_FLAGS_GROUP group; 	///< The ThreadX event flags
    bool groupCreated;
    const char *name;

    TX_MUTEX mtx;
    static const char mutexName[];
    bool mutexCreated;

    static constexpr bool verbose = true;

};

template <size_t groupSizeBytes>
const char ThreadXEventFlags<groupSizeBytes>::mutexName[] = "ThreadXEventFlags-Mutex";



#endif /* UTILITIES_COMMON_THREADXEVENTFLAGS_H_ */
