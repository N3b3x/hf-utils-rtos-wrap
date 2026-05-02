/**
 * @file OsEventFlags.h
 * @brief Lightweight C++ wrapper for the RTOS event-flag-group primitive.
 *
 * Thread-safety: internally thread-safe; the underlying FreeRTOS event group
 * is already MT-safe, so no extra mutex is layered on top.
 *
 * Allocation: the underlying RTOS handle is created eagerly in the
 * constructor; no heap allocation in `Set` / `Clear` / `Wait`.
 *
 * Public surface uses modern C++ types (`uint32_t`, `enum class WaitMode`) —
 * no `OS_Ulong`, `OS_OR`, `OS_AND`, `OS_WAIT_FOREVER` leakage. Pass
 * `UINT32_MAX` to wait forever.
 */
#ifndef OS_EVENT_FLAGS_H_
#define OS_EVENT_FLAGS_H_

#include <cstdint>
#include <climits>
#include "OsAbstraction.h"
#include "OsUtility.h"

/**
 * @enum WaitMode
 * @brief Selects ANY-of-bits vs ALL-of-bits semantics for `Wait` calls.
 */
enum class WaitMode : uint8_t {
    Any,  ///< Wake when any of the requested bits are set.
    All,  ///< Wake only when all of the requested bits are set.
};

/**
 * @class OsEventFlags
 * @brief Eager-init event-flag group with modern C++ public surface.
 *
 * @tparam kReserved  Unused; kept for source compatibility. Pass anything.
 */
template <size_t kReserved = 1>
class OsEventFlags {
public:
    explicit OsEventFlags(const char* groupName) noexcept
        : name_(groupName)
    {
        created_ = os_event_flags_create_ex(group_, name_);
    }

    OsEventFlags(const OsEventFlags&) = delete;
    OsEventFlags& operator=(const OsEventFlags&) = delete;

    ~OsEventFlags() noexcept
    {
        if (created_) {
            os_event_flags_delete_ex(group_);
        }
    }

    /// True if the underlying RTOS handle was created successfully.
    [[nodiscard]] bool IsValid() const noexcept { return created_; }

    /**
     * @brief Set @p bits in the group.
     */
    bool Set(uint32_t bits) noexcept
    {
        if (!created_) return false;
        return os_event_flags_set_ex(group_, static_cast<OS_Ulong>(bits));
    }

    /**
     * @brief Clear @p bits in the group.
     */
    bool Clear(uint32_t bits) noexcept
    {
        if (!created_) return false;
        return os_event_flags_clear_ex(group_, static_cast<OS_Ulong>(bits));
    }

    /**
     * @brief Wait for @p bits to be set per @p mode, up to @p timeout_ms.
     *
     * Bits are NOT cleared on return.
     *
     * @param[in]  bits        Bit mask to wait for.
     * @param[in]  mode        ANY or ALL semantics.
     * @param[in]  timeout_ms  Max wait in ms; pass `UINT32_MAX` to wait forever.
     * @param[out] actual_bits Bits seen at wake-time (for ANY mode caller can
     *                         tell which fired).
     * @return true on success (bits satisfied), false on timeout.
     */
    bool Wait(uint32_t bits, WaitMode mode, uint32_t timeout_ms,
              uint32_t& actual_bits) noexcept
    {
        actual_bits = 0;
        if (!created_) return false;
        OS_Ulong actual = 0;
        const OS_Uint option = (mode == WaitMode::All)
                                   ? static_cast<OS_Uint>(OS_AND)
                                   : static_cast<OS_Uint>(OS_OR);
        const bool ok = os_event_flags_get_ex(group_, static_cast<OS_Ulong>(bits),
                                              option, actual, ToTicks(timeout_ms));
        actual_bits = static_cast<uint32_t>(actual);
        return ok;
    }

private:
    static OS_Ulong ToTicks(uint32_t timeout_ms) noexcept
    {
        return (timeout_ms == UINT32_MAX)
            ? static_cast<OS_Ulong>(OS_WAIT_FOREVER)
            : static_cast<OS_Ulong>(os_convert_msec_to_delay_ticks(timeout_ms));
    }

    const char*   name_;
    bool          created_{false};
    OS_EventGroup group_{};
};

#endif /* OS_EVENT_FLAGS_H_ */

