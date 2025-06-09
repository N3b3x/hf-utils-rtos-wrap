#pragma once
/**
 * @file PeriodicTimer.h
 * @brief C++ wrapper for FreeRTOS timers.
 */

#include "OsUtility.h"

/**
 * @class PeriodicTimer
 * @brief Simple RAII wrapper around OS timers.
 */
class PeriodicTimer {
public:
    /** Construct an empty timer. */
    PeriodicTimer() noexcept : timer{}, created(false) {}

    /** Delete timer on destruction. */
    ~PeriodicTimer() { if (created) os_timer_deactivate_and_delete_ex(timer); }

    /**
     * @brief Create a periodic timer.
     *
     * @param name      Timer name used for debugging.
     * @param callback  Function called on each expiration.
     * @param arg       Value passed to the callback.
     * @param periodMs  Period in milliseconds.
     * @param autoStart Whether to start immediately after creation.
     * @return true on success.
     */
    bool Create(const char* name, void (*callback)(uint32_t), uint32_t arg,
                uint32_t periodMs, bool autoStart = false) noexcept {
        if (created) return false;
        uint32_t ticks = os_convert_msec_to_delay_ticks(periodMs);
        created = os_timer_create_ex(timer, name, callback, arg, ticks, ticks,
                                autoStart ? OS_AUTO_START : OS_DONT_START);
        return created;
    }

    /** Start the timer. */
    bool Start() noexcept { return os_timer_activate_ex(timer); }

    /** Stop the timer. */
    bool Stop() noexcept { return os_timer_deactivate_ex(timer); }

    /** Delete the timer. */
    bool Destroy() noexcept {
        bool res = os_timer_deactivate_and_delete_ex(timer);
        if (res) created = false;
        return res;
    }

    /** Check if the timer was successfully created. */
    bool IsValid() const noexcept { return created; }

private:
    OS_Timer timer;
    bool created;
};

