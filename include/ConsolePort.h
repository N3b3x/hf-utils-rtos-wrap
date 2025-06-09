#pragma once
/**
 * @file ConsolePort.h
 * @brief Logging facade that wraps the ESP‑IDF log library.
 *
 * This header provides a small convenience class used throughout the
 * component to output formatted log statements.  All methods are header
 * only and simply forward to the equivalent ESP log functions so the
 * wrapper can be compiled as a lightweight inline utility.
 */

#include "esp_log.h"
#include <cstdarg>

/**
 * @brief Provides lightweight formatted logging functions.
 */
class ConsolePort
{
public:
    /**
     * @brief Retrieve singleton instance.
     * @return Reference to the global ConsolePort instance.
     */
    static ConsolePort& GetInstance()
    {
        static ConsolePort inst;
        return inst;
    }

    /**
     * @brief Set the log level for the component.
     * @param level Log level as defined by esp_log_level_t.
     */
    void SetLevel(esp_log_level_t level) const { esp_log_level_set(TAG, level); }

    /** @name Logging helpers */
    ///@{

    /** Info level output. */
    template<typename... Args>
    void Info(const char* fmt, Args... args) const
    {
        ESP_LOGI(TAG, fmt, args...);
    }

    /** Warning level output. */
    template<typename... Args>
    void Warn(const char* fmt, Args... args) const
    {
        ESP_LOGW(TAG, fmt, args...);
    }

    /** Error level output. */
    template<typename... Args>
    void Error(const char* fmt, Args... args) const
    {
        ESP_LOGE(TAG, fmt, args...);
    }

    /** Debug level output. */
    template<typename... Args>
    void Debug(const char* fmt, Args... args) const
    {
        ESP_LOGD(TAG, fmt, args...);
    }

    /** Verbose level output. */
    template<typename... Args>
    void Verbose(const char* fmt, Args... args) const
    {
        ESP_LOGV(TAG, fmt, args...);
    }

    /** Info level output when condition is true. */
    template<typename... Args>
    void WriteConditional(bool cond, const char* fmt, Args... args) const
    {
        if (cond) {
            ESP_LOGI(TAG, fmt, args...);
        }
    }

    ///@}

private:
    ConsolePort() = default;
    static constexpr const char* TAG = "ConsolePort";
};

/** Helper macro used throughout the component to log conditionally. */
#define WRITE_CONDITIONAL(cond, fmt, ...) \
    ConsolePort::GetInstance().WriteConditional(cond, fmt, ##__VA_ARGS__)

