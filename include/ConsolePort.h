#pragma once
/**
 * @file ConsolePort.h
 * @brief printf-style ESP-IDF logging with C and C++ compatible interface.
 *
 * All methods forward to esp_log_writev for true printf semantics.
 * Provides both C and C++ compatible logging functions.
 */

#include "esp_log.h"
#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Change the runtime log level for a given TAG.
 * @param tag   null-terminated string tag
 * @param level one of ESP_LOG_NONE, ESP_LOG_ERROR, ESP_LOG_WARN, ESP_LOG_INFO, ESP_LOG_DEBUG, ESP_LOG_VERBOSE
 */
void console_set_level(const char* tag, esp_log_level_t level);

/**
 * @brief Log at INFO level.
 * @param tag   log tag
 * @param fmt   printf-style format string
 * @param ...   printf-style args
 */
void console_info(const char* tag, const char* fmt, ...);

/**
 * @brief Log at WARN level.
 * @param tag   log tag
 * @param fmt   printf-style format string
 * @param ...   printf-style args
 */
void console_warn(const char* tag, const char* fmt, ...);

/**
 * @brief Log at ERROR level.
 * @param tag   log tag
 * @param fmt   printf-style format string
 * @param ...   printf-style args
 */
void console_error(const char* tag, const char* fmt, ...);

/**
 * @brief Log at DEBUG level.
 * @param tag   log tag
 * @param fmt   printf-style format string
 * @param ...   printf-style args
 */
void console_debug(const char* tag, const char* fmt, ...);

/**
 * @brief Log at VERBOSE level.
 * @param tag   log tag
 * @param fmt   printf-style format string
 * @param ...   printf-style args
 */
void console_verbose(const char* tag, const char* fmt, ...);

/**
 * @brief Conditional logging at INFO level.
 * @param cond  only log if true
 * @param tag   log tag
 * @param fmt   printf-style format string
 * @param ...   printf-style args
 */
void console_write_conditional(bool cond, const char* tag, const char* fmt, ...);

/**
 * @brief Unconditional logging at INFO level.
 * @param tag   log tag
 * @param fmt   printf-style format string
 * @param ...   printf-style args
 */
void console_write(const char* tag, const char* fmt, ...);

#ifdef __cplusplus
}

// C++ wrapper class for compatibility
class ConsolePort {
public:
    /// Retrieve the singleton instance
    static ConsolePort& GetInstance() {
        static ConsolePort inst;
        return inst;
    }

    void SetLevel(const char* tag, esp_log_level_t level) const {
        console_set_level(tag, level);
    }

    void Info(const char* tag, const char* fmt, ...) const {
        va_list ap; va_start(ap, fmt);
        esp_log_writev(ESP_LOG_INFO, tag, fmt, ap);
        va_end(ap);
    }

    void Warn(const char* tag, const char* fmt, ...) const {
        va_list ap; va_start(ap, fmt);
        esp_log_writev(ESP_LOG_WARN, tag, fmt, ap);
        va_end(ap);
    }

    void Error(const char* tag, const char* fmt, ...) const {
        va_list ap; va_start(ap, fmt);
        esp_log_writev(ESP_LOG_ERROR, tag, fmt, ap);
        va_end(ap);
    }

    void Debug(const char* tag, const char* fmt, ...) const {
        va_list ap; va_start(ap, fmt);
        esp_log_writev(ESP_LOG_DEBUG, tag, fmt, ap);
        va_end(ap);
    }

    void Verbose(const char* tag, const char* fmt, ...) const {
        va_list ap; va_start(ap, fmt);
        esp_log_writev(ESP_LOG_VERBOSE, tag, fmt, ap);
        va_end(ap);
    }

    static void Write(const char* tag, const char* fmt, ...) {
        va_list ap; va_start(ap, fmt);
        esp_log_writev(ESP_LOG_INFO, tag, fmt, ap);
        va_end(ap);
    }

    static void WriteConditional(bool cond, const char* tag, const char* fmt, ...) {
        if (!cond) return;
        va_list ap; va_start(ap, fmt);
        esp_log_writev(ESP_LOG_INFO, tag, fmt, ap);
        va_end(ap);
    }

private:
    ConsolePort() = default;
    ~ConsolePort() = default;
    ConsolePort(const ConsolePort&) = delete;
    ConsolePort& operator=(const ConsolePort&) = delete;
};
#endif
