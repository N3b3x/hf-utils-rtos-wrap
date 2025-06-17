#pragma once
/**
 * @file ConsolePort.h
 * @brief printf-style ESP-IDF logging with per-call TAG and singleton access.
 *
 * All methods forward to esp_log_writev for true printf semantics.
 */

#include "esp_log.h"
#include <cstdarg>

class ConsolePort {
public:
    /// Retrieve the singleton instance
    static ConsolePort& GetInstance() {
        static ConsolePort inst;
        return inst;
    }

    /**
     * @brief Change the runtime log level for a given TAG.
     * @param tag   null-terminated string tag
     * @param level one of ESP_LOG_NONE, ESP_LOG_ERROR, ESP_LOG_WARN, ESP_LOG_INFO, ESP_LOG_DEBUG, ESP_LOG_VERBOSE
     */
    void SetLevel(const char* tag, esp_log_level_t level) const {
        esp_log_level_set(tag, level);
    }

    /** @name printf-style logging APIs (specify tag per call) */
    ///@{
    void Info(const char* tag, const char* fmt, ...) const {
        va_list ap; va_start(ap, fmt);
        esp_log_writev(ESP_LOG_INFO,    tag, fmt, ap);
        va_end(ap);
    }

    void Warn(const char* tag, const char* fmt, ...) const {
        va_list ap; va_start(ap, fmt);
        esp_log_writev(ESP_LOG_WARN,    tag, fmt, ap);
        va_end(ap);
    }

    void Error(const char* tag, const char* fmt, ...) const {
        va_list ap; va_start(ap, fmt);
        esp_log_writev(ESP_LOG_ERROR,   tag, fmt, ap);
        va_end(ap);
    }

    void Debug(const char* tag, const char* fmt, ...) const {
        va_list ap; va_start(ap, fmt);
        esp_log_writev(ESP_LOG_DEBUG,   tag, fmt, ap);
        va_end(ap);
    }

    void Verbose(const char* tag, const char* fmt, ...) const {
        va_list ap; va_start(ap, fmt);
        esp_log_writev(ESP_LOG_VERBOSE, tag, fmt, ap);
        va_end(ap);
    }

    /**
     * @brief printf-style conditional Info-level log.
     * @param cond  only log if true
     * @param tag   log tag
     * @param fmt   printf-style format string
     * @param ...   printf-style args
     */
    void WriteConditional(bool cond, const char* tag, const char* fmt, ...) const {
        if (!cond) return;
        va_list ap; va_start(ap, fmt);
        esp_log_writev(ESP_LOG_INFO, tag, fmt, ap);
        va_end(ap);
    }
    ///@}

private:
    ConsolePort() = default;
    ~ConsolePort() = default;
    ConsolePort(const ConsolePort&) = delete;
    ConsolePort& operator=(const ConsolePort&) = delete;
};

/** Helper macro for conditional logging via singleton. */
#define WRITE_CONDITIONAL(cond, tag, fmt, ...) \
    ConsolePort::GetInstance().WriteConditional(cond, tag, fmt, ##__VA_ARGS__)
