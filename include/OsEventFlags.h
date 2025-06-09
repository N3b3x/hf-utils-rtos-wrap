/**
  * Nebula Tech Corporation
  *
  * Copyright © 2023 Nebula Tech Corporation.   All Rights Reserved.
  * This file is part of HardFOC and is licensed under the GNU General Public License v3.0 or later.
  *
  */
#ifndef OS_EVENT_FLAGS_H_
#define OS_EVENT_FLAGS_H_

#include "OsAbstraction.h"

#include "ConsolePort.h"

#include "ThingsToString.h"
#include "Utility.h"
#include "OsUtility.h"

template <size_t groupSizeBytes>
class OsEventFlags {
public:
    /**
     * @brief Construct a new OsEventFlags object.
     *
     * The constructor does not initialize the OS event flags or the mutex.
     * The event flags and the mutex are initialized the first time an event is set or retrieved.
     */
    OsEventFlags(const char *groupName) :
        initialized(false),
        groupCreated(false),
        mutexCreated(false),
        name(groupName)
    {
        /// No code at this time.
    }

    /**
     * @brief Destroy the OsEventFlags object.
     *
     * If the event flags have been initialized, they are deleted.
     * If the mutex has been initialized, it is deleted.
     */
    ~OsEventFlags() {
        if (groupCreated) {
            os_event_flags_delete_ex(group);
        }
        if (mutexCreated) {
            os_mutex_delete_ex(mtx);
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
    bool Set(OS_Ulong flagsToSet) noexcept {
        if (EnsureInitialized()) {
            MutexGuard guard((OS_Mutex*)&mtx);
            return os_event_flags_set_ex(group, flagsToSet);
        }
        ConsolePort::WriteConditional(verbose, "OsEventFlags::Set() - [%s] Event flags not initialized.", name);
        return false;
    }

    /**
     * @brief Get event flags.
     *
     * If the event flags have not been initialized, they are initialized before the event is retrieved.
     *
     * @return The retrieved flags.
     */
    bool Get(OS_Ulong& flagsToGet, OS_Uint getOption, OS_Ulong wait_option = OS_NO_WAIT) noexcept {
        OS_Ulong actualFlags;
        if (EnsureInitialized()) {
            MutexGuard guard((OS_Mutex*)&mtx);
            return os_event_flags_get_ex(group, flagsToGet, getOption, actualFlags, wait_option);
        }
        ConsolePort::WriteConditional(verbose, "OsEventFlags::Get() - [%s] Event flags not initialized.", name);
        return false;
    }

private:
    bool Initialize() noexcept
    {
        if(!mutexCreated) {
            mutexCreated = os_mutex_create_ex(mtx, mutexName, OS_INHERIT);
        }

        if(!groupCreated) {
            groupCreated = os_event_flags_create_ex(group, name);
        }
        return mutexCreated && groupCreated;
    }

    bool initialized; ///< Whether the event flags have been initialized

    OS_EventGroup group; 	///< The OS event flags
    bool groupCreated;
    const char *name;

    OS_Mutex mtx;
    static const char mutexName[];
    bool mutexCreated;

    static constexpr bool verbose = true;

};

template <size_t groupSizeBytes>
const char OsEventFlags<groupSizeBytes>::mutexName[] = "OsEventFlags-Mutex";



#endif /* OS_EVENT_FLAGS_H_ */
