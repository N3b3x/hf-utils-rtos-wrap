/**
 * @file OsQueue.h
 * @brief Lightweight C++ wrapper for the RTOS queue primitive.
 *
 * Thread-safety: internally thread-safe; the underlying FreeRTOS queue is
 * already MT-safe so no extra mutex is layered on top.
 *
 * Allocation: queue storage is held inline as a fixed-capacity member array;
 * no heap allocation. The underlying RTOS queue handle is created eagerly in
 * the constructor.
 *
 * Public surface uses modern C++ types — `uint32_t timeout_ms`, `bool`. No
 * `OS_Ulong` / `OS_WAIT_FOREVER` leakage. Pass `UINT32_MAX` to wait forever.
 */
#ifndef OS_QUEUE_H_
#define OS_QUEUE_H_

#include <cstdint>
#include <cstddef>
#include <climits>
#include "OsAbstraction.h"
#include "OsUtility.h"

/**
 * @class OsQueue
 * @brief Fixed-capacity, eager-init MPMC queue of trivially-copyable items.
 *
 * @tparam MessageType  Element type (must be trivially copyable; passed by
 *                      value through the underlying queue).
 * @tparam kCapacity    Maximum number of pending elements.
 */
template <typename MessageType, size_t kCapacity>
class OsQueue {
public:
    /**
     * @brief Construct and create the underlying RTOS queue eagerly.
     */
    explicit OsQueue(const char* queueName) noexcept
        : name_(queueName)
    {
        constexpr OS_Uint kItemWords =
            static_cast<OS_Uint>((sizeof(MessageType) + sizeof(uint32_t) - 1U) / sizeof(uint32_t));
        created_ = os_queue_create_ex(queue_, name_, kItemWords,
                                      storage_, sizeof(storage_));
    }

    OsQueue(const OsQueue&) = delete;
    OsQueue& operator=(const OsQueue&) = delete;

    ~OsQueue() noexcept
    {
        if (created_) {
            os_queue_delete_ex(queue_);
        }
    }

    /// True if the underlying RTOS queue was created successfully.
    [[nodiscard]] bool IsValid() const noexcept { return created_; }

    /**
     * @brief Send a message; blocks up to @p timeout_ms.
     * @return true on success, false on timeout / failure.
     */
    bool Send(const MessageType& message, uint32_t timeout_ms = UINT32_MAX) noexcept
    {
        if (!created_) return false;
        MessageType local = message;
        return os_queue_send_ex(queue_, &local, ToTicks(timeout_ms));
    }

    /**
     * @brief Receive a message; blocks up to @p timeout_ms.
     * @return true on success, false on timeout / failure.
     */
    bool Receive(MessageType& out, uint32_t timeout_ms = UINT32_MAX) noexcept
    {
        if (!created_) return false;
        return os_queue_receive_ex(queue_, &out, ToTicks(timeout_ms));
    }

    /**
     * @brief Compile-time capacity in elements.
     */
    [[nodiscard]] static constexpr size_t Capacity() noexcept { return kCapacity; }

private:
    static OS_Ulong ToTicks(uint32_t timeout_ms) noexcept
    {
        return (timeout_ms == UINT32_MAX)
            ? static_cast<OS_Ulong>(OS_WAIT_FOREVER)
            : static_cast<OS_Ulong>(os_convert_msec_to_delay_ticks(timeout_ms));
    }

    const char* name_;
    bool        created_{false};
    OS_Queue    queue_{};
    MessageType storage_[kCapacity];
};

#endif /* OS_QUEUE_H_ */

