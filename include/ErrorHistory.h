/**
 * @file ErrorHistory.h
 * @brief Fixed-capacity ring buffer of error / event records.
 *
 * Three types in one header:
 *   - `hf::ErrorHistoryReader<Record>`         — pure-virtual read-side ABC.
 *   - `hf::ErrorHistoryWriter<Record>`         — pure-virtual write-side ABC.
 *   - `hf::ErrorHistory<Record, kCapacity>`    — concrete impl deriving from both.
 *
 * @par Thread-safety
 *   All operations take an internal `RtosMutex`; safe from any task context.
 *   Not ISR-safe — callers from ISR should buffer to a queue and drain from
 *   a task. `Push` returns `false` if it cannot acquire the mutex.
 *
 * @par Allocation
 *   No heap allocation. Backing storage is an inline `Record[kCapacity]`.
 *   When the ring is full, `Push` overwrites the oldest entry and bumps
 *   `OverwriteCount()`.
 *
 * @par Constraints
 *   `Record` must be trivially copyable.
 */
#ifndef HF_UTILS_RTOS_WRAP_ERRORHISTORY_H_
#define HF_UTILS_RTOS_WRAP_ERRORHISTORY_H_

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <mutex>
#include <type_traits>

#include "RtosMutex.h"

namespace hf {

template <typename Record>
class ErrorHistoryReader {
public:
    virtual ~ErrorHistoryReader() noexcept = default;

    /// Number of entries currently held (≤ Capacity()).
    [[nodiscard]] virtual std::size_t Size() const noexcept = 0;

    /// Maximum number of entries the ring can hold.
    [[nodiscard]] virtual std::size_t Capacity() const noexcept = 0;

    /// Monotonic counter; bumps on every successful `Push` / `Pop` / `Clear`.
    [[nodiscard]] virtual uint32_t Seq() const noexcept = 0;

    /// Number of `Push` calls that overwrote an older entry.
    [[nodiscard]] virtual uint32_t OverwriteCount() const noexcept = 0;

    /**
     * @brief Copy up to @p max_out records into @p out, oldest first.
     * @return Number of records actually copied.
     */
    virtual std::size_t Snapshot(Record* out, std::size_t max_out) const noexcept = 0;
};

template <typename Record>
class ErrorHistoryWriter {
public:
    virtual ~ErrorHistoryWriter() noexcept = default;

    /// Append @p record. Overwrites oldest entry if full.
    virtual bool Push(const Record& record) noexcept = 0;

    /// Pop oldest record into @p out; returns `false` if empty.
    virtual bool Pop(Record& out) noexcept = 0;

    /// Drop all records.
    virtual bool Clear() noexcept = 0;
};

template <typename Record, std::size_t kCapacity>
class ErrorHistory final
    : public ErrorHistoryReader<Record>
    , public ErrorHistoryWriter<Record>
{
    static_assert(kCapacity > 0, "ErrorHistory kCapacity must be > 0");
    static_assert(std::is_trivially_copyable_v<Record>,
                  "ErrorHistory<Record, N> requires a trivially copyable Record");

public:
    ErrorHistory() noexcept = default;

    ErrorHistory(const ErrorHistory&)            = delete;
    ErrorHistory& operator=(const ErrorHistory&) = delete;

    /* ── Writer ──────────────────────────────────────────────────── */

    bool Push(const Record& record) noexcept override
    {
        std::lock_guard<RtosMutex> guard(mutex_);
        std::memcpy(&records_[head_], &record, sizeof(Record));
        head_ = (head_ + 1U) % kCapacity;
        if (size_ < kCapacity) {
            ++size_;
        } else {
            tail_ = (tail_ + 1U) % kCapacity;
            ++overwrite_count_;
        }
        ++seq_;
        return true;
    }

    bool Pop(Record& out) noexcept override
    {
        std::lock_guard<RtosMutex> guard(mutex_);
        if (size_ == 0U) return false;
        std::memcpy(&out, &records_[tail_], sizeof(Record));
        tail_ = (tail_ + 1U) % kCapacity;
        --size_;
        ++seq_;
        return true;
    }

    bool Clear() noexcept override
    {
        std::lock_guard<RtosMutex> guard(mutex_);
        head_ = tail_ = size_ = 0U;
        ++seq_;
        return true;
    }

    /* ── Reader ──────────────────────────────────────────────────── */

    [[nodiscard]] std::size_t Size() const noexcept override
    {
        std::lock_guard<RtosMutex> guard(mutex_);
        return size_;
    }

    [[nodiscard]] std::size_t Capacity() const noexcept override
    {
        return kCapacity;
    }

    [[nodiscard]] uint32_t Seq() const noexcept override
    {
        std::lock_guard<RtosMutex> guard(mutex_);
        return seq_;
    }

    [[nodiscard]] uint32_t OverwriteCount() const noexcept override
    {
        std::lock_guard<RtosMutex> guard(mutex_);
        return overwrite_count_;
    }

    std::size_t Snapshot(Record* out, std::size_t max_out) const noexcept override
    {
        if (out == nullptr || max_out == 0U) return 0U;
        std::lock_guard<RtosMutex> guard(mutex_);
        const std::size_t n = (size_ < max_out) ? size_ : max_out;
        std::size_t       idx = tail_;
        for (std::size_t i = 0; i < n; ++i) {
            std::memcpy(&out[i], &records_[idx], sizeof(Record));
            idx = (idx + 1U) % kCapacity;
        }
        return n;
    }

private:
    mutable RtosMutex mutex_{};
    Record            records_[kCapacity]{};
    std::size_t       head_{0};
    std::size_t       tail_{0};
    std::size_t       size_{0};
    uint32_t          seq_{0};
    uint32_t          overwrite_count_{0};
};

}  // namespace hf

#endif /* HF_UTILS_RTOS_WRAP_ERRORHISTORY_H_ */
